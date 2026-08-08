//! \file descriptor.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_DESCRIPTOR_CPP_

#include <scratch/color.hpp>
#include <scratch/game.hpp>
#include <scratch/descriptor.hpp>
#include <scratch/logger.hpp>
#include <scratch/scratch.hpp>
#include <scratch/protocol_telnet.hpp>

namespace Scratch {
namespace Net {

// ScratchMUD types.
using Color = Scratch::Net::Color;

//! Constructor.
//! \param game the game state
//! \param socket the Boost socket
Descriptor::Descriptor(
	Game& game,
	Socket&& socket) :
	colorBit_(true),
	game_(game),
	input_(),
	lineInput_(),
	name_(),
	output_(),
	promptBit_(true),
	protocol_(),
	socket_(std::move(socket)),
	terminalType_(),
	windowHeight_(24),
	windowWidth_(80),
	writePending_(false) {
    // Default to TELNET.
    protocol_ = std::make_unique<TelnetProtocol>(*this);
}

//! Destructor.
Descriptor::~Descriptor() noexcept {
    this->Close();
}

//! Backspaces one character.
//! \sa #BackspaceLine()
void Descriptor::Backspace() {
    auto const lineInput = lineInput_.str();
    if (lineInput.length()) {
	auto const lineInputN = lineInput.length();
	lineInput_.str(lineInput.substr(0, lineInputN - 1));
	lineInput_.clear();
	lineInput_.seekp(0, std::ios::end);
    }
}

//! Backspaces the entire line.
//! \sa #Backspace()
void Descriptor::BackspaceLine() {
    lineInput_.str(String());
    lineInput_.clear();
}

//! Closes the descriptor.
//! \sa #Closed() const
void Descriptor::Close() noexcept {
    if (!socket_.is_open())
	return;

    LOGGER_NETWORK() << "Descriptor " << name_ << " disconnected.";

    // Close Boost socket. Outstanding async ops are cancelled and their
    // completion handlers are queued on IO context before close returns.
    ErrorCode errorCode;
    socket_.close(errorCode);

    // Log failures.
    if (errorCode) {
	LOGGER_SYSTEM() << "Error closing socket.";
	LOGGER_SYSTEM() << " >> " << errorCode;
	LOGGER_SYSTEM() << " >> " << errorCode.message();
    }

    // Reap from game index after cancelled completions
    // so handlers still observe live, map-owned object.
    boost::asio::post(game_.GetIoContext(), [name = name_, &game = game_]() {
	game.EraseDescriptor(name);
    });
}

//! Returns whether the descriptor is closed.
//! \sa #Close()
bool Descriptor::Closed() const noexcept {
    return !socket_.is_open();
}

//! Returns the color code.
//! \param color the color code: C_x
const char *Descriptor::GetColor(const int color) noexcept {
    if (colorBit_) {
	switch (color) {
	case Color::C_CHARCOAL:	return "\x1b[0;30m";
	case Color::C_CRIMSON:	return "\x1b[0;31m";
	case Color::C_FOREST:	return "\x1b[0;32m";
	case Color::C_OCHRE:	return "\x1b[0;33m";
	case Color::C_INDIGO:	return "\x1b[0;34m";
	case Color::C_PURPLE:	return "\x1b[0;35m";
	case Color::C_TEAL:	return "\x1b[0;36m";
	case Color::C_SILVER:	return "\x1b[0;37m";
	case Color::C_GRAY:	return "\x1b[1;30m";
	case Color::C_PINK:	return "\x1b[1;31m";
	case Color::C_LIME:	return "\x1b[1;32m";
	case Color::C_AMBER:	return "\x1b[1;33m";
	case Color::C_AZURE:	return "\x1b[1;34m";
	case Color::C_VIOLET:	return "\x1b[1;35m";
	case Color::C_AQUA:	return "\x1b[1;36m";
	case Color::C_SNOW:	return "\x1b[1;37m";
	case Color::C_NORMAL:	return "\x1b[0m";
	default:		return "";
	}
    }
    return "";
}

//! Delivers one application-data byte from the protocol.
//! \param byteReceived the byte to deliver
void Descriptor::DeliverByte(const std::uint8_t byteReceived) {
    if (this->Closed()) {
	LOGGER_NETWORK() << "Descriptor " << name_ << " already closed.";
    } else if (std::strchr("\b\x7f", byteReceived) != nullptr) {
	this->Backspace();
    } else if (byteReceived == '\n') {
	this->ReceiveLine(lineInput_.str());
	this->BackspaceLine();
    } else if (std::isprint(byteReceived) && byteReceived != '\r') {
	if (lineInput_.str().size() < MaxInput)
	    lineInput_ << static_cast<char>(byteReceived);
    }
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

    // Interrupt the prompt line so \r in the message cannot overwrite it.
    if (!promptBit_)
	this->Write("\r\n");

    // Write message.
    this->Write(message);
    if (std::find(std::begin(message), std::end(message), '\n') != std::end(message))
	promptBit_ = true;
}

//! Sets the prompt bit.
//! \param promptBit the prompt bit value
//! \sa #GetPromptBit() const
void Descriptor::SetPromptBit(const bool promptBit) noexcept {
    promptBit_ = promptBit;
    if (!promptBit)
	return;

    // Show the prompt now if the connection is idle; otherwise the
    // write completion handler shows it once output drains.
    const auto self = this->shared_from_this();
    boost::asio::post(game_.GetIoContext(), std::function<void()>([self]() {
	if (!self->Closed() && !self->game_.GetShutdown() &&
	    self->promptBit_ && !self->writePending_ && !self->output_.size())
	    self->WritePrompt();
    }));
}

//! Begins asynchronous I/O after the descriptor is indexed by the game.
void Descriptor::Start() {
    protocol_->OnStart();
    this->InitAsyncRead();
    this->Write("");
}

//! Writes application output through the protocol.
//! \param message the message to write
void Descriptor::Write(const String& message) {
    protocol_->Send(message);
}

//! Writes the prompt.
void Descriptor::WritePrompt() {
    this->PrintFormat("%s:ScratchMUD:> %s",
	this->GetColor(Color::C_CRIMSON),
	this->GetColor(Color::C_NORMAL));

    // Restore in-progress input that local echo already
    // showed on the prompt line that we interrupted.
    const auto pendingInput = lineInput_.str();
    if (!pendingInput.empty())
	this->Write(pendingInput);

    promptBit_ = false;
    protocol_->OnPrompt();
}

//! Writes raw bytes to the wire.
//! \param message the message to write
void Descriptor::WriteRaw(const String& message) {
    if (this->Closed()) {
	LOGGER_ASSERT() << "Descriptor " << name_ << " already closed.";
	return;
    }

    // Post onto the IO context so buffer mutations stay single-threaded.
    // std::function type-erases the handler so shared_ptr captures do not
    // trip -Werror=inline inside Boost.Asio templates.
    const auto self = this->shared_from_this();
    boost::asio::post(game_.GetIoContext(), std::function<void()>([self, message]() {
	if (self->Closed() || self->game_.GetShutdown())
	    return;

	// Cap pending wire bytes. Try a flush if idle; if still no room,
	// drop the connection with an overflow error.
	if (self->output_.size() + message.size() > MaxOutput) {
	    if (!self->writePending_ && self->output_.size())
		self->InitAsyncWrite();
	    if (self->output_.size() + message.size() > MaxOutput) {
		LOGGER_NETWORK() << "Descriptor " << self->name_ << " output exceeded " << MaxOutput << " bytes; closing.";
		self->Close();
	    }
	}

	if (!self->Closed() && !message.empty())
	    self->output_.sputn(message.data(), static_cast<std::streamsize>(message.size()));

	if (!self->Closed() && !self->writePending_ && self->output_.size())
	    self->InitAsyncWrite();
    }));
}

//! Configures an asynchronous read.
void Descriptor::InitAsyncRead() {
    MutableBuffersType mutableInput = input_.prepare(MaxString);
    const auto self = this->shared_from_this();
    socket_.async_read_some(boost::asio::buffer(mutableInput),
	std::function<void(const ErrorCode&, std::size_t)>(
	[self](const ErrorCode& errorCode, std::size_t nBytes) {
	    if (errorCode.value() == boost::asio::error::eof) {
		self->Close();
	    } else if (errorCode) {
		// Log failures. operation_aborted is expected after Close().
		if (errorCode != boost::asio::error::operation_aborted) {
		    LOGGER_NETWORK() << "Error reading descriptor " << self->name_ << ".";
		    LOGGER_NETWORK() << " >> " << errorCode;
		    LOGGER_NETWORK() << " >> " << errorCode.message();
		}
		self->Close();
	    } else {
		// Commit bytes.
		self->input_.commit(nBytes);

		// Read bytes from input buffer.
		std::istream input(&self->input_);
		while (!self->game_.GetShutdown() && input) {
		    const int byteReceived = input.get();
		    if (byteReceived == std::char_traits<char>::eof())
			break;
		    self->protocol_->Receive(static_cast<std::uint8_t>(byteReceived));
		}

		// Advance stream buffer.
		self->input_.consume(nBytes);

		// Configure asynchronouse read.
		if (!self->game_.GetShutdown() && !self->Closed())
		    self->InitAsyncRead();
	    }
	}));
}

//! Configures an asynchronous write.
void Descriptor::InitAsyncWrite() {
    if (writePending_ || !output_.size())
	return;

    writePending_ = true;
    const auto self = this->shared_from_this();
    boost::asio::async_write(socket_, output_.data(),
	std::function<void(ErrorCode, std::size_t)>(
	[self](ErrorCode errorCode, std::size_t nBytes) {
	    self->writePending_ = false;

	    if (errorCode) {
		// Log failure. operation_aborted is expected after Close().
		if (errorCode != boost::asio::error::operation_aborted) {
		    LOGGER_NETWORK() << "Error writing descriptor" << self->name_ << ".";
		    LOGGER_NETWORK() << " >> " << errorCode;
		    LOGGER_NETWORK() << " >> " << errorCode.message();
		}
		self->Close();
		return;
	    }

	    // Advance the output buffer past bytes written.
	    self->output_.consume(nBytes);

	    // Continue draining, or show the prompt when idle.
	    if (!self->game_.GetShutdown() && !self->Closed() && self->output_.size())
		self->InitAsyncWrite();
	    else if (!self->game_.GetShutdown() && !self->Closed() && self->promptBit_)
		self->WritePrompt();
	}));
}

//! Processes line input.
//! \param lineReceived the line input to process
void Descriptor::ReceiveLine(const String& lineReceived) {
    if (this->Closed()) {
	LOGGER_NETWORK() << "Descriptor " << name_ << " already closed.";
    } else {
	for (auto d: game_.GetDescriptors()) {
	    d->PrintFormat("%s[%s%s%s]: %s%s%s\r\n",
		this->GetColor(Color::C_SILVER), this->GetColor(Color::C_FOREST), name_.c_str(),
		this->GetColor(Color::C_SILVER), this->GetColor(Color::C_FOREST), lineReceived.c_str(),
		this->GetColor(Color::C_NORMAL));
	}
    }
}

}; // namespace Net
}; // namespace Scratch
