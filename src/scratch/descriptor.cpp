//! \file descriptor.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_DESCRIPTOR_CPP_

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
	name_(),
	output_(),
	socket_(std::move(socket)) {
    // Initialize asynchronous read.
    this->InitAsyncRead();
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
//! \param messsage the message to write
void Descriptor::Write(const String& message) {
    // Nothing to write.
    if (!message.size())
	return;

    // Post message.
    boost::asio::post(game_.GetIoContext(),
	[this, message]() {
	    // Are we writing right now?
	    const bool inProgress = !output_.empty();

	    // Enqueue message.
	    output_.push_back(message);

	    // Configure asynchronous write.
	    if (!game_.GetShutdown() && !inProgress)
		this->InitAsyncWrite();
	});
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
	    if (!output_.empty() && !game_.GetShutdown())
		this->InitAsyncWrite();
	});
}

//! Processes one byte on input.
//! \param byteReceived the byte to process
void Descriptor::ReceiveByte(const int byteReceived) {
    // Nothing.
}

//! Processes line input.
//! \param lineReceived the line input to process
void Descriptor::ReceiveLine(const String& lineReceived) {
    // Nothing.
}

//! Processes one byte of line input.
//! \param lineByteReceived the line byte to process
void Descriptor::ReceiveLineByte(const int lineByteReceived) {
    // Nothing.
}

}; // namespace Net
}; // namespace Scratch
