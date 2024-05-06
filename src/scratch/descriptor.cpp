//! \file descriptor.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2023 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_DESCRIPTOR_CPP_

#define TELCMDS
#define TELOPTS

#include <scratch/color.hpp>
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
	mColorBit(true),
	mGame(game),
	mInput(),
	mLineInput(),
	mName(),
	mOutput(),
	mPromptBit(true),
	mSocket(std::move(socket)),
	mTelnetCommand(/* None */ 0),
	mTelnetOption(/* None */ 0),
	mTelnetSb(),
	mTelnetSbBit(false) {
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
    if (!mSocket.is_open())
	return;

    // Close Boost socket.
    ErrorCode errorCode;
    mSocket.close(errorCode);

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
    return !mSocket.is_open();
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
    if (!mPromptBit && !mLineInput.str().length())
	this->Write("\r\n");

    // Write message.
    this->Write(message);
    if (std::find(std::begin(message), std::end(message), '\n') != std::end(message))
	mPromptBit = true;
}

//! Writes to the descriptor.
//! \param messsage the message to write
void Descriptor::Write(const String& message) {
    // Post message.
    boost::asio::post(mGame.GetIoContext(), [this, message]() {
	// Are we writing right now?
	const bool inProgress = !mOutput.empty();

	// Enqueue message.
	mOutput.push_back(message);

	// Configure asynchronous write.
	if (!mGame.GetShutdown() && !inProgress)
	    this->InitAsyncWrite();
    });
}

//! Writes the prompt.
void Descriptor::WritePrompt() {
    this->PrintFormat("%s:ScratchMUD:> %s", QT_RED, QT_NORMAL);
    mPromptBit = false;
}

//! Backspaces one character.
//! \sa #BackspaceLine()
void Descriptor::Backspace() {
    auto const lineInput = mLineInput.str();
    if (lineInput.length()) {
	auto const lineInputN = lineInput.length();
	auto const lineInputBackspaced = lineInput.substr(lineInputN - 1);
	mLineInput.str(lineInputBackspaced);
    }
}

//! Backspaces the entire line.
//! \sa #Backspace()
void Descriptor::BackspaceLine() {
    mLineInput.str(String());
    mLineInput.clear();
}

//! Configures an asynchronous read.
void Descriptor::InitAsyncRead() {
    MutableBuffersType mutableInput = mInput.prepare(MaxString);
    mSocket.async_read_some(boost::asio::buffer(mutableInput),
	[this](const ErrorCode& errorCode, std::size_t nBytes) {
	    if (errorCode.value() == boost::asio::error::eof) {
		LOGGER_NETWORK() << "Descriptor " << mName << " disconnected.";
		mSocket.close();
	    } else if (errorCode) {
		// Log failures.
		LOGGER_NETWORK() << "Error reading descriptor " << mName << ".";
		LOGGER_NETWORK() << " >> " << errorCode;
		LOGGER_NETWORK() << " >> " << errorCode.message();
		mSocket.close();
	    } else {
		// Commit bytes.
		mInput.commit(nBytes);

		// Read bytes from input buffer.
		std::istream input(&mInput);
		while (!mGame.GetShutdown() && input) {
		    const int byteReceived = input.get();
		    this->ReceiveByte(byteReceived);
		}

		// Advance stream buffer.
		mInput.consume(nBytes);

		// Configure asynchronouse read.
		if (!mGame.GetShutdown())
		    this->InitAsyncRead();
	    }
	});
}

//! Configures an asynchronous write.
void Descriptor::InitAsyncWrite() {
    auto& message = mOutput.front();
    boost::asio::async_write(mSocket, boost::asio::buffer(message),
	[this, message](ErrorCode errorCode, std::size_t /* length */) {
	    if (errorCode) {
		// Log failure.
		LOGGER_NETWORK() << "Error writing descriptor" << mName << ".";
		LOGGER_NETWORK() << " >> " << errorCode;
		LOGGER_NETWORK() << " >> " << errorCode.message();
		mSocket.close();
		return;
	    }

	    // Dequeue message.
	    mOutput.pop_front();

	    //  Configure asynchronouse write.
	    if (!mGame.GetShutdown() && !mOutput.empty())
		this->InitAsyncWrite();
	    else if (!mGame.GetShutdown() && mPromptBit)
		this->WritePrompt();
	});
}

//! Processes one byte on input.
//! \param byteReceived the byte to process
void Descriptor::ReceiveByte(const int byteReceived) {
    switch (mTelnetCommand) {
    case /* None */ 0:
	if (byteReceived == IAC) {
	    mTelnetCommand = IAC;
	} else if (mTelnetSbBit) {
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
	mTelnetOption = byteReceived;
	LOGGER_NETWORK() << "Descriptor " << mName << " received IAC " << TELCMD(mTelnetCommand) << " " << TELOPT(mTelnetOption) << ".";
	mTelnetCommand = /* None */ 0;
	mTelnetOption  = /* none */ 0;
	break;
    default:
	LOGGER_NETWORK() << "Descriptor " << mName << " has unknown TELNET state IAC " << TELCMD(mTelnetCommand) << ".";
	mTelnetCommand = /* None */ 0;
	mTelnetOption  = /* none */ 0;
	break;
    }
}

//! Processes line input.
//! \param lineReceived the line input to process
void Descriptor::ReceiveLine(const String& lineReceived) {
    if (this->Closed()) {
	LOGGER_NETWORK() << "Descriptor " << mName << " already closed.";
    } else {
	for (auto d: mGame.GetDescriptors()) {
	    d->PrintFormat("%s[%s%s%s]: %s%s%s\r\n", QT_WHITE, QT_GREEN, mName.c_str(), QT_WHITE, QT_GREEN, lineReceived.c_str(), QT_NORMAL);
	}
    }
}

//! Processes one byte of line input.
//! \param lineByteReceived the line byte to process
void Descriptor::ReceiveLineByte(const int lineByteReceived) {
    if (this->Closed()) {
	LOGGER_NETWORK() << "Descriptor " << mName << " already closed.";
    } else if (std::strchr("\b\x7f", lineByteReceived) != nullptr) {
	this->Backspace();
    } else if (lineByteReceived == '\n') {
	this->ReceiveLine(mLineInput.str());
	this->BackspaceLine();
    } else if (std::isprint(lineByteReceived) && lineByteReceived != '\r') {
	mLineInput << (char) lineByteReceived;
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
	if (mTelnetSbBit) {
	    this->ReceiveTelnetSbByte(byteReceived);
	    mTelnetCommand = SB;
	} else {
	    this->ReceiveLineByte(byteReceived);
	    mTelnetCommand = /* None */ 0;
	}
	return;
    case SE:
	this->ReceiveTelnetSb(mTelnetSb.str());
	break;
    default:
	mTelnetCommand = byteReceived;
	return;
    }

    mTelnetCommand = /* None */ 0;
    mTelnetOption  = /* None */ 0;
}

//! Processes TELNET-SB input.
//! \param sbReceived the TELNET-SB input to process
void Descriptor::ReceiveTelnetSb(const String& sbReceived) {
    mTelnetCommand = /* None */ 0;
    mTelnetOption  = /* None */ 0;
}

//! Processes one byte of TELNET-SB byte.
//! \param sbByteReceived the TELNET-SB byte to process
void Descriptor::ReceiveTelnetSbByte(const int sbByteReceived) {
    if (this->Closed()) {
	LOGGER_NETWORK() << "Descriptor " << mName << " already closed.";
    } else {
	mTelnetSb << (char) sbByteReceived;
    }
}

}; // namespace Net
}; // namespace Scratch
