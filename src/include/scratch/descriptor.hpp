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

    //! Gets the TELNET-SB bit.
    //! \sa #SetTelnetSbBit(const bool)
    bool GetTelnetSbBit() const noexcept {
	return telnetSbBit_;
    }

    //! Writes to the descriptor.
    //! \param message the message to print
    void Print(const String& message) noexcept;

    //! Writes to the descriptor.
    //! \param format the printf-style format specifier
    void PrintFormat(const String& format, ...) noexcept;

    //! Writes a TELNET command.
    //! \param command the TELNET command to write
    //! \sa #PutCommand(const std::uint8_t, const std::uint8_t)
    void PutCommand(const std::uint8_t command) noexcept {
	this->PrintFormat("%c%c", IAC, (char) command);
    }

    //! Writes a TELNET command.
    //! \param command the TELNET command to write
    //! \param option the TELNET command option to write
    //! \sa #PutCommand(const std::uint8_t, const std::uint8_t)
    void PutCommand(
	const std::uint8_t command,
	const std::uint8_t option) noexcept {
	this->PrintFormat("%c%c%c", IAC, (char) command, (char) option);
    }

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
    //! \sa #GetPromptBit() const
    void SetPromptBit(const bool promptBit) noexcept {
	promptBit_ = promptBit;
	this->Write("");
    }

    //! Sets the TELNET-SB bit.
    //! \sa #GetTelnetSbBit() const
    void SetTelnetSbBit(const bool telnetSbBit) {
	telnetSbBit_ = telnetSbBit;
    }

    //! Writes to the descriptor.
    //! \param messsage the message to write
    void Write(const String& message);

    //! Writes the prompt.
    void WritePrompt();

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

    //! The output queue.
    std::deque<String> output_;

    //! The prompt bit.
    //! \sa #GetPromptBit() const
    //! \sa #SetPromptBit(const bool)
    bool promptBit_;

    //! The Boost socket.
    Socket socket_;

    //! The TELNET protocol command.
    std::uint8_t telnetCommand_;

    //! The TELNET protocol option.
    std::uint8_t telnetOption_;

    //! The TELNET-SB input buffer.
    std::ostringstream telnetSb_;

    //! The TELNET-SB bit.
    //! \sa #GetTelnetSbBit() const
    //! \sa #SetTelnetSbBit(const bool)
    bool telnetSbBit_;

    //! Backspaces one character.
    //! \sa #BackspaceLine()
    void Backspace();

    //! Backspaces the entire line.
    //! \sa #Backspace()
    void BackspaceLine();

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

    //! Processes TELNET-IAC input.
    //! \param byteReceived the byte to process
    void ReceiveTelnetIac(const std::uint8_t byteReceived);

    //! Processes TELNET-SB input.
    //! \param sbReceived the TELNET-SB input to process
    void ReceiveTelnetSb(const String& sbReceived);

    //! Processes one byte of TELNET-SB input.
    //! \param sbByteReceived the TELNET-SB byte to process
    void ReceiveTelnetSbByte(const int sbByteReceived);
};
//! \}

}; // namespace Net
}; // namespace Scratch

#endif // _SCRATCH_DESCRIPTOR_HXX_
