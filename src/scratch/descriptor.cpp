//! \file descriptor.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_DESCRIPTOR_CPP_

#include <scratch/color.hpp>
#include <scratch/descriptor.hpp>
#include <scratch/descriptor_bindings.hpp>
#include <scratch/game.hpp>
#include <scratch/logger.hpp>
#include <scratch/lua.hpp>
#include <scratch/protocol_telnet.hpp>
#include <scratch/scratch.hpp>
#include <scratch/state.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/user.hpp>

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
	editName_(),
	editState_(),
	editString_(),
	editUser_(),
	game_(game),
	input_(),
	lineInput_(),
	name_(),
	output_(),
	promptBit_(true),
	protocol_(),
	socket_(std::move(socket)),
	state_(),
	terminalType_(),
	user_(),
	windowHeight_(24),
	windowWidth_(80),
	writeFlushPosted_(false),
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
//! \sa #Login(const UserPtr&)
void Descriptor::Close() noexcept {
    if (!socket_.is_open())
	return;

    LOGGER_NETWORK() << "Descriptor " << name_ << " disconnected.";

    if (user_) {
	user_->SetLastLogout(std::time(nullptr));
	game_.GetUsers().Save(user_->GetName());
    }
    user_.reset();
    editUser_.reset();
    editString_.clear();
    editName_.clear();

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
//! \param color the color
const char *Descriptor::GetColor(const int color) const noexcept {
    if (!colorBit_)
	return "";

    auto id = static_cast<Color::ColorEnum>(color);
    if (user_) {
	const auto& metaColors = user_->GetMetaColors();
	auto found = metaColors.find(id);
	if (found != metaColors.end())
	    return this->GetColor(found->second);
    }

    switch (static_cast<int>(id)) {
    case Color::C_CHARCOAL:		return "\x1b[0;30m";
    case Color::C_CRIMSON:		return "\x1b[0;31m";
    case Color::C_FOREST:		return "\x1b[0;32m";
    case Color::C_OCHRE:		return "\x1b[0;33m";
    case Color::C_INDIGO:		return "\x1b[0;34m";
    case Color::C_PURPLE:		return "\x1b[0;35m";
    case Color::C_TEAL:			return "\x1b[0;36m";
    case Color::C_SILVER:		return "\x1b[0;37m";
    case Color::C_GRAY:			return "\x1b[1;30m";
    case Color::C_PINK:			return "\x1b[1;31m";
    case Color::C_LIME:			return "\x1b[1;32m";
    case Color::C_AMBER:		return "\x1b[1;33m";
    case Color::C_AZURE:		return "\x1b[1;34m";
    case Color::C_VIOLET:		return "\x1b[1;35m";
    case Color::C_AQUA:			return "\x1b[1;36m";
    case Color::C_SNOW:			return "\x1b[1;37m";
    case Color::C_NORMAL:		return "\x1b[0m";
    case Color::C_EMPHASIS:		return this->GetColor(Color::C_LIME);
    case Color::C_ENUM:			return this->GetColor(Color::C_PURPLE);
    case Color::C_FAILED:		return this->GetColor(Color::C_CRIMSON);
    case Color::C_KEY:			return this->GetColor(Color::C_CRIMSON);
    case Color::C_NUMBER:		return this->GetColor(Color::C_TEAL);
    case Color::C_OKAY:			return this->GetColor(Color::C_TEAL);
    case Color::C_NAME:			return this->GetColor(Color::C_PINK);
    case Color::C_PERCENT:		return this->GetColor(Color::C_CRIMSON);
    case Color::C_PROMPT:		return this->GetColor(Color::C_FOREST);
    case Color::C_PUNCTUATION:		return this->GetColor(Color::C_GRAY);
    case Color::C_TEXT:			return this->GetColor(Color::C_OCHRE);
    case Color::C_YESNO:		return this->GetColor(Color::C_PURPLE);
    default:				return "";
    }
}

//! Delivers one application-data byte from the protocol.
//! \param byteReceived the byte to deliver
void Descriptor::DeliverByte(const std::uint8_t byteReceived) {
    if (this->Closed()) {
	LOGGER_NETWORK() << "Descriptor " << name_ << " already closed.";
    } else if (std::strchr("\b\x7f", byteReceived) != nullptr) {
	this->Backspace();
    } else if (byteReceived == '\n') {
	const auto line = lineInput_.str();
	this->BackspaceLine();
	this->ReceiveLine(line);
    } else if (std::isprint(byteReceived) && byteReceived != '\r') {
	if (lineInput_.str().size() < MaxInput)
	    lineInput_ << static_cast<char>(byteReceived);
    }
}

//! Logs in the specified user, setting LastLogin and persisting.
//! \param user the user to log in
//! \sa #Close()
//! \sa #GetUser() const
void Descriptor::Login(const UserPtr& user) noexcept {
    if (!user) {
	LOGGER_ASSERT() << "Descriptor " << name_ << " login with null user.";
    } else if (user_ != user) {
	// Log out whoever was previously attached before logging in the
	// newly-specified user; both edges get their own timestamp and save.
	if (user_) {
	    user_->SetLastLogout(std::time(nullptr));
	    game_.GetUsers().Save(user_->GetName());
	}

	user_ = user;
	user_->SetLastLogin(std::time(nullptr));
	game_.GetUsers().Save(user_->GetName());
    }
}

//! Writes to the descriptor.
//! \param message the message to print
void Descriptor::Print(const String& message) noexcept {
    if (this->Closed())
	return;

    this->EndLine();
    this->Write(message);
    if (!message.empty())
	promptBit_ = message.back() == '\n';
}

//! Writes to the descriptor.
//! \param format the printf-style format specifier
void Descriptor::PrintFormat(const String& format, ...) noexcept {
    if (this->Closed())
	return;

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

    this->EndLine();
    this->Write(message);
    if (N > 0)
	promptBit_ = message[N - 1] == '\n';
}

//! Sets the prompt bit.
//! \param promptBit the prompt bit value
//! \sa #GetPromptBit() const
void Descriptor::SetPromptBit(const bool promptBit) noexcept {
    if (!promptBit)
	return;

    const auto self = this->shared_from_this();
    boost::asio::post(game_.GetIoContext(), std::function<void()>([self]() {
	if (!self->Closed() && !self->game_.GetShutdown() &&
	    self->state_ && self->state_->GetPromptBit() &&
	    !self->writePending_ && !self->output_.size())
	    self->WritePrompt();
    }));
}

//! Enables or disables Quiet (server echo / hidden client local echo).
//! \param quiet whether Quiet is enabled
void Descriptor::SetQuiet(bool quiet) {
    protocol_->SetQuiet(quiet);
}

//! Sets the connection state.
//! \param state the state to enter
//! \sa #GetState() const
//! \sa #SetStateByName(const String&)
void Descriptor::SetState(const StatePtr& state) {
    const auto lastState = state_;
    const bool lastQuiet = lastState && lastState->GetQuietBit();

    // Run focus lost hook.
    if (lastState) {
	if (!this->RunStateHook(lastState->GetFocusLost(), "FocusLost"))
	    return;
	if (state_ != lastState)
	    return;
    }

    state_ = state;

    // Run focus hook.
    if (state_)
	this->RunStateHook(state_->GetFocus(), "Focus");
    if (state_ != state)
	return;

    // Set quiet and prompt bits as needed.
    const bool quiet = state_ && state_->GetQuietBit();
    if (lastQuiet != quiet)
	this->SetQuiet(quiet);
    if (state_ && state_->GetPromptBit() &&
	!writePending_ && !output_.size())
	this->WritePrompt();
}

//! Sets the connection state by name.
//! \param stateName the repository name of the state to enter
//! \sa #GetState() const
//! \sa #SetState(const StatePtr&)
void Descriptor::SetStateByName(const String& stateName) {
    auto state = game_.GetStates().Get(stateName);
    if (!state) {
	LOGGER_NETWORK() << "Unknown connection state '" << stateName << "'.";
    } else {
	this->SetState(state);
    }
}

//! Begins asynchronous I/O after the descriptor is indexed by the game.
void Descriptor::Start() {
    protocol_->OnStart();
    this->SetState(game_.GetStates().Get("Login"));
    this->InitAsyncRead();
    this->Write("");
}

//! Writes application output through the protocol.
//! \param message the message to write
void Descriptor::Write(const String& message) {
    if (this->Closed())
	return;
    protocol_->Send(message);
}

//! Writes the prompt.
void Descriptor::WritePrompt() {
    if (!state_ || !state_->GetPromptBit())
	return;

    this->EndLine();

    char message[MaxString] = {'\0'};
    std::snprintf(message, sizeof(message), "%s:ScratchMUD:> %s",
	this->GetColor(Color::C_PROMPT),
	this->GetColor(Color::C_NORMAL));
    this->Write(message);

    // Restore interrupted input.
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

	// Coalesce queued WriteRaw posts before async_write.
	// Must not mutate streambuf during in-flight write.
	if (!self->Closed() && !self->writePending_ &&
	    self->output_.size() && !self->writeFlushPosted_) {
	    self->writeFlushPosted_ = true;
	    boost::asio::post(self->game_.GetIoContext(), std::function<void()>([self]() {
		self->writeFlushPosted_ = false;
		if (!self->Closed() && !self->game_.GetShutdown() &&
		    !self->writePending_ && self->output_.size())
		    self->InitAsyncWrite();
	    }));
	}
    }));
}

