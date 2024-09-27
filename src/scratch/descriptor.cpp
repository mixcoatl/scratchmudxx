//! \file descriptor.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2024 scratchmud.org
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
	mGame(game),
	mInput(),
	mName(),
	mOutput(),
	mSocket(std::move(socket)) {
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
//! \param messsage the message to write
void Descriptor::Write(const String& message) {
    // Nothing to write.
    if (!message.size())
	return;

    // Post message.
    boost::asio::post(mGame.GetIoContext(),
	[this, message]() {
	    // Are we writing right now?
	    const bool inProgress = !mOutput.empty();

	    // Enqueue message.
	    mOutput.push_back(message);

	    // Configure asynchronous write.
	    if (!mGame.GetShutdown() && !inProgress)
		this->InitAsyncWrite();
	});
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
	    if (!mOutput.empty() && !mGame.GetShutdown())
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
