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
#include <scratch/descriptor.hpp>
#include <scratch/descriptor_bindings.hpp>
#include <scratch/enumeration.hpp>
#include <scratch/game.hpp>
#include <scratch/lua.hpp>
#include <scratch/repository.hpp>
#include <scratch/scratch.hpp>
#include <scratch/social.hpp>
#include <scratch/storage_file.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/string.hpp>
#include <scratch/user.hpp>
#include <scratch/user_bindings.hpp>

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
using ThingPtr = Scratch::Core::ThingPtr;
using UserPtr = Scratch::Core::UserPtr;
using WeakCommandPtr = std::weak_ptr<Command>;
using WeakCommandRepositoryPtr = std::weak_ptr<CommandRepository>;
using WeakSocialPtr = std::weak_ptr<Social>;
using WeakUserPtr = std::weak_ptr<User>;

static WeakCommandRepositoryPtr CheckWeakCommandRepositoryPtr(
	lua_State* L,
	const int index = 1);

//! Returns a weak Command handle.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
static WeakCommandPtr CheckWeakCommandPtr(
	lua_State* L,
	const int index) {
    return *static_cast<WeakCommandPtr*>(
	luaL_checkudata(L, index, CommandBindings::MetaName));
}

//! Returns a weak Social handle.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
static WeakSocialPtr CheckWeakSocialPtr(
	lua_State* L,
	const int index) {
    return *static_cast<WeakSocialPtr*>(
	luaL_checkudata(L, index, CommandBindings::SocialMetaName));
}

//! Handles Command userdata garbage collection.
static int CommandGc(lua_State* L) {
    static_cast<WeakCommandPtr*>(
	luaL_checkudata(L, 1, CommandBindings::MetaName))->~WeakCommandPtr();
    return 0;
}

//! Handles Social userdata garbage collection.
static int SocialGc(lua_State* L) {
    static_cast<WeakSocialPtr*>(
	luaL_checkudata(L, 1, CommandBindings::SocialMetaName))->~WeakSocialPtr();
    return 0;
}

CommandPtr CommandBindings::Check(
	lua_State* L,
	const int index) {
    CommandPtr command = CheckWeakCommandPtr(L, index).lock();
    if (!command)
	luaL_argerror(L, index, "invalid command");
    return command;
}

void CommandBindings::Push(
	Lua& lua,
	CommandPtr command) {
    lua.PushUserdata(std::move(command), MetaName);
}

SocialPtr CommandBindings::CheckSocial(
	lua_State* L,
	const int index) {
    SocialPtr social = CheckWeakSocialPtr(L, index).lock();
    if (!social)
	luaL_argerror(L, index, "invalid social");
    return social;
}

void CommandBindings::PushSocial(
	Lua& lua,
	SocialPtr social) {
    lua.PushUserdata(std::move(social), SocialMetaName);
}

CommandRepository& CommandBindings::CheckRepository(
	lua_State* L,
	const int index) {
    CommandRepositoryPtr repo = CheckWeakCommandRepositoryPtr(L, index).lock();
    if (!repo)
	luaL_argerror(L, index, "invalid command repository");
    return *repo;
}

void CommandBindings::PushRepository(Lua& lua) {
    lua.PushUserdata(lua.GetGame().GetCommands(), RepositoryMetaName);
}

static WeakCommandRepositoryPtr CheckWeakCommandRepositoryPtr(
	lua_State* L,
	const int index) {
    return *static_cast<WeakCommandRepositoryPtr*>(
	luaL_checkudata(L, index, CommandBindings::RepositoryMetaName));
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
	auto* box = luaL_testudata(L, index, UserBindings::MetaName);
	if (!box)
	    luaL_argerror(L, index, "expected user, string, or number");
	UserPtr user = static_cast<WeakUserPtr*>(box)->lock();
	if (!user)
	    luaL_argerror(L, index, "invalid user");
	return ActionParam(user);
    }
    if (type == LUA_TNUMBER)
	return ActionParam(lua_tonumber(L, index));
    if (type == LUA_TSTRING)
	return ActionParam(Lua::CheckString(L, index));
    luaL_argerror(L, index, "expected user, string, or number");
    return ActionParam();
}

