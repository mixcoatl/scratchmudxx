//! \file command_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_COMMAND_BINDINGS_CPP_

#include <scratch/action.hpp>
#include <scratch/color.hpp>
#include <scratch/command.hpp>
#include <scratch/command_bindings.hpp>
#include <scratch/descriptor_bindings.hpp>
#include <scratch/trust.hpp>
#include <scratch/game.hpp>
#include <scratch/instance.hpp>
#include <scratch/lua.hpp>
#include <scratch/repository.hpp>
#include <scratch/scratch.hpp>
#include <scratch/social.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Scripting {

//! Metatable name for Command userdata.
const char CommandBindings::MetaName[] = "Scratch.Command";

//! Metatable name for CommandRepository userdata.
const char CommandBindings::RepositoryMetaName[] = "Scratch.CommandRepository";

//! Metatable name for Social userdata.
const char CommandBindings::SocialMetaName[] = "Scratch.Social";

using ActionParam = Scratch::Core::ActionParam;
using Color = Scratch::Net::Color;
using CommandRepositoryPtr = Scratch::Core::CommandRepositoryPtr;
using Trust = Scratch::Core::Trust;
using Instance = Scratch::Core::Instance;
using InstancePtr = Scratch::Core::InstancePtr;
using WeakCommandPtr = std::weak_ptr<Command>;
using WeakInstancePtr = std::weak_ptr<Instance>;
using WeakSocialPtr = std::weak_ptr<Social>;

CommandRepository& CommandBindings::CheckRepository(
	lua_State* L,
	const int index) {
    return *Lua::CheckWeakUserdata<CommandRepository>(
	L, RepositoryMetaName, "invalid command repository", index);
}

void CommandBindings::PushRepository(Lua& lua) {
    lua.PushUserdata(lua.GetGame().GetCommands(), RepositoryMetaName);
}

//! Sets a Command name.
static void CommandSetName(
    CommandPtr command,
    Game& game,
    String name) {
    auto repo = game.GetCommands();
    if (!repo->IsValidThingId(name))
	throw std::invalid_argument("invalid command id");
    if (repo->Contains(command)) {
	throw std::runtime_error("cannot set_name on a live command");
    }
    command->SetName(name);
}

//! Installs the A targets table.
static void RegisterTargetsTable(Lua& lua) {
    auto* L = lua.GetState();
    lua_createtable(L, 0, 6);
    lua_pushinteger(L, Scratch::Core::ACT_NOREPEAT);
    lua_setfield(L, -2, "NOREPEAT");
    lua_pushinteger(L, Scratch::Core::ACT_TOALL);
    lua_setfield(L, -2, "TO_ALL");
    lua_pushinteger(L, Scratch::Core::ACT_TOCHAR);
    lua_setfield(L, -2, "TO_CHAR");
    lua_pushinteger(L, Scratch::Core::ACT_TONOTVICT);
    lua_setfield(L, -2, "TO_NOTVICT");
    lua_pushinteger(L, Scratch::Core::ACT_TOROOM);
    lua_setfield(L, -2, "TO_ROOM");
    lua_pushinteger(L, Scratch::Core::ACT_TOVICT);
    lua_setfield(L, -2, "TO_VICT");
    lua.SetSafe("A");
}

//! Registers Command and Social bindings.
//! \param lua the Lua facade
void CommandBindings::Register(Lua& lua) {
    lua.Class<Command>(CommandBindings::MetaName).
	Function("create_social", &Command::CreateSocialProxy).
	Function("get_action", &Command::GetAction).
	Function("get_created", &Command::GetCreated).
	Function("get_created_by", &Command::GetCreatedBy).
	Function("get_keywords", &Command::GetKeywords).
	Function("get_modified", &Command::GetModified).
	Function("get_modified_by", &Command::GetModifiedBy).
	Function("get_name", &Command::GetName).
	Function("get_social", &Command::GetSocial).
	Function("get_trust", &Command::GetTrust).
	Function("perform_social", &Command::PerformSocial, Injected<Game>()).
	Function("set_action", &Command::SetAction).
	Function("set_created", &Command::SetCreated).
	Function("set_created_by", &Command::SetCreatedBy).
	Function("set_keywords", &Command::SetKeywords).
	Function("set_modified", &Command::SetModified).
	Function("set_modified_by", &Command::SetModifiedBy).
	Function("set_name", &CommandSetName, Injected<Game>()).
	Function("set_social", &Command::SetSocial).
	Function("set_trust", &Command::SetTrust);
    lua.Class<Social>(CommandBindings::SocialMetaName).
	Function("get_found", &Social::GetFound).
	Function("get_found_auto", &Social::GetFoundAuto).
	Function("get_no_argument", &Social::GetNoArgument).
	Function("set_found", &Social::SetFound).
	Function("set_found_auto", &Social::SetFoundAuto).
	Function("set_no_argument", &Social::SetNoArgument);
    lua.Function("action", &Game::Action, Optional(), Optional(), Optional());
    lua.Function("dispatch_command", &Game::DispatchCommand);
    lua.Function("get_commands", &Game::GetCommands);
    lua.Function("get_commands_index", &Game::GetCommandsIndex);
    lua.Function("run_command_hook", &Game::RunCommandHook);
    RegisterTargetsTable(lua);
}

}; // namespace Scripting
}; // namespace Scratch
