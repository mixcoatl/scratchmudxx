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
#include <scratch/instance_bindings.hpp>
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
using InstancePtr = Scratch::Core::InstancePtr;
using WeakCommandPtr = std::weak_ptr<Command>;
using WeakInstancePtr = std::weak_ptr<Instance>;
using WeakSocialPtr = std::weak_ptr<Social>;

//! Handles Command userdata garbage collection.
static int CommandGc(lua_State* L) {
    return Lua::DestroyWeakUserdata<Command>(
	L, CommandBindings::MetaName);
}

//! Handles Social userdata garbage collection.
static int SocialGc(lua_State* L) {
    return Lua::DestroyWeakUserdata<Social>(
	L, CommandBindings::SocialMetaName);
}

CommandPtr CommandBindings::Check(
	lua_State* L,
	const int index) {
    return Lua::CheckWeakUserdata<Command>(
	L, MetaName, "invalid command", index);
}

void CommandBindings::Push(
	Lua& lua,
	CommandPtr command) {
    lua.PushUserdata(std::move(command), MetaName);
}

SocialPtr CommandBindings::CheckSocial(
	lua_State* L,
	const int index) {
    return Lua::CheckWeakUserdata<Social>(
	L, SocialMetaName, "invalid social", index);
}

void CommandBindings::PushSocial(
	Lua& lua,
	SocialPtr social) {
    lua.PushUserdata(std::move(social), SocialMetaName);
}

CommandRepository& CommandBindings::CheckRepository(
	lua_State* L,
	const int index) {
    return *Lua::CheckWeakUserdata<CommandRepository>(
	L, RepositoryMetaName, "invalid command repository", index);
}

void CommandBindings::PushRepository(Lua& lua) {
    lua.PushUserdata(lua.GetGame().GetCommands(), RepositoryMetaName);
}

//! Parses an ActionParam.
//! \param L the \c lua_State
//! \param index the stack index of the value
static ActionParam CheckActionParam(
	lua_State* L,
	const int index) {
    if (lua_isnoneornil(L, index))
	return ActionParam();
    const int type = lua_type(L, index);
    if (type == LUA_TUSERDATA) {
	auto* box = luaL_testudata(L, index, InstanceBindings::MetaName);
	if (!box)
	    luaL_argerror(L, index, "expected instance, string, or number");
	InstancePtr instance = static_cast<WeakInstancePtr*>(box)->lock();
	if (!instance)
	    luaL_argerror(L, index, "invalid instance");
	return ActionParam(instance);
    }
    if (type == LUA_TNUMBER)
	return ActionParam(lua_tonumber(L, index));
    if (type == LUA_TSTRING)
	return ActionParam(Lua::CheckString(L, index));
    luaL_argerror(L, index, "expected instance, string, or number");
    return ActionParam();
}

//! Handles lua run_command_hook(command, performer, line).
static int RunCommandHookProxy(lua_State* L) {
    if (lua_gettop(L) != 3)
	return luaL_error(L, "run_command_hook expects 3 arguments");
    auto command = CommandBindings::Check(L, 1);
    auto performer = InstanceBindings::Check(L, 2);
    luaL_checktype(L, 3, LUA_TSTRING);
    Lua::CheckGame(L).RunCommandHook(
	    command, performer, Lua::CheckString(L, 3));
    return 0;
}

//! Handles Command:get_action().
static int CommandGetAction(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto command = CommandBindings::Check(L, 1);
    auto action = command->GetAction();
    command.reset();
    lua.PushString(std::move(action));
    return 1;
}

//! Handles Command:get_created().
static int CommandGetCreated(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto command = CommandBindings::Check(L, 1);
    const auto created = command->GetCreated();
    command.reset();
    lua.PushInt(static_cast<lua_Integer>(created));
    return 1;
}

//! Handles Command:get_created_by().
static int CommandGetCreatedBy(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto command = CommandBindings::Check(L, 1);
    auto createdBy = command->GetCreatedBy();
    command.reset();
    lua.PushString(std::move(createdBy));
    return 1;
}

//! Handles Command:get_keywords().
static int CommandGetKeywords(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto command = CommandBindings::Check(L, 1);
    auto keywords = command->GetKeywords();
    command.reset();
    lua.PushStringSet(std::move(keywords));
    return 1;
}

//! Handles Command:get_modified().
static int CommandGetModified(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto command = CommandBindings::Check(L, 1);
    const auto modified = command->GetModified();
    command.reset();
    lua.PushInt(static_cast<lua_Integer>(modified));
    return 1;
}

//! Handles Command:get_modified_by().
static int CommandGetModifiedBy(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto command = CommandBindings::Check(L, 1);
    auto modifiedBy = command->GetModifiedBy();
    command.reset();
    lua.PushString(std::move(modifiedBy));
    return 1;
}