//! Handles lua run_command_hook(command, performer, line).
static int RunCommandHookProxy(lua_State* L) {
    if (lua_gettop(L) != 3)
	return luaL_error(L, "run_command_hook expects 3 arguments");
    auto command = CommandBindings::Check(L, 1);
    auto* box = luaL_testudata(L, 2, UserBindings::MetaName);
    if (!box)
	return luaL_argerror(L, 2, "expected user");
    UserPtr performer = static_cast<WeakUserPtr*>(box)->lock();
    if (!performer)
	return luaL_argerror(L, 2, "invalid user");
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

//! Handles Command:get_permissions().
static int CommandGetPermissions(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto command = CommandBindings::Check(L, 1);
    auto permissions = command->GetPermissions();
    command.reset();
    lua.PushStringSet(std::move(permissions));
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

//! Handles Command:has_permission(permission).
static int CommandHasPermission(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "has_permission expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto& lua = Lua::CheckLua(L);
    auto command = CommandBindings::Check(L, 1);
    const bool present = command->HasPermission(Lua::CheckString(L, 2));
    command.reset();
    lua.PushBool(present);
    return 1;
}

//! Returns whether \p name is a Permission member.
static bool PermissionCatalogHas(
	Scratch::Core::Game& game,
	const String& name) {
    auto enumeration = game.GetEnumerations()->Get("Permission");
    if (!enumeration)
	return false;
    return enumeration->Contains(name);
}

//! Handles Command:create_social().
static int CommandCreateSocial(lua_State* L) {
    auto command = CommandBindings::Check(L, 1);
    if (!command->GetSocial())
	command->SetSocial(std::make_shared<Social>());
    command.reset();
    return 0;
}

//! Handles Command:add_permission(permission).
static int CommandAddPermission(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "add_permission expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto& game = Lua::CheckGame(L);
    auto command = CommandBindings::Check(L, 1);
    const auto name = Lua::CheckString(L, 2);
    if (!PermissionCatalogHas(game, name)) {
	command.reset();
	return luaL_argerror(L, 2, "unknown permission");
    }
    command->AddPermission(name);
    return 0;
}

//! Handles Command:erase_permission(permission).
static int CommandErasePermission(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "erase_permission expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    CommandBindings::Check(L, 1)->ErasePermission(Lua::CheckString(L, 2));
    return 0;
}

//! Handles Command:set_action(text).
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
    luaL_checktype(L, 2, LUA_TTABLE);
    StringSetCi keywords;
    const int len = static_cast<int>(luaL_len(L, 2));
    for (int i = 1; i <= len; ++i) {
	lua_rawgeti(L, 2, i);
	if (!lua_isstring(L, -1)) {
	    lua_pop(L, 1);
	    keywords.clear();
	    return luaL_argerror(L, 2, "keywords must be strings");
	}
	keywords.insert(Lua::CheckString(L, -1));
	lua_pop(L, 1);
    }
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

//! Handles Command:set_permissions(table).
static int CommandSetPermissions(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_permissions expects 1 argument");
    luaL_checktype(L, 2, LUA_TTABLE);
    auto& game = Lua::CheckGame(L);
    StringSetCi permissions;
    const int len = static_cast<int>(luaL_len(L, 2));
    for (int i = 1; i <= len; ++i) {
	lua_rawgeti(L, 2, i);
	if (!lua_isstring(L, -1)) {
	    lua_pop(L, 1);
	    permissions.clear();
	    return luaL_argerror(L, 2, "permissions must be strings");
	}
	const auto name = Lua::CheckString(L, -1);
	lua_pop(L, 1);
	if (!PermissionCatalogHas(game, name)) {
	    permissions.clear();
	    return luaL_argerror(L, 2, "unknown permission");
	}
	permissions.insert(name);
    }
    CommandBindings::Check(L, 1)->SetPermissions(permissions);
    return 0;
}

//! Handles Command:set_name(name).
static int CommandSetName(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_name expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto repo = Lua::CheckGame(L).GetCommands();
    auto command = CommandBindings::Check(L, 1);
    if (repo->Contains(command)) {
	command.reset();
	return luaL_error(L, "cannot set_name on a live command");
    }
    command->SetName(Lua::CheckString(L, 2));
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
    luaL_newmetatable(L, CommandBindings::MetaName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    static const luaL_Reg methods[] = {
	{"__gc", CommandGc},
	{"add_permission", CommandAddPermission},
	{"create_social", CommandCreateSocial},
	{"erase_permission", CommandErasePermission},
	{"get_action", CommandGetAction},
	{"get_created", CommandGetCreated},
	{"get_created_by", CommandGetCreatedBy},
	{"get_keywords", CommandGetKeywords},
	{"get_modified", CommandGetModified},
	{"get_modified_by", CommandGetModifiedBy},
	{"get_name", CommandGetName},
	{"get_permissions", CommandGetPermissions},
	{"get_social", CommandGetSocial},
	{"has_permission", CommandHasPermission},
	{"set_action", CommandSetAction},
	{"set_created", CommandSetCreated},
	{"set_created_by", CommandSetCreatedBy},
	{"set_keywords", CommandSetKeywords},
	{"set_modified", CommandSetModified},
	{"set_modified_by", CommandSetModifiedBy},
	{"set_name", CommandSetName},
	{"set_permissions", CommandSetPermissions},
	{"set_social", CommandSetSocial},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

static void RegisterSocialMeta(lua_State* L) {
    luaL_newmetatable(L, CommandBindings::SocialMetaName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
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
    static_cast<WeakCommandRepositoryPtr*>(
	luaL_checkudata(L, 1, CommandBindings::RepositoryMetaName))
	    ->~WeakCommandRepositoryPtr();
    return 0;
}

static int CommandRepositoryClear(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "clear expects no arguments");
    auto& game = Lua::CheckGame(L);
    CommandBindings::CheckRepository(L).Clear();
    game.RebuildCommandIndex();
    return 0;
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
    if (Scratch::Algorithm::StringCompareCi(
	    Scratch::Algorithm::StringSanitizeCopy(name), name))
	return luaL_error(L, "invalid command id");
    auto& game = Lua::CheckGame(L);
    auto& repo = CommandBindings::CheckRepository(L);
    auto command = CommandBindings::Check(L, 3);
    repo.Store(name, command);
    game.RebuildCommandIndex();
    return 0;
}

static void RegisterCommandRepositoryMeta(lua_State* L) {
    luaL_newmetatable(L, CommandBindings::RepositoryMetaName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    static const luaL_Reg methods[] = {
	{"__gc", CommandRepositoryGc},
	{"clear", CommandRepositoryClear},
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
    if (!Color::IsMeta(metacolor) && !Color::IsReal(metacolor))
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
    auto* box = luaL_testudata(L, 1, UserBindings::MetaName);
    if (!box)
	return luaL_argerror(L, 1, "expected user");
    UserPtr performer = static_cast<WeakUserPtr*>(box)->lock();
    if (!performer)
	return luaL_argerror(L, 1, "invalid user");
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

//! Handles lua get_descriptor_for(thing).
static int GetDescriptorForProxy(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_descriptor_for expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    auto& game = Lua::CheckGame(L);
    ThingPtr thing;
    if (!lua_isnil(L, 1)) {
	auto* box = luaL_testudata(L, 1, UserBindings::MetaName);
	if (!box)
	    return luaL_argerror(L, 1, "expected user");
	thing = static_cast<WeakUserPtr*>(box)->lock();
	if (!thing)
	    return luaL_argerror(L, 1, "invalid user");
    }
    DescriptorBindings::Push(lua, game.GetDescriptorFor(thing));
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
    auto* actorBox = luaL_testudata(L, -1, UserBindings::MetaName);
    if (!actorBox) {
	lua_pop(L, 2);
	command.reset();
	return luaL_error(L, "run_social requires a user actor");
    }
    UserPtr actor = static_cast<WeakUserPtr*>(actorBox)->lock();
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

    lua.PushFunction(ActionProxy);
    lua.SetSafe("action");
    lua.PushFunction(DispatchCommandProxy);
    lua.SetSafe("dispatch_command");
    lua.PushFunction(GetCommandsProxy);
    lua.SetSafe("get_commands");
    lua.PushFunction(GetCommandsIndexProxy);
    lua.SetSafe("get_commands_index");
    lua.PushFunction(GetDescriptorForProxy);
    lua.SetSafe("get_descriptor_for");
    lua.PushFunction(RunCommandHookProxy);
    lua.SetSafe("run_command_hook");
    lua.PushFunction(RunSocialProxy);
    lua.SetSafe("run_social");

    RegisterTargetsTable(lua);
}

}; // namespace Scripting
}; // namespace Scratch
