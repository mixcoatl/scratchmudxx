//! \file descriptor.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_DESCRIPTOR_CPP_

#define TELCMDS
#define TELOPTS

#include <scratch/game.hpp>
#include <scratch/descriptor.hpp>
#include <scratch/logger.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Net {

//! Constructor.
//! \param game the game state
//! \param socket the Boost socket
Descriptor::Descriptor(
	Game& game,
	Socket&& socket) :
	game_(game),
	input_(),
	lineInput_(),
	name_(),
	output_(),
	promptBit_(true),
	socket_(std::move(socket)),
	telnetCommand_(/* None */ 0),
	telnetOption_(/* None */ 0),
	telnetSb_(),
	telnetSbBit_(false) {
    // Initialize asynchronous operations.
    this->InitAsyncRead();
    this->Write("");
}

//! Destructor.
Descriptor::~Descriptor() noexcept {
    this->Close();
}

//! Closes the descriptor.
//! \sa #Closed() const
void Descriptor::Close() noexcept {
    if (!socket_.is_open())
	return;

    // Close Boost socket.
    ErrorCode errorCode;
    socket_.close(errorCode);

    // Log failures.
    if (errorCode) {
	LOGGER_SYSTEM() << "Error closing socket.";
	LOGGER_SYSTEM() << " >> " << errorCode;
	LOGGER_SYSTEM() << " >> " << errorCode.message();
    }
}

//! Returns whether the descriptor is closed.
//! \sa #Close()
bool Descriptor::Closed() const noexcept {
    return !socket_.is_open();
}

//! Writes to the descriptor.
//! \param message the message to print
void Descriptor::Print(const String& message) noexcept {
    this->Write(message);
}

//! Writes to the descriptor.
//! \param format the printf-style format specifier
void Descriptor::PrintFormat(const String& format, ...) noexcept {
    // Format message.
    va_list args;
    va_start(args, format);
    char message[MaxString] = {'\0'};
    const int N = std::vsnprintf(message, sizeof(message), format.c_str(), args);
    va_end(args);

    // Check return.
    if (N < 0) {
	LOGGER_MAIN() << "std::vsnprintf() failed: errno=" << errno;
	return;
    }

    // Interrupt.
    if (!promptBit_ && !lineInput_.str().length())
	this->Write("\r\n");

    // Write message.
    this->Write(message);
    if (std::find(std::begin(message), std::end(message), '\n') != std::end(message))
	promptBit_ = true;
}

//! Writes to the descriptor.
//! \param messsage the message to write
void Descriptor::Write(const String& message) {
    // Post message.
    boost::asio::post(game_.GetIoContext(), [this, message]() {
	// Are we writing right now?
	const bool inProgress = !output_.empty();

	// Enqueue message.
	output_.push_back(message);

	// Configure asynchronous write.
	if (!game_.GetShutdown() && !inProgress)
	    this->InitAsyncWrite();
    });
}

//! Writes the prompt.
void Descriptor::WritePrompt() {
    this->PrintFormat(":ScratchMUD:> ");
    promptBit_ = false;
}

//! Backspaces one character.
//! \sa #BackspaceLine()
void Descriptor::Backspace() {
    auto const lineInput = lineInput_.str();
    if (lineInput.length()) {
	auto const lineInputN = lineInput.length();
	auto const lineInputBackspaced = lineInput.substr(lineInputN - 1);
	lineInput_.str(lineInputBackspaced);
    }
}

//! Backspaces the entire line.
//! \sa #Backspace()
void Descriptor::BackspaceLine() {
    lineInput_.str(String());
    lineInput_.clear();
}

//! Configures an asynchronous read.
void Descriptor::InitAsyncRead() {
    MutableBuffersType mutableInput = input_.prepare(MaxString);
    socket_.async_read_some(boost::asio::buffer(mutableInput),
	[this](const ErrorCode& errorCode, std::size_t nBytes) {
	    if (errorCode.value() == boost::asio::error::eof) {
		LOGGER_NETWORK() << "Descriptor " << name_ << " disconnected.";
		socket_.close();
	    } else if (errorCode) {
		// Log failures.
		LOGGER_NETWORK() << "Error reading descriptor " << name_ << ".";
		LOGGER_NETWORK() << " >> " << errorCode;
		LOGGER_NETWORK() << " >> " << errorCode.message();
		socket_.close();
	    } else {
		// Commit bytes.
		input_.commit(nBytes);

		// Read bytes from input buffer.
		std::istream input(&input_);
		while (!game_.GetShutdown() && input) {
		    const int byteReceived = input.get();
		    this->ReceiveByte(byteReceived);
		}

		// Advance stream buffer.
		input_.consume(nBytes);

		// Configure asynchronouse read.
		if (!game_.GetShutdown())
		    this->InitAsyncRead();
	    }
	});
}