//! Handles Command:get_name().
static int CommandGetName(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto command = CommandBindings::Check(L, 1);
    auto name = command->GetName();
    command.reset();
    lua.PushString(std::move(name));
    return 1;
}

//! Handles Command:get_social().
static int CommandGetSocial(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto command = CommandBindings::Check(L, 1);
    auto social = command->GetSocial();
    command.reset();
    CommandBindings::PushSocial(lua, std::move(social));
    return 1;
}

//! Handles Command:get_trust().
static int CommandGetTrust(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto command = CommandBindings::Check(L, 1);
    const auto trust = command->GetTrust();
    command.reset();
    if (!Trust::IsDefined(trust)) {
	lua.PushString(String());
	return 1;
    }
    lua.PushString(Trust::ToString(trust));
    return 1;
}

//! Handles Command:create_social().
static int CommandCreateSocial(lua_State* L) {
    auto command = CommandBindings::Check(L, 1);
    if (!command->GetSocial())
	command->SetSocial(std::make_shared<Social>());
    command.reset();
    return 0;
}

//! Handles Command:set_action(action).
static int CommandSetAction(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_action expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    CommandBindings::Check(L, 1)->SetAction(Lua::CheckString(L, 2));
    return 0;
}

//! Handles Command:set_created(created).
static int CommandSetCreated(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_created expects 1 argument");
    const auto created = static_cast<std::time_t>(luaL_checkinteger(L, 2));
    CommandBindings::Check(L, 1)->SetCreated(created);
    return 0;
}

//! Handles Command:set_created_by(created_by).
static int CommandSetCreatedBy(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_created_by expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    CommandBindings::Check(L, 1)->SetCreatedBy(Lua::CheckString(L, 2));
    return 0;
}

//! Handles Command:set_keywords(table).
static int CommandSetKeywords(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_keywords expects 1 argument");
    StringSetCi keywords;
    Lua::CheckStringSet(L, keywords, 2, "keywords must be strings");
    CommandBindings::Check(L, 1)->SetKeywords(keywords);
    return 0;
}

//! Handles Command:set_modified(modified).
static int CommandSetModified(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_modified expects 1 argument");
    const auto modified = static_cast<std::time_t>(luaL_checkinteger(L, 2));
    CommandBindings::Check(L, 1)->SetModified(modified);
    return 0;
}

//! Handles Command:set_modified_by(modified_by).
static int CommandSetModifiedBy(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_modified_by expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    CommandBindings::Check(L, 1)->SetModifiedBy(Lua::CheckString(L, 2));
    return 0;
}

//! Handles Command:set_name(name).
static int CommandSetName(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_name expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    const auto name = Lua::CheckString(L, 2);
    auto repo = Lua::CheckGame(L).GetCommands();
    if (!repo->IsValidThingId(name))
	return luaL_error(L, "invalid command id");
    auto command = CommandBindings::Check(L, 1);
    if (repo->Contains(command)) {
	repo.reset();
	command.reset();
	return luaL_error(L, "cannot set_name on a live command");
    }
    command->SetName(name);
    return 0;
}

//! Handles Command:set_social(social|nil).
static int CommandSetSocial(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_social expects 1 argument");
    auto command = CommandBindings::Check(L, 1);
    if (lua_isnil(L, 2)) {
	command->SetSocial(nullptr);
	return 0;
    }
    command->SetSocial(CommandBindings::CheckSocial(L, 2));
    return 0;
}

//! Handles Command:set_trust(trust).
static int CommandSetTrust(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_trust expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto command = CommandBindings::Check(L, 1);
    const auto name = Lua::CheckString(L, 2);
    if (name.empty()) {
	command->SetTrust(Trust::TRUST_NONE);
	return 0;
    }
    const auto trust = Trust::ByName(name);
    if (!Trust::IsDefined(trust)) {
	command.reset();
	return luaL_argerror(L, 2, "unknown trust");
    }
    command->SetTrust(trust);
    return 0;
}

//! Handles Social:get_found().
static int SocialGetFound(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto social = CommandBindings::CheckSocial(L, 1);
    auto found = social->GetFound();
    social.reset();
    lua.PushString(std::move(found));
    return 1;
}

//! Handles Social:get_found_auto().
static int SocialGetFoundAuto(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto social = CommandBindings::CheckSocial(L, 1);
    auto foundAuto = social->GetFoundAuto();
    social.reset();
    lua.PushString(std::move(foundAuto));
    return 1;
}

//! Handles Social:get_no_argument().
static int SocialGetNoArgument(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto social = CommandBindings::CheckSocial(L, 1);
    auto noArgument = social->GetNoArgument();
    social.reset();
    lua.PushString(std::move(noArgument));
    return 1;
}

