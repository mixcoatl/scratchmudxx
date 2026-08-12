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
#include <deque>

// Forward declarations.
namespace Scratch {
namespace Core {
class Enumeration;
class Game;
class State;
class User;
}; // namespace Core
}; // namespace Scratch

namespace Scratch {
namespace Net {

// Forward declarations.
class Editor;
class Menu;
class Protocol;

// Boost types.
using ErrorCode = boost::system::error_code;
using MutableBuffersType = boost::asio::streambuf::mutable_buffers_type;
using Socket = boost::asio::ip::tcp::socket;
using StreamBuf = boost::asio::streambuf;

// ScratchMUD types.
using EditorPtr = std::shared_ptr<Editor>;
using Enumeration = Scratch::Core::Enumeration;
using EnumerationPtr = std::shared_ptr<Enumeration>;
using Game = Scratch::Core::Game;
using MenuPtr = std::shared_ptr<Menu>;
using State = Scratch::Core::State;
using StatePtr = std::shared_ptr<State>;
using User = Scratch::Core::User;
using UserPtr = std::shared_ptr<User>;

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

    //! Clears the descriptor-owned editor.
    //! \sa #EnsureEditor()
    //! \sa #GetEditor() const
    void ClearEditor() noexcept;

    //! Clears the descriptor-owned menu.
    //! \sa #EnsureMenu()
    //! \sa #GetMenu() const
    void ClearMenu() noexcept;

    //! Closes the descriptor.
    //! \sa #Closed() const
    //! \sa #Login(const UserPtr&)
    void Close() noexcept;

    //! Returns whether the descriptor is closed.
    //! \sa #Close()
    bool Closed() const noexcept;

    //! Returns whether the editor is intercepting input.
    //! \sa #GetEditor() const
    bool IsEditorActive() const noexcept;

    //! Delivers one application-data byte from the protocol.
    //! \param byteReceived the byte to deliver
    void DeliverByte(const std::uint8_t byteReceived);

    //! Returns the editor, creating one if needed.
    //! \sa #ClearEditor()
    //! \sa #GetEditor() const
    EditorPtr EnsureEditor();

    //! Returns the menu, creating an empty one if needed.
    //! \sa #ClearMenu()
    //! \sa #GetMenu() const
    MenuPtr EnsureMenu();

    //! Returns the color code.
    //! \param color the color
    const char *GetColor(const int color) const noexcept;
    //! Gets the color bit.
    //! \sa #SetColorBit(const bool)
    bool GetColorBit() const noexcept {
	return colorBit_;
    }

    //! Gets the repository key of the thing being edited, if any.
    //! \remark Empty when the edit draft is new (not cloned from a named thing).
    //! \sa #SetEditName(const String&)
    String GetEditName() const noexcept {
	return editName_;
    }

    //! Gets the enumeration being edited.
    //! \sa #SetEditEnumeration(const EnumerationPtr&)
    EnumerationPtr GetEditEnumeration() const noexcept {
	return editEnumeration_;
    }


    //! Gets the connection state being edited.
    //! \sa #SetEditState(const StatePtr&)
    StatePtr GetEditState() const noexcept {
	return editState_;
    }

    //! Gets the multi-step edit string.
    //! \remark Login name, password confirm, and similar.
    //! \sa #SetEditString(const String&)
    String GetEditString() const noexcept {
	return editString_;
    }

    //! Gets the user being edited.
    //! \sa #SetEditUser(const UserPtr&)
    UserPtr GetEditUser() const noexcept {
	return editUser_;
    }

    //! Gets the descriptor-owned editor, if any.
    //! \sa #ClearEditor()
    //! \sa #EnsureEditor()
    EditorPtr GetEditor() const noexcept {
	return editor_;
    }