//! Configures an asynchronous write.
void Descriptor::InitAsyncWrite() {
    auto& message = output_.front();
    boost::asio::async_write(socket_, boost::asio::buffer(message),
	[this, message](ErrorCode errorCode, std::size_t /* length */) {
	    if (errorCode) {
		// Log failure.
		LOGGER_NETWORK() << "Error writing descriptor" << name_ << ".";
		LOGGER_NETWORK() << " >> " << errorCode;
		LOGGER_NETWORK() << " >> " << errorCode.message();
		socket_.close();
		return;
	    }

	    // Dequeue message.
	    output_.pop_front();

	    //  Configure asynchronouse write.
	    if (!game_.GetShutdown() && !output_.empty())
		this->InitAsyncWrite();
	    else if (!game_.GetShutdown() && promptBit_)
		this->WritePrompt();
	});
}

//! Processes one byte on input.
//! \param byteReceived the byte to process
void Descriptor::ReceiveByte(const int byteReceived) {
    switch (telnetCommand_) {
    case /* None */ 0:
	if (byteReceived == IAC) {
	    telnetCommand_ = IAC;
	} else if (telnetSbBit_) {
	    this->ReceiveTelnetSbByte(byteReceived);
	} else {
	    this->ReceiveLineByte(byteReceived);
	}
	break;
    case IAC:
	this->ReceiveTelnetIac(byteReceived);
	break;
    case DO:   case DONT:
    case WILL: case WONT:
    case SB:
	telnetOption_ = byteReceived;
	LOGGER_NETWORK() << "Descriptor " << name_ << " received IAC " << TELCMD(telnetCommand_) << " " << TELOPT(telnetOption_) << ".";
	telnetCommand_ = /* None */ 0;
	telnetOption_  = /* none */ 0;
	break;
    default:
	LOGGER_NETWORK() << "Descriptor " << name_ << " has unknown TELNET state IAC " << TELCMD(telnetCommand_) << ".";
	telnetCommand_ = /* None */ 0;
	telnetOption_  = /* none */ 0;
	break;
    }
}

//! Processes line input.
//! \param lineReceived the line input to process
void Descriptor::ReceiveLine(const String& lineReceived) {
    if (this->Closed()) {
	LOGGER_NETWORK() << "Descriptor " << name_ << " already closed.";
    } else {
	for (auto d: game_.GetDescriptors()) {
	    d->PrintFormat("[%s]: %s\r\n", name_.c_str(), lineReceived.c_str());
	}
    }
}

//! Processes one byte of line input.
//! \param lineByteReceived the line byte to process
void Descriptor::ReceiveLineByte(const int lineByteReceived) {
    if (this->Closed()) {
	LOGGER_NETWORK() << "Descriptor " << name_ << " already closed.";
    } else if (std::strchr("\b\x7f", lineByteReceived) != nullptr) {
	this->Backspace();
    } else if (lineByteReceived == '\n') {
	this->ReceiveLine(lineInput_.str());
	this->BackspaceLine();
    } else if (std::isprint(lineByteReceived) && lineByteReceived != '\r') {
	lineInput_ << (char) lineByteReceived;
    }
}

//! Processes TELNET-IAC input.
//! \param byteReceived the byte to process
void Descriptor::ReceiveTelnetIac(const std::uint8_t byteReceived) {
    switch (byteReceived) {
    case EC:
	this->Backspace();
	break;
    case EL:
	this->BackspaceLine();
	break;
    case IAC:
	if (telnetSbBit_) {
	    this->ReceiveTelnetSbByte(byteReceived);
	    telnetCommand_ = SB;
	} else {
	    this->ReceiveLineByte(byteReceived);
	    telnetCommand_ = /* None */ 0;
	}
	return;
    case SE:
	this->ReceiveTelnetSb(telnetSb_.str());
	break;
    default:
	telnetCommand_ = byteReceived;
	return;
    }

    telnetCommand_ = /* None */ 0;
    telnetOption_  = /* None */ 0;
}

//! Processes TELNET-SB input.
//! \param sbReceived the TELNET-SB input to process
void Descriptor::ReceiveTelnetSb(const String& sbReceived) {
    telnetCommand_ = /* None */ 0;
    telnetOption_  = /* None */ 0;
}

//! Processes one byte of TELNET-SB byte.
//! \param sbByteReceived the TELNET-SB byte to process
void Descriptor::ReceiveTelnetSbByte(const int sbByteReceived) {
    if (this->Closed()) {
	LOGGER_NETWORK() << "Descriptor " << name_ << " already closed.";
    } else {
	telnetSb_ << (char) sbByteReceived;
    }
}

}; // namespace Net
}; // namespace Scratch