//! Handles Social:set_found(text).
static int SocialSetFound(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_found expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    CommandBindings::CheckSocial(L, 1)->SetFound(Lua::CheckString(L, 2));
    return 0;
}

//! Handles Social:set_found_auto(text).
static int SocialSetFoundAuto(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_found_auto expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    CommandBindings::CheckSocial(L, 1)->SetFoundAuto(Lua::CheckString(L, 2));
    return 0;
}

//! Handles Social:set_no_argument(text).
static int SocialSetNoArgument(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_no_argument expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    CommandBindings::CheckSocial(L, 1)->SetNoArgument(Lua::CheckString(L, 2));
    return 0;
}

static void RegisterCommandMeta(lua_State* L) {
    Lua::RegisterMetatable(L, CommandBindings::MetaName);
    static const luaL_Reg methods[] = {
	{"__gc", CommandGc},
	{"create_social", CommandCreateSocial},
	{"get_action", CommandGetAction},
	{"get_created", CommandGetCreated},
	{"get_created_by", CommandGetCreatedBy},
	{"get_keywords", CommandGetKeywords},
	{"get_modified", CommandGetModified},
	{"get_modified_by", CommandGetModifiedBy},
	{"get_name", CommandGetName},
	{"get_social", CommandGetSocial},
	{"get_trust", CommandGetTrust},
	{"set_action", CommandSetAction},
	{"set_created", CommandSetCreated},
	{"set_created_by", CommandSetCreatedBy},
	{"set_keywords", CommandSetKeywords},
	{"set_modified", CommandSetModified},
	{"set_modified_by", CommandSetModifiedBy},
	{"set_name", CommandSetName},
	{"set_social", CommandSetSocial},
	{"set_trust", CommandSetTrust},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

static void RegisterSocialMeta(lua_State* L) {
    Lua::RegisterMetatable(L, CommandBindings::SocialMetaName);
    static const luaL_Reg methods[] = {
	{"__gc", SocialGc},
	{"get_found", SocialGetFound},
	{"get_found_auto", SocialGetFoundAuto},
	{"get_no_argument", SocialGetNoArgument},
	{"set_found", SocialSetFound},
	{"set_found_auto", SocialSetFoundAuto},
	{"set_no_argument", SocialSetNoArgument},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

static int CommandRepositoryGc(lua_State* L) {
    return Lua::DestroyWeakUserdata<CommandRepository>(
	L, CommandBindings::RepositoryMetaName);
}

static int CommandRepositoryErase(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "erase expects 1 argument");
    auto& game = Lua::CheckGame(L);
    CommandBindings::CheckRepository(L).Erase(Lua::CheckString(L, 2));
    game.RebuildCommandIndex();
    return 0;
}

static int CommandRepositoryGet(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "get expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    CommandBindings::Push(
	    lua,
	    CommandBindings::CheckRepository(L).Get(Lua::CheckString(L, 2)));
    return 1;
}

static int CommandRepositoryGetIds(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_ids expects no arguments");
    auto& lua = Lua::CheckLua(L);
    lua.PushStringSet(CommandBindings::CheckRepository(L).GetIds());
    return 1;
}

static int CommandRepositoryLoad(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "load expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    auto& game = Lua::CheckGame(L);
    const bool ok = CommandBindings::CheckRepository(L).Load(
	    Lua::CheckString(L, 2));
    if (ok)
	game.RebuildCommandIndex();
    lua.PushBool(ok);
    return 1;
}

static int CommandRepositoryLoadIndex(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "load_index expects no arguments");
    auto& lua = Lua::CheckLua(L);
    auto& game = Lua::CheckGame(L);
    const bool ok = CommandBindings::CheckRepository(L).LoadIndex();
    if (ok)
	game.RebuildCommandIndex();
    lua.PushBool(ok);
    return 1;
}

static int CommandRepositorySave(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "save expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    lua.PushBool(CommandBindings::CheckRepository(L).Save(
	    Lua::CheckString(L, 2)));
    return 1;
}

static int CommandRepositorySaveIndex(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "save_index expects no arguments");
    auto& lua = Lua::CheckLua(L);
    lua.PushBool(CommandBindings::CheckRepository(L).SaveIndex());
    return 1;
}

static int CommandRepositoryStore(lua_State* L) {
    if (lua_gettop(L) != 3)
	return luaL_error(L, "store expects 2 arguments");
    luaL_checktype(L, 2, LUA_TSTRING);
    const auto name = Lua::CheckString(L, 2);
    auto& game = Lua::CheckGame(L);
    auto& repo = CommandBindings::CheckRepository(L);
    if (!repo.IsValidThingId(name))
	return luaL_error(L, "invalid command id");
    auto command = CommandBindings::Check(L, 3);
    repo.Store(name, command);
    game.RebuildCommandIndex();
    return 0;
}

