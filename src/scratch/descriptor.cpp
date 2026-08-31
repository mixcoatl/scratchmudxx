//! \file descriptor.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_DESCRIPTOR_CPP_

#include <scratch/color.hpp>
#include <scratch/color_bindings.hpp>
#include <scratch/command.hpp>
#include <scratch/config.hpp>
#include <scratch/descriptor.hpp>
#include <scratch/descriptor_bindings.hpp>
#include <scratch/editor.hpp>
#include <scratch/game.hpp>
#include <scratch/instance.hpp>
#include <scratch/logger.hpp>
#include <scratch/lua.hpp>
#include <scratch/menu.hpp>
#include <scratch/player.hpp>
#include <scratch/protocol_telnet.hpp>
#include <scratch/room.hpp>
#include <scratch/room_exit.hpp>
#include <scratch/scratch.hpp>
#include <scratch/state.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/string.hpp>
#include <scratch/user.hpp>
#include <scratch/zone.hpp>

namespace Scratch {
namespace Net {

// ScratchMUD types.
using Color = Scratch::Net::Color;
using Strings = Scratch::Algorithm::Strings;

//! Constructor.
//! \param game the game state
//! \param socket the Boost socket
Descriptor::Descriptor(
	Game& game,
	Socket&& socket) :
	colorBit_(true),
	editCommand_(),
	editExit_(),
	editName_(),
	editPlayer_(),
	editRoom_(),
	editState_(),
	editString_(),
	editUser_(),
	editZone_(),
	game_(game),
	input_(),
	lineInput_(),
	editor_(),
	menu_(),
	name_(),
	output_(),
	promptBit_(true),
	protocol_(),
	socket_(std::move(socket)),
	state_(),
	stateStack_(),
	terminalType_(),
	instance_(),
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

//! Clears the descriptor-owned editor.
//! \sa #EnsureEditor()
//! \sa #GetEditor() const
void Descriptor::ClearEditor() noexcept {
    editor_.reset();
}

//! Clears the descriptor-owned menu.
//! \sa #EnsureMenu()
//! \sa #GetMenu() const
void Descriptor::ClearMenu() noexcept {
    if (this->IsEditorActive())
	return;
    menu_.reset();
}

//! Clears the command edit draft.
void Descriptor::ClearEditCommand() {
    if (this->IsEditorActive())
	return;
    editCommand_.reset();
    editName_.clear();
    editString_.clear();
}

//! Clears the room-exit edit draft.
void Descriptor::ClearEditExit() {
    if (this->IsEditorActive())
	return;
    editExit_.reset();
    editString_.clear();
}

//! Clears the player edit draft.
void Descriptor::ClearEditPlayer() {
    if (this->IsEditorActive())
	return;
    editPlayer_.reset();
    editName_.clear();
    editString_.clear();
}

//! Clears the room edit draft.
void Descriptor::ClearEditRoom() {
    if (this->IsEditorActive())
	return;
    editRoom_.reset();
    editExit_.reset();
    editString_.clear();
}

//! Clears the state edit draft.
void Descriptor::ClearEditState() {
    if (this->IsEditorActive())
	return;
    editState_.reset();
    editName_.clear();
    editString_.clear();
}

//! Clears the user edit draft.
void Descriptor::ClearEditUser() {
    if (this->IsEditorActive())
	return;
    editUser_.reset();
    editName_.clear();
    editString_.clear();
}

//! Clears the zone edit draft.
void Descriptor::ClearEditZone() {
    if (this->IsEditorActive())
	return;
    editZone_.reset();
    editRoom_.reset();
    editExit_.reset();
    editName_.clear();
    editString_.clear();
}

//! Sets the command edit draft.
//! \param editCommand the source command, or null for a blank draft
CommandPtr Descriptor::SetEditCommand(const CommandPtr& editCommand) {
    if (this->IsEditorActive())
	return nullptr;
    const auto originalName = editCommand ? editCommand->GetName() : String();
    editCommand_ = editCommand ?
	std::make_shared<Command>(*editCommand) :
	std::make_shared<Command>();
    editName_ = originalName;
    editString_.clear();
    return editCommand_;
}

//! Sets the room-exit edit draft.
//! \param editExit the source exit, or null for a blank draft
RoomExitPtr Descriptor::SetEditExit(const RoomExitPtr& editExit) {
    if (this->IsEditorActive())
	return nullptr;
    editExit_ = editExit ?
	std::make_shared<RoomExit>(*editExit) :
	std::make_shared<RoomExit>();
    return editExit_;
}

//! Sets the player edit draft.
//! \param editPlayer the source player, or null for a blank draft
PlayerPtr Descriptor::SetEditPlayer(const PlayerPtr& editPlayer) {
    if (this->IsEditorActive())
	return nullptr;
    const auto originalName = editPlayer ? editPlayer->GetName() : String();
    editPlayer_ = editPlayer ?
	std::make_shared<Player>(*editPlayer) :
	std::make_shared<Player>();
    editName_ = originalName;
    editString_.clear();
    return editPlayer_;
}

//! Sets the room edit draft.
//! \param editRoom the source room, or null for a blank draft
RoomPtr Descriptor::SetEditRoom(const RoomPtr& editRoom) {
    if (this->IsEditorActive())
	return nullptr;
    const auto originalName = editRoom ? editRoom->GetName() : String();
    editRoom_ = editRoom ?
	std::make_shared<Room>(*editRoom) :
	std::make_shared<Room>();
    editExit_.reset();
    editName_ = originalName;
    editString_.clear();
    return editRoom_;
}

//! Sets the state edit draft.
//! \param editState the source state, or null for a blank draft
StatePtr Descriptor::SetEditState(const StatePtr& editState) {
    if (this->IsEditorActive())
	return nullptr;
    const auto originalName = editState ? editState->GetName() : String();
    editState_ = editState ?
	std::make_shared<State>(*editState) :
	std::make_shared<State>();
    editName_ = originalName;
    editString_.clear();
    return editState_;
}

//! Sets the user edit draft.
//! \param editUser the source user, or null for a blank draft
UserPtr Descriptor::SetEditUser(const UserPtr& editUser) {
    if (this->IsEditorActive())
	return nullptr;
    const auto originalName = editUser ? editUser->GetName() : String();
    editUser_ = editUser ?
	std::make_shared<User>(*editUser) :
	std::make_shared<User>();
    editName_ = originalName;
    editString_.clear();
    return editUser_;
}

//! Sets the zone edit draft.
//! \param editZone the source zone, or null for a blank draft
ZonePtr Descriptor::SetEditZone(const ZonePtr& editZone) {
    if (this->IsEditorActive())
	return nullptr;
    const auto originalName = editZone ? editZone->GetName() : String();
    editZone_ = editZone ?
	std::make_shared<Zone>(*editZone) :
	std::make_shared<Zone>();
    if (editZone) {
	for (const auto& roomName: editZone_->GetRoomNames()) {
	    auto room = editZone_->GetRoom(roomName);
	    editZone_->StoreRoom(roomName, room);
	}
    }
    editRoom_.reset();
    editExit_.reset();
    editName_ = originalName;
    editString_.clear();
    return editZone_;
}

//! Closes the descriptor.
//! \sa #Closed() const
//! \sa #Login(const UserPtr&)
void Descriptor::Close() noexcept {
    if (!socket_.is_open())
	return;

    LOGGER_NETWORK() << "Descriptor " << name_ << " disconnected.";

    this->ClearEditor();

    this->SetCharacter(nullptr);

    if (user_) {
	user_->SetLastLogout(std::time(nullptr));
	game_.GetUsers()->Save(user_->GetName());
    }
    user_.reset();
    editCommand_.reset();
    editName_.clear();
    editExit_.reset();
    editPlayer_.reset();
    editZone_.reset();
    editRoom_.reset();
    editState_.reset();
    editString_.clear();
    editUser_.reset();
    stateStack_.clear();
    state_.reset();

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

//! Returns whether the descriptor is closed or expired.
//! \param descriptor the descriptor
bool Descriptor::ClosedProxy(
	WeakDescriptorPtr descriptor) noexcept {
    auto live = descriptor.lock();
    return !live || live->Closed();
}

EditorPtr Descriptor::GetEditorProxy(
	WeakDescriptorPtr descriptor) noexcept {
    auto live = descriptor.lock();
    if (!live || !live->editor_ || live->editor_->IsActive())
	return EditorPtr();
    return live->editor_;
}

//! Returns whether the editor is intercepting input.
//! \sa #GetEditor() const
bool Descriptor::IsEditorActive() const noexcept {
    return editor_ && editor_->IsActive();
}

//! Returns the editor, creating one if needed.
//! \sa #ClearEditor()
//! \sa #GetEditor() const
EditorPtr Descriptor::EnsureEditor() {
    if (!editor_)
	editor_ = std::make_shared<Editor>(*this);
    return editor_;
}

//! Returns the menu, creating an empty one if needed.
//! \sa #ClearMenu()
//! \sa #GetMenu() const
MenuPtr Descriptor::EnsureMenu() {
    if (!menu_)
	menu_ = std::make_shared<Menu>();
    return menu_;
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

    if (auto config = game_.GetConfig()) {
	const auto& metaColors = config->GetMetaColors();
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
    case Color::C_RESTRICTED:		return this->GetColor(Color::C_PINK);
    case Color::C_SAY:			return this->GetColor(Color::C_FOREST);
    case Color::C_SOCIAL:		return this->GetColor(Color::C_PURPLE);
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
	// Character detach before account swap.
	this->SetCharacter(nullptr);

	// Prior user logout edge.
	if (user_) {
	    user_->SetLastLogout(std::time(nullptr));
	    game_.GetUsers()->Save(user_->GetName());
	}

	user_ = user;
	user_->SetLastLogin(std::time(nullptr));
	game_.GetUsers()->Save(user_->GetName());
    }
}

//! Creates a character from \p player and attaches it.
//! \param player the player prototype
//! \sa #GetCharacter() const
//! \sa #SetCharacter(const InstancePtr&)
void Descriptor::CreateCharacter(const PlayerPtr& player) noexcept {
    if (this->IsEditorActive() || !player)
	return;
    if (game_.GetInstanceFor(player))
	return;
    auto instance = std::make_shared<Instance>();
    instance->SetPlayer(player);
    this->SetCharacter(instance);
}

//! Sets the attached character.
//! \param instance the instance, or null to clear
//! \remark Does not erase the live instance.
//! \sa #CreateCharacter(const PlayerPtr&)
//! \sa #GetCharacter() const
void Descriptor::SetCharacter(const InstancePtr& instance) noexcept {
    if (this->IsEditorActive())
	return;
    if (instance_ == instance)
	return;

    // Control release, instance remains live.
    if (instance_) {
	auto player = instance_->GetPlayer();
	if (player) {
	    auto players = game_.GetPlayers();
	    if (players)
		players->Save(player->GetName());
	}
	instance_->SetDescriptor(nullptr);
	instance_.reset();
    }

    if (!instance)
	return;

    // Control held elsewhere.
    auto other = instance->GetDescriptor();
    if (other && other.get() != this)
	return;

    // Live insert if needed.
    bool alreadyLive = false;
    if (!instance->GetName().empty()) {
	auto mapped = game_.GetInstance(instance->GetName());
	if (mapped == instance)
	    alreadyLive = true;
	else if (mapped)
	    return;
    }
    if (!alreadyLive) {
	auto world = game_.GetWorld(String());
	if (!world || !world->AddInstance(instance))
	    return;
    }

    instance_ = instance;
    instance->SetDescriptor(this->shared_from_this());
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

//! Writes cells in a column-major fold.
//! \param cells the pre-rendered cell strings
//! \sa #Print(const String&)
//! \sa #GetWindowWidth() const
void Descriptor::PrintColumns(const std::vector<String>& cells) noexcept {
    if (this->Closed() || cells.empty())
	return;

    std::size_t longest = 0;
    for (const auto& cell : cells) {
	longest = std::max(longest,
	    Scratch::Algorithm::Strings::StripCopy(cell).size());
    }
    if (longest == 0)
	return;

    // Gap of two spaces between columns.
    const auto stride = longest + 2;
    auto win = static_cast<std::size_t>(this->GetWindowWidth());
    if (win < 1)
	win = 80;
    const auto nColumns = std::max<std::size_t>(1, win / stride);
    const auto count = cells.size();
    const auto nRows = std::max<std::size_t>(4,
	(count + nColumns - 1) / nColumns);

    String out;
    const auto* cNormal = this->GetColor(Color::C_NORMAL);
    for (std::size_t row = 0; row < nRows; ++row) {
	String line;
	bool any = false;
	for (std::size_t column = 0; column < nColumns; ++column) {
	    const auto itemN = column * nRows + row;
	    if (itemN >= count)
		continue;
	    any = true;
	    auto cell = cells[itemN];
	    const auto visible =
		Scratch::Algorithm::Strings::StripCopy(cell).size();
	    const auto pad = stride > visible ? stride - visible : 0;
	    cell.append(pad, ' ');
	    line += cell;
	}
	if (any) {
	    out += line;
	    out += cNormal;
	    out += "\r\n";
	}
    }
    this->Print(out);
}

//! Writes to the descriptor.
//! \param format the printf-style format specifier
void Descriptor::PrintFormat(const String& format, ...) noexcept {
    if (this->Closed())
	return;

    va_list args;
    va_start(args, format);
    const auto message = Strings::FormatVa(format.c_str(), args);
    va_end(args);

    this->EndLine();
    this->Write(message);
    if (!message.empty())
	promptBit_ = message[message.size() - 1] == '\n';
}

//! Renders the descriptor-owned menu.
//! \return \c false if there is no menu or no prompt
//! \sa #GetMenu() const
bool Descriptor::PrintMenu() {
    if (!menu_)
	return false;
    return menu_->Print(*this);
}

//! Sets the prompt bit.
//! \param promptBit the prompt bit value
//! \sa #GetPromptBit() const
void Descriptor::SetPromptBit(const bool promptBit) noexcept {
    if (!promptBit)
	return;

    const auto self = this->shared_from_this();
    boost::asio::post(game_.GetIoContext(), std::function<void()>([self]() {
	if (self->Closed() || self->game_.GetShutdown() ||
	    self->writePending_ || self->output_.size())
	    return;
	if (self->IsEditorActive()) {
	    self->WritePrompt();
	    return;
	}
	if (self->state_ && self->state_->GetPromptBit())
	    self->WritePrompt();
    }));
}

//! Enables or disables Quiet (server echo / hidden client local echo).
//! \param quiet whether Quiet is enabled
void Descriptor::SetQuiet(bool quiet) {
    protocol_->SetQuiet(quiet);
}

//! Pops the connection state.
//! \remark Leaves a null state when the stack becomes empty.
//! \sa #PushState(const StatePtr&)
//! \sa #PopStateUntil(const StatePtr&)
//! \sa #SetState(const StatePtr&)
void Descriptor::PopState() {
    if (stateStack_.empty())
	return;

    const bool lastQuiet = state_ && state_->GetQuietBit();
    const auto leaving = stateStack_.front();
    if (!this->RunStateHook(leaving->GetFocusLost(), "FocusLost"))
	return;
    if (state_ != leaving)
	return;

    stateStack_.pop_front();
    state_ = stateStack_.empty() ? StatePtr() : stateStack_.front();
    this->ClearEditor();
    this->ClearMenu();

    if (state_) {
	const auto expected = state_;
	this->RunStateHook(expected->GetFocus(), "Focus");
	if (state_ != expected)
	    return;
    }

    const bool quiet = state_ && state_->GetQuietBit();
    if (lastQuiet != quiet)
	this->SetQuiet(quiet);
    if (state_ && state_->GetPromptBit() &&
	!writePending_ && !output_.size())
	this->WritePrompt();
}

//! Pops connection states until \a target is current.
//! \param target the state to resume
//! \remark Logs and leaves a null state if \a target is missing.
//! \sa #PopState()
void Descriptor::PopStateUntil(const StatePtr& target) {
    // Repository canonical.
    const auto wanted = target ?
	game_.GetStates()->Get(target->GetName()) : StatePtr();
    if (!wanted) {
	LOGGER_NETWORK() << "Descriptor " << name_
	    << " pop_state_until with unknown target.";
	return;
    }

    const bool lastQuiet = state_ && state_->GetQuietBit();

    // Same-state reentry.
    if (!stateStack_.empty() && stateStack_.front() == wanted) {
	const auto leaving = stateStack_.front();
	if (!this->RunStateHook(leaving->GetFocusLost(), "FocusLost"))
	    return;
	if (state_ != leaving)
	    return;
	this->ClearEditor();
	this->ClearMenu();
	this->RunStateHook(wanted->GetFocus(), "Focus");
	if (state_ != wanted)
	    return;
	const bool quiet = state_ && state_->GetQuietBit();
	if (lastQuiet != quiet)
	    this->SetQuiet(quiet);
	if (state_ && state_->GetPromptBit() &&
	    !writePending_ && !output_.size())
	    this->WritePrompt();
	return;
    }

    while (!stateStack_.empty() && stateStack_.front() != wanted) {
	const auto leaving = stateStack_.front();
	if (!this->RunStateHook(leaving->GetFocusLost(), "FocusLost"))
	    return;
	if (state_ != leaving)
	    return;
	stateStack_.pop_front();
	state_ = stateStack_.empty() ? StatePtr() : stateStack_.front();
    }

    if (stateStack_.empty()) {
	LOGGER_NETWORK() << "Descriptor " << name_
	    << " pop_state_until missed '" << wanted->GetName() << "'.";
	this->ClearEditor();
	this->ClearMenu();
	if (lastQuiet)
	    this->SetQuiet(false);
	return;
    }

    this->ClearEditor();
    this->ClearMenu();
    this->RunStateHook(wanted->GetFocus(), "Focus");
    if (state_ != wanted)
	return;

    const bool quiet = state_ && state_->GetQuietBit();
    if (lastQuiet != quiet)
	this->SetQuiet(quiet);
    if (state_ && state_->GetPromptBit() &&
	!writePending_ && !output_.size())
	this->WritePrompt();
}

//! Pops connection states until the named state is current.
//! \param stateName the repository name to resume
//! \sa #PopStateUntil(const StatePtr&)
void Descriptor::PopStateUntilByName(const String& stateName) {
    auto state = game_.GetStates()->Get(stateName);
    if (!state) {
	LOGGER_NETWORK() << "Unknown connection state '" << stateName << "'.";
	return;
    }
    this->PopStateUntil(state);
}

//! Pushes a connection state.
//! \param state the state to enter
//! \remark Does not run FocusLost on the covered state.
//! \sa #PopState()
//! \sa #SetState(const StatePtr&)
void Descriptor::PushState(const StatePtr& state) {
    // Repository canonical.
    const auto next = state ?
	game_.GetStates()->Get(state->GetName()) : StatePtr();
    if (!next) {
	if (state)
	    LOGGER_NETWORK() << "Unknown connection state '"
		<< state->GetName() << "'.";
	return;
    }

    const bool lastQuiet = state_ && state_->GetQuietBit();

    // Same-state reentry.
    if (!stateStack_.empty() && stateStack_.front() == next) {
	this->ClearEditor();
	this->ClearMenu();
	this->RunStateHook(next->GetFocus(), "Focus");
	if (state_ != next)
	    return;
	const bool quiet = state_ && state_->GetQuietBit();
	if (lastQuiet != quiet)
	    this->SetQuiet(quiet);
	if (state_ && state_->GetPromptBit() &&
	    !writePending_ && !output_.size())
	    this->WritePrompt();
	return;
    }

    stateStack_.push_front(next);
    state_ = next;
    this->ClearEditor();
    this->ClearMenu();
    this->RunStateHook(next->GetFocus(), "Focus");
    if (state_ != next)
	return;

    const bool quiet = state_ && state_->GetQuietBit();
    if (lastQuiet != quiet)
	this->SetQuiet(quiet);
    if (state_ && state_->GetPromptBit() &&
	!writePending_ && !output_.size())
	this->WritePrompt();
}

//! Pushes a connection state by name.
//! \param stateName the repository name of the state to enter
//! \sa #PushState(const StatePtr&)
void Descriptor::PushStateByName(const String& stateName) {
    auto state = game_.GetStates()->Get(stateName);
    if (!state) {
	LOGGER_NETWORK() << "Unknown connection state '" << stateName << "'.";
    } else {
	this->PushState(state);
    }
}

//! Sets the connection state.
//! \param state the state to enter
//! \remark Drains the connection-state stack first.
//! \sa #GetState() const
//! \sa #PushState(const StatePtr&)
//! \sa #SetStateByName(const String&)
void Descriptor::SetState(const StatePtr& state) {
    // Repository canonical. Null drains stack.
    const auto next = state ?
	game_.GetStates()->Get(state->GetName()) : StatePtr();
    if (state && !next) {
	LOGGER_NETWORK() << "Unknown connection state '"
	    << state->GetName() << "'.";
	return;
    }

    const bool lastQuiet = state_ && state_->GetQuietBit();

    // Same-state reentry.
    if (next && !stateStack_.empty() && stateStack_.front() == next) {
	const auto leaving = stateStack_.front();
	if (!this->RunStateHook(leaving->GetFocusLost(), "FocusLost"))
	    return;
	if (state_ != leaving)
	    return;
	this->ClearEditor();
	this->ClearMenu();
	this->RunStateHook(next->GetFocus(), "Focus");
	if (state_ != next)
	    return;
	const bool quiet = state_ && state_->GetQuietBit();
	if (lastQuiet != quiet)
	    this->SetQuiet(quiet);
	if (state_ && state_->GetPromptBit() &&
	    !writePending_ && !output_.size())
	    this->WritePrompt();
	return;
    }

    while (!stateStack_.empty()) {
	const auto leaving = stateStack_.front();
	if (!this->RunStateHook(leaving->GetFocusLost(), "FocusLost"))
	    return;
	if (state_ != leaving)
	    return;
	stateStack_.pop_front();
	state_ = stateStack_.empty() ? StatePtr() : stateStack_.front();
    }

    if (!next) {
	this->ClearEditor();
	this->ClearMenu();
	if (lastQuiet)
	    this->SetQuiet(false);
	return;
    }

    stateStack_.push_front(next);
    state_ = next;
    this->ClearEditor();
    this->ClearMenu();
    this->RunStateHook(next->GetFocus(), "Focus");
    if (state_ != next)
	return;

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
    auto state = game_.GetStates()->Get(stateName);
    if (!state) {
	LOGGER_NETWORK() << "Unknown connection state '" << stateName << "'.";
    } else {
	this->SetState(state);
    }
}

//! Begins asynchronous I/O after the descriptor is indexed by the game.
void Descriptor::Start() {
    protocol_->OnStart();
    this->SetState(game_.GetStates()->Get(
	game_.GetConfig()->GetBootstrapState()));
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
    if (this->IsEditorActive()) {
	editor_->PrintPrompt();
	return;
    }

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
	    else if (!self->game_.GetShutdown() && !self->Closed() && self->promptBit_) {
		if (self->IsEditorActive())
		    self->WritePrompt();
		else if (self->state_ && self->state_->GetPromptBit())
		    self->WritePrompt();
	    }
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
    } else if (this->IsEditorActive()) {
	if (!editor_->Receive(lineReceived))
	    this->ResumeAfterEditor();
    } else {
	const auto before = state_;
	const auto received = state_->GetReceived();
	if (!received.empty())
	    this->RunStateHook(received, "Received", lineReceived);
	// Transition owns prompt; redisplay only if Received left state unchanged.
	if (!this->Closed() && state_ && state_ == before &&
	    state_->GetPromptBit())
	    this->SetPromptBit(true);
    }
}

//! Re-runs Focus after an editor finish.
//! \remark Auto-clears with a bookkeeping log if Lua did not call clear_editor.
void Descriptor::ResumeAfterEditor() {
    if (this->Closed())
	return;
    if (state_)
	this->RunStateHook(state_->GetFocus(), "Focus");
    if (editor_) {
	LOGGER_NETWORK() << "Descriptor " << name_
	    << " auto-cleared editor after Focus.";
	this->ClearEditor();
    }
}

//! Runs a connection-state Lua hook with \c d, \c line, and \c Q.
//! \param hook the Lua source to execute
//! \param hookName hook label appended to the Caller identity (\c Focus, \c FocusLost, \c Received)
//! \param line the input line for \c line
//! \return \c true if the hook is empty or executed successfully
bool Descriptor::RunStateHook(
	const String& hook,
	const String& hookName,
	const String& line) {
    if (hook.empty())
	return true;

    auto& lua = game_.GetLua();
    Scratch::Scripting::Lua::Caller caller(
	lua,
	name_ + ":" + hookName);
    if (!caller.IsActive())
	return false;
    Scratch::Scripting::DescriptorBindings::Push(
	lua,
	this->shared_from_this());
    lua.SetEnv("d");
    lua.PushString(line);
    lua.SetEnv("line");
    lua.PushStringMap(
	Scratch::Scripting::ColorBindings::Codes(*this));
    lua.SetEnv("Q");
    return lua.Execute(hook);
}

}; // namespace Net
}; // namespace Scratch
