//! \file descriptor.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
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

// Forward declarations.
class Protocol;

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
    //! \sa #Write(const String&)
    template<class T>
    Descriptor& operator<<(const T& value) {
	this->Write(boost::lexical_cast<String>(value));
	return *this;
    }

    //! Backspaces one character.
    //! \sa #BackspaceLine()
    void Backspace();

    //! Backspaces the entire line.
    //! \sa #Backspace()
    void BackspaceLine();

    //! Closes the descriptor.
    //! \sa #Closed() const
    void Close() noexcept;

    //! Returns whether the descriptor is closed.
    //! \sa #Close()
    bool Closed() const noexcept;

    //! Delivers one application-data byte from the protocol.
    //! \param byteReceived the byte to deliver
    void DeliverByte(const std::uint8_t byteReceived);

    //! Returns the color code.
    //! \param color the color code: C_x
    const char *GetColor(const int color) noexcept;

    //! Gets the color bit.
    //! \sa #SetColorBit(const bool)
    bool GetColorBit() const noexcept {
	return colorBit_;
    }

    //! Gets the descriptor name.
    //! \sa #SetName(const String&)
    String GetName() const noexcept {
	return name_;
    }

    //! Gets the prompt bit.
    //! \sa #SetPromptBit(const bool)
    bool GetPromptBit() const noexcept {
	return promptBit_;
    }

    //! Gets the TELNET terminal type.
    //! \sa #SetTerminalType(const String&)
    String GetTerminalType() const noexcept {
	return terminalType_;
    }

    //! Gets the terminal window height in characters.
    //! \remark Defaults to 24 until NAWS reports a size.
    //! \sa #SetWindowSize(const std::uint16_t, const std::uint16_t)
    std::uint16_t GetWindowHeight() const noexcept {
	return windowHeight_;
    }

    //! Gets the terminal window width in characters.
    //! \remark Defaults to 80 until NAWS reports a size.
    //! \sa #SetWindowSize(const std::uint16_t, const std::uint16_t)
    std::uint16_t GetWindowWidth() const noexcept {
	return windowWidth_;
    }

    //! Writes to the descriptor.
    //! \param message the message to print
    void Print(const String& message) noexcept;

    //! Writes to the descriptor.
    //! \param format the printf-style format specifier
    void PrintFormat(const String& format, ...) noexcept;

    //! Sets the color bit.
    //! \sa #GetColorBit() const
    void SetColorBit(const bool colorBit) noexcept {
	colorBit_ = colorBit;
    }

    //! Sets the descriptor name.
    //! \sa #GetName() const
    void SetName(const String& name) {
	name_ = name;
    }

    //! Sets the prompt bit.
    //! \param promptBit the prompt bit value
    //! \sa #GetPromptBit() const
    void SetPromptBit(const bool promptBit) noexcept;

    //! Sets the TELNET terminal type.
    //! \param terminalType the terminal type string
    //! \sa #GetTerminalType() const
    void SetTerminalType(const String& terminalType) {
	terminalType_ = terminalType;
    }

    //! Sets the terminal window size in characters.
    //! \param width the window width
    //! \param height the window height
    //! \sa #GetWindowHeight() const
    //! \sa #GetWindowWidth() const
    void SetWindowSize(
	const std::uint16_t width,
	const std::uint16_t height) noexcept {
	windowWidth_ = width;
	windowHeight_ = height;
    }

    //! Begins asynchronous I/O after the descriptor is indexed by the game.
    void Start();

    //! Writes application output through the protocol.
    //! \param message the message to write
    void Write(const String& message);

    //! Writes the prompt.
    void WritePrompt();

    //! Writes raw bytes to the wire.
    //! \param message the message to write
    void WriteRaw(const String& message);

protected:
    //! The color bit.
    //! \sa #GetColorBit() const
    //! \sa #SetColorBit(const bool)
    bool colorBit_;

    //! The game state.
    Game& game_;

    //! The input buffer.
    //! \remark Used by \ref boost::asio::async_read().
    StreamBuf input_;

    //! The line input buffer.
    std::ostringstream lineInput_;

    //! The descriptor name.
    //! \sa #GetName() const
    //! \sa #SetName(const String&)
    String name_;

    //! The pending wire output buffer.
    StreamBuf output_;

    //! The prompt bit.
    //! \sa #GetPromptBit() const
    //! \sa #SetPromptBit(const bool)
    bool promptBit_;

    //! The wire protocol.
    std::unique_ptr<Protocol> protocol_;

    //! The Boost socket.
    Socket socket_;

    //! The TELNET terminal type.
    //! \sa #GetTerminalType() const
    //! \sa #SetTerminalType(const String&)
    String terminalType_;

    //! The terminal window height in characters.
    //! \remark Defaults to 24 until NAWS reports a size.
    //! \sa #GetWindowHeight() const
    //! \sa #SetWindowSize(const std::uint16_t, const std::uint16_t)
    std::uint16_t windowHeight_;

    //! The terminal window width in characters.
    //! \remark Defaults to 80 until NAWS reports a size.
    //! \sa #GetWindowWidth() const
    //! \sa #SetWindowSize(const std::uint16_t, const std::uint16_t)
    std::uint16_t windowWidth_;

    //! Whether an asynchronous write is pending.
    bool writePending_;

    //! Configures an asynchronous read.
    void InitAsyncRead();

    //! Configures an asynchronous write.
    void InitAsyncWrite();

    //! Processes line input.
    //! \param lineReceived the line input to process
    void ReceiveLine(const String& lineReceived);
};
//! \}

}; // namespace Net
}; // namespace Scratch

#endif // _SCRATCH_DESCRIPTOR_HPP_