static void RegisterCommandRepositoryMeta(lua_State* L) {
    Lua::RegisterMetatable(L, CommandBindings::RepositoryMetaName);
    static const luaL_Reg methods[] = {
	{"__gc", CommandRepositoryGc},
	{"erase", CommandRepositoryErase},
	{"get", CommandRepositoryGet},
	{"get_ids", CommandRepositoryGetIds},
	{"load", CommandRepositoryLoad},
	{"load_index", CommandRepositoryLoadIndex},
	{"save", CommandRepositorySave},
	{"save_index", CommandRepositorySaveIndex},
	{"store", CommandRepositoryStore},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

//! Handles lua action(...).
static int ActionProxy(lua_State* L) {
    const int argc = lua_gettop(L);
    if (argc < 4 || argc > 7)
	return luaL_error(L, "action expects 4 to 7 arguments");
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TNUMBER);
    luaL_checktype(L, 3, LUA_TSTRING);

    const auto metaName = Lua::CheckString(L, 1);
    const auto metacolor = Color::ByName(metaName);
    if (Color::ToString(metacolor).empty())
	return luaL_argerror(L, 1, "invalid color");
    const unsigned targets = static_cast<unsigned>(lua_tointeger(L, 2));
    const auto message = Lua::CheckString(L, 3);
    const auto subject = CheckActionParam(L, 4);
    const auto direct = argc >= 5 ? CheckActionParam(L, 5) : ActionParam();
    const auto indirect = argc >= 6 ? CheckActionParam(L, 6) : ActionParam();
    const auto extra = argc >= 7 ? CheckActionParam(L, 7) : ActionParam();

    Lua::CheckGame(L).Action(
	    metacolor, targets, message, subject, direct, indirect, extra);
    return 0;
}

//! Handles lua dispatch_command(performer, line).
static int DispatchCommandProxy(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "dispatch_command expects 2 arguments");
    auto performer = InstanceBindings::Check(L, 1);
    luaL_checktype(L, 2, LUA_TSTRING);
    Lua::CheckGame(L).DispatchCommand(performer, Lua::CheckString(L, 2));
    return 0;
}

//! Handles lua get_commands().
static int GetCommandsProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "get_commands expects no arguments");
    auto& lua = Lua::CheckLua(L);
    CommandBindings::PushRepository(lua);
    return 1;
}

//! Handles lua get_commands_index().
static int GetCommandsIndexProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "get_commands_index expects no arguments");
    auto& lua = Lua::CheckLua(L);
    auto& game = Lua::CheckGame(L);
    lua_createtable(L, 0, static_cast<int>(game.GetCommandsIndex().size()));
    const int table = lua_absindex(L, -1);
    for (const auto& pair: game.GetCommandsIndex()) {
	CommandBindings::Push(lua, pair.second);
	lua_setfield(L, table, pair.first.c_str());
    }
    return 1;
}

//! Handles lua run_social().
static int RunSocialProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "run_social expects no arguments");

    auto& lua = Lua::CheckLua(L);
    auto& game = Lua::CheckGame(L);
    lua.PushCallerEnv();
    if (!lua_istable(L, -1)) {
	lua_pop(L, 1);
	return luaL_error(L, "run_social requires an action hook env");
    }
    const int env = lua_absindex(L, -1);

    lua_getfield(L, env, "command");
    auto command = CommandBindings::Check(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, env, "actor");
    auto* actorBox = luaL_testudata(L, -1, InstanceBindings::MetaName);
    if (!actorBox) {
	lua_pop(L, 2);
	command.reset();
	return luaL_error(L, "run_social requires an instance actor");
    }
    InstancePtr actor = static_cast<WeakInstancePtr*>(actorBox)->lock();
    lua_pop(L, 1);
    if (!actor) {
	lua_pop(L, 1);
	command.reset();
	return luaL_error(L, "invalid actor");
    }

    lua_getfield(L, env, "line");
    String line;
    if (lua_isstring(L, -1))
	line = Lua::CheckString(L, -1);
    lua_pop(L, 2); // line + env

    auto social = command->GetSocial();
    command.reset();
    if (!social)
	return 0;

    game.RunSocial(actor, social, line);
    return 0;
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
    auto* L = lua.GetState();
    RegisterCommandMeta(L);
    RegisterSocialMeta(L);
    RegisterCommandRepositoryMeta(L);

    lua.SetSafe("action", ActionProxy);
    lua.SetSafe("dispatch_command", DispatchCommandProxy);
    lua.SetSafe("get_commands", GetCommandsProxy);
    lua.SetSafe("get_commands_index", GetCommandsIndexProxy);
    lua.SetSafe("run_command_hook", RunCommandHookProxy);
    lua.SetSafe("run_social", RunSocialProxy);

    RegisterTargetsTable(lua);
}

}; // namespace Scripting
}; // namespace Scratch