    //! Gets the descriptor-owned menu, if any.
    //! \sa #ClearMenu()
    //! \sa #EnsureMenu()
    MenuPtr GetMenu() const noexcept {
	return menu_;
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

    //! Gets the connection state.
    //! \remark Mirrors the front of the connection-state stack.
    //! \sa #SetState(const StatePtr&)
    //! \sa #SetStateByName(const String&)
    //! \sa #PushState(const StatePtr&)
    //! \sa #PopState()
    StatePtr GetState() const noexcept {
	return state_;
    }

    //! Gets the TELNET terminal type.
    //! \sa #SetTerminalType(const String&)
    String GetTerminalType() const noexcept {
	return terminalType_;
    }

    //! Gets the attached user.
    //! \sa #Login(const UserPtr&)
    UserPtr GetUser() const noexcept {
	return user_;
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

    //! Logs in the specified user, setting LastLogin and persisting.
    //! \param user the user to log in
    //! \remark No-op for the already-attached user; logs out a different
    //!     previously-attached user first. Rejects a null user.
    //! \sa #Close()
    //! \sa #GetUser() const
    void Login(const UserPtr& user) noexcept;

    //! Writes to the descriptor.
    //! \param message the message to print
    void Print(const String& message) noexcept;

    //! Writes cells in a column-major fold.
    //! \param cells the pre-rendered cell strings
    //! \sa #Print(const String&)
    //! \sa #GetWindowWidth() const
    void PrintColumns(const std::vector<String>& cells) noexcept;

    //! Writes to the descriptor.
    //! \param format the printf-style format specifier
    void PrintFormat(const String& format, ...) noexcept;

    //! Renders the descriptor-owned menu.
    //! \return \c false if there is no menu or no prompt
    //! \sa #GetMenu() const
    bool PrintMenu();

    //! Sets the color bit.
    //! \sa #GetColorBit() const
    void SetColorBit(const bool colorBit) noexcept {
	colorBit_ = colorBit;
    }

    //! Sets the repository key of the thing being edited.
    //! \sa #GetEditName() const
    void SetEditName(const String& editName) {
	editName_ = editName;
    }

    //! Sets the enumeration being edited.
    //! \param editEnumeration the enumeration draft being edited
    //! \sa #GetEditEnumeration() const
    void SetEditEnumeration(const EnumerationPtr& editEnumeration) {
	editEnumeration_ = editEnumeration;
    }


    //! Sets the connection state being edited.
    //! \param editState the connection state being edited
    //! \sa #GetEditState() const
    void SetEditState(const StatePtr& editState) {
	editState_ = editState;
    }

    //! Sets the multi-step edit string.
    //! \param editString the value to store
    //! \sa #GetEditString() const
    void SetEditString(const String& editString) {
	editString_ = editString;
    }

    //! Sets the user being edited.
    //! \param editUser the user draft being edited
    //! \sa #GetEditUser() const
    void SetEditUser(const UserPtr& editUser) {
	editUser_ = editUser;
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

    //! Enables or disables Quiet (server echo / hidden client local echo).
    //! \param quiet whether Quiet is enabled
    void SetQuiet(bool quiet);

    //! Pops the connection state.
    //! \remark Leaves a null state when the stack becomes empty.
    //! \sa #PushState(const StatePtr&)
    //! \sa #PopStateUntil(const StatePtr&)
    //! \sa #SetState(const StatePtr&)
    void PopState();

    //! Pops connection states until \a target is current.
    //! \param target the state to resume
    //! \remark Logs and leaves a null state if \a target is missing.
    //! \sa #PopState()
    void PopStateUntil(const StatePtr& target);

    //! Pops connection states until the named state is current.
    //! \param stateName the repository name to resume
    //! \sa #PopStateUntil(const StatePtr&)
    void PopStateUntilByName(const String& stateName);

    //! Pushes a connection state.
    //! \param state the state to enter
    //! \remark Does not run FocusLost on the covered state.
    //! \sa #PopState()
    //! \sa #SetState(const StatePtr&)
    void PushState(const StatePtr& state);

    //! Pushes a connection state by name.
    //! \param stateName the repository name of the state to enter
    //! \sa #PushState(const StatePtr&)
    void PushStateByName(const String& stateName);

    //! Sets the connection state.
    //! \param state the state to enter
    //! \remark Drains the connection-state stack first.
    //! \sa #GetState() const
    //! \sa #PushState(const StatePtr&)
    //! \sa #SetStateByName(const String&)
    void SetState(const StatePtr& state);

    //! Sets the connection state by name.
    //! \param stateName the repository name of the state to enter
    //! \sa #GetState() const
    //! \sa #SetState(const StatePtr&)
    void SetStateByName(const String& stateName);

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

    //! The repository key of the thing being edited, if any.
    //! \remark Empty when the edit draft is new (not cloned from a named thing).
    //! \sa #GetEditName() const
    //! \sa #SetEditName(const String&)
    String editName_;

    //! The enumeration being edited.
    //! \sa #GetEditEnumeration() const
    //! \sa #SetEditEnumeration(const EnumerationPtr&)
    EnumerationPtr editEnumeration_;


    //! The connection state being edited.
    //! \sa #GetEditState() const
    //! \sa #SetEditState(const StatePtr&)
    StatePtr editState_;

    //! The multi-step edit string.
    //! \remark Login name, password confirm, and similar.
    //! \sa #GetEditString() const
    //! \sa #SetEditString(const String&)
    String editString_;

    //! The user being edited.
    //! \sa #GetEditUser() const
    //! \sa #SetEditUser(const UserPtr&)
    UserPtr editUser_;

    //! The game state.
    Game& game_;

    //! The input buffer.
    //! \remark Used by \ref boost::asio::async_read().
    StreamBuf input_;

    //! The line input buffer.
    std::ostringstream lineInput_;

    //! The descriptor-owned editor.
    //! \sa #ClearEditor()
    //! \sa #EnsureEditor()
    //! \sa #GetEditor() const
    EditorPtr editor_;

    //! The descriptor-owned menu.
    //! \sa #ClearMenu()
    //! \sa #EnsureMenu()
    //! \sa #GetMenu() const
    MenuPtr menu_;

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

    //! The connection state.
    //! \remark Mirrors the front of \c stateStack_.
    //! \sa #GetState() const
    //! \sa #SetState(const StatePtr&)
    //! \sa #SetStateByName(const String&)
    StatePtr state_;

    //! The connection-state stack.
    //! \remark Front is the current state.
    std::deque<StatePtr> stateStack_;

    //! The TELNET terminal type.
    //! \sa #GetTerminalType() const
    //! \sa #SetTerminalType(const String&)
    String terminalType_;

    //! The attached user.
    //! \sa #GetUser() const
    //! \sa #Login(const UserPtr&)
    UserPtr user_;

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

    //! Whether deferred output flush is already posted.
    //! \remark Coalesces WriteRaw posts before async_write.
    bool writeFlushPosted_;

    //! Whether an asynchronous write is pending.
    bool writePending_;

    //! Ends the current line if needed.
    void EndLine();

    //! Configures an asynchronous read.
    void InitAsyncRead();

    //! Configures an asynchronous write.
    void InitAsyncWrite();

    //! Processes line input.
    //! \param lineReceived the line input to process
    void ReceiveLine(const String& lineReceived);

    //! Re-runs Focus after an editor finish.
    //! \remark Auto-clears with a bookkeeping log if Lua did not call clear_editor.
    void ResumeAfterEditor();

    //! Runs a connection-state Lua hook with \c d, \c line, and \c Q.
    //! \param hook the Lua source to execute
    //! \param hookName hook label appended to the Caller identity (\c Focus, \c FocusLost, \c Received)
    //! \param line the input line for \c line
    //! \return \c true if the hook is empty or executed successfully
    bool RunStateHook(
	const String& hook,
	const String& hookName,
	const String& line = String());
};
//! \}

}; // namespace Net
}; // namespace Scratch

#endif // _SCRATCH_DESCRIPTOR_HPP_
