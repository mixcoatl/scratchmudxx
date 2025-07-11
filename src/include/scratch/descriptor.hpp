//! \file descriptor.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_DESCRIPTOR_HPP_
#define _SCRATCH_DESCRIPTOR_HPP_

#include <scratch/scratch.hpp>

// Forward declarations.
namespace Scratch {
namespace Core {
class Game;
}; // namespace Core
}; // namespace Scratch

namespace Scratch {
namespace Net {

// Boost types.
using ErrorCode = boost::system::error_code;
using MutableBuffersType = boost::asio::streambuf::mutable_buffers_type;
using Socket = boost::asio::ip::tcp::socket;
using StreamBuf = boost::asio::streambuf;

// ScratchMUD types.
using Game = Scratch::Core::Game;

//! The descriptor class. \{
class Descriptor: public std::enable_shared_from_this<Descriptor> {
public:
    //! Constructor.
    //! \param game the game state
    //! \param socket the Boost socket
    Descriptor(
	Game& game,
	Socket&& socket);

    //! Destructor.
    virtual ~Descriptor() noexcept;

    //! Writes to the descriptor.
    //! \param value the value to write
    //! \sa #send(const String&)
    template<class T>
    Descriptor& operator<<(const T& value) {
	this->Write(boost::lexical_cast<String>(value));
	return *this;
    }

    //! Closes the descriptor.
    //! \sa #Closed() const
    void Close() noexcept;

    //! Returns whether the descriptor is closed.
    //! \sa #Close()
    bool Closed() const noexcept;

    //! Gets the descriptor name.
    //! \sa #SetName(const String&)
    String GetName() const noexcept {
	return name_;
    }

    //! Sets the descriptor name.
    //! \sa #GetName() const
    void SetName(const String& name) {
	name_ = name;
    }

    //! Writes to the descriptor.
    //! \param messsage the message to write
    void Write(const String& message);

protected:
    //! The game state.
    Game& game_;

    //! The input buffer.
    //! \remark Used by \ref boost::asio::async_read().
    StreamBuf input_;

    //! The descriptor name.
    //! \sa #GetName() const
    //! \sa #SetName(const String&)
    String name_;

    //! The output queue.
    std::deque<String> output_;

    //! The Boost socket.
    Socket socket_;

    //! Configures an asynchronous read.
    void InitAsyncRead();

    //! Configures an asynchronous write.
    void InitAsyncWrite();

    //! Processes one byte of input.
    //! \param byteReceived the byte to process
    void ReceiveByte(const int byteReceived);

    //! Processes line input.
    //! \param lineReceived the line input to process
    void ReceiveLine(const String& lineReceived);

    //! Processes one byte of line input.
    //! \param lineByteReceived the line byte to process
    void ReceiveLineByte(const int lineByteReceived);
};
//! \}

}; // namespace Net
}; // namespace Scratch

#endif // _SCRATCH_DESCRIPTOR_HXX_