//! Ends the current line if needed.
void Descriptor::EndLine() {
    if (promptBit_)
	return;
    this->Write("\r\n");
    promptBit_ = true;
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

	    // Continue draining, or show the prompt.
	    if (!self->game_.GetShutdown() && !self->Closed() && self->output_.size())
		self->InitAsyncWrite();
	    else if (!self->game_.GetShutdown() && !self->Closed() &&
		     self->state_ && self->state_->GetPromptBit() && self->promptBit_)
		self->WritePrompt();
	}));
}

//! Processes line input.
//! \param lineReceived the line input to process
void Descriptor::ReceiveLine(const String& lineReceived) {
    if (this->Closed()) {
	LOGGER_NETWORK() << "Descriptor " << name_ << " already closed.";
    } else if (!state_) {
	LOGGER_NETWORK() << "Descriptor " << name_ << " has no connection state; closing.";
	this->Close();
    } else {
	const auto received = state_->GetReceived();
	if (!received.empty())
	    this->RunStateHook(received, "Received", lineReceived);
	if (!this->Closed() && state_->GetPromptBit())
	    this->SetPromptBit(true);
    }
}

//! Runs a connection-state Lua hook with \c d, \c line, and \c Q.
//! \param hook the Lua source to execute
//! \param hookName hook label appended to the Execute caller (\c Focus, \c FocusLost, \c Received)
//! \param line the input line to inject as global \c line
//! \return \c true if the hook is empty or executed successfully
bool Descriptor::RunStateHook(
	const String& hook,
	const String& hookName,
	const String& line) {
    if (hook.empty())
	return true;

    auto& lua = game_.GetLua();
    // Save outer hook globals.
    lua.GetGlobal("d");
    lua.GetGlobal("line");
    lua.GetGlobal("Q");
    Scratch::Scripting::DescriptorBindings::Push(
	lua,
	this->shared_from_this());
    lua.SetGlobal("d");
    lua.PushString(line);
    lua.SetGlobal("line");
    const bool ok = lua.Execute(name_ + ":" + hookName, hook);
    // Restore.
    lua.SetGlobal("Q");
    lua.SetGlobal("line");
    lua.SetGlobal("d");
    return ok;
}

}; // namespace Net
}; // namespace Scratch
