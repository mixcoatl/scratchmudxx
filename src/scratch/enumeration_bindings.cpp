//! \file enumeration_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_ENUMERATION_BINDINGS_CPP_

#include <scratch/enumeration.hpp>
#include <scratch/enumeration_bindings.hpp>
#include <scratch/game.hpp>
#include <scratch/lua.hpp>
#include <scratch/repository.hpp>
#include <scratch/scratch.hpp>
#include <scratch/storage_file.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Scripting {

//! Metatable name for Enumeration userdata.
const char EnumerationBindings::MetaName[] = "Scratch.Enumeration";

//! Metatable name for EnumerationRepository userdata.
const char EnumerationBindings::RepositoryMetaName[] =
	"Scratch.EnumerationRepository";

// ScratchMUD types.
using EnumerationRepositoryPtr = Scratch::Core::EnumerationRepositoryPtr;
using WeakEnumerationPtr = std::weak_ptr<Enumeration>;
using WeakEnumerationRepositoryPtr = std::weak_ptr<EnumerationRepository>;

static WeakEnumerationRepositoryPtr CheckWeakEnumerationRepositoryPtr(
	lua_State* L,
	const int index = 1);

//! Returns a copy of the weak Enumeration handle stored in userdata.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
static WeakEnumerationPtr CheckWeakEnumerationPtr(
	lua_State* L,
	const int index) {
    return *static_cast<WeakEnumerationPtr*>(
	luaL_checkudata(L, index, EnumerationBindings::MetaName));
}

//! Handles Enumeration userdata garbage collection.
static int EnumerationGc(lua_State* L) {
    static_cast<WeakEnumerationPtr*>(
	luaL_checkudata(L, 1, EnumerationBindings::MetaName))->~WeakEnumerationPtr();
    return 0;
}

//! Handles Enumeration:add_member(member).
static int EnumerationAddMember(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "add_member expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    EnumerationBindings::Check(L, 1)->AddMember(Lua::CheckString(L, 2));
    return 0;
}

//! Handles Enumeration:erase_member(member).
static int EnumerationEraseMember(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "erase_member expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    EnumerationBindings::Check(L, 1)->EraseMember(Lua::CheckString(L, 2));
    return 0;
}

//! Handles Enumeration:get_created().
static int EnumerationGetCreated(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto enumeration = EnumerationBindings::Check(L, 1);
    const auto created = enumeration->GetCreated();
    enumeration.reset();
    lua.PushInt(static_cast<lua_Integer>(created));
    return 1;
}

//! Handles Enumeration:get_created_by().
static int EnumerationGetCreatedBy(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto enumeration = EnumerationBindings::Check(L, 1);
    auto createdBy = enumeration->GetCreatedBy();
    enumeration.reset();
    lua.PushString(std::move(createdBy));
    return 1;
}

//! Handles Enumeration:get_members().
static int EnumerationGetMembers(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto enumeration = EnumerationBindings::Check(L, 1);
    auto members = enumeration->GetMembers();
    enumeration.reset();
    lua.PushStringSet(std::move(members));
    return 1;
}

//! Handles Enumeration:get_modified().
static int EnumerationGetModified(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto enumeration = EnumerationBindings::Check(L, 1);
    const auto modified = enumeration->GetModified();
    enumeration.reset();
    lua.PushInt(static_cast<lua_Integer>(modified));
    return 1;
}

//! Handles Enumeration:get_modified_by().
static int EnumerationGetModifiedBy(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto enumeration = EnumerationBindings::Check(L, 1);
    auto modifiedBy = enumeration->GetModifiedBy();
    enumeration.reset();
    lua.PushString(std::move(modifiedBy));
    return 1;
}

//! Handles Enumeration:get_name().
static int EnumerationGetName(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto enumeration = EnumerationBindings::Check(L, 1);
    auto name = enumeration->GetName();
    enumeration.reset();
    lua.PushString(std::move(name));
    return 1;
}

//! Handles Enumeration:has_member(member).
static int EnumerationHasMember(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "has_member expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto& lua = Lua::CheckLua(L);
    auto enumeration = EnumerationBindings::Check(L, 1);
    const auto member = Lua::CheckString(L, 2);
    const bool present = enumeration->Contains(member);
    enumeration.reset();
    lua.PushBool(present);
    return 1;
}

//! Handles Enumeration:set_created(created).
static int EnumerationSetCreated(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_created expects 1 argument");
    const auto created = static_cast<std::time_t>(luaL_checkinteger(L, 2));
    EnumerationBindings::Check(L, 1)->SetCreated(created);
    return 0;
}

//! Handles Enumeration:set_created_by(created_by).
static int EnumerationSetCreatedBy(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_created_by expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    EnumerationBindings::Check(L, 1)->SetCreatedBy(Lua::CheckString(L, 2));
    return 0;
}

//! Handles Enumeration:set_members(table).
static int EnumerationSetMembers(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_members expects 1 argument");
    luaL_checktype(L, 2, LUA_TTABLE);
    StringSetCi members;
    const int len = static_cast<int>(luaL_len(L, 2));
    for (int i = 1; i <= len; ++i) {
	lua_rawgeti(L, 2, i);
	if (!lua_isstring(L, -1)) {
	    lua_pop(L, 1);
	    members.clear();
	    return luaL_argerror(L, 2, "members must be strings");
	}
	members.insert(Lua::CheckString(L, -1));
	lua_pop(L, 1);
    }
    EnumerationBindings::Check(L, 1)->SetMembers(members);
    return 0;
}

//! Handles Enumeration:set_modified(modified).
static int EnumerationSetModified(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_modified expects 1 argument");
    const auto modified = static_cast<std::time_t>(luaL_checkinteger(L, 2));
    EnumerationBindings::Check(L, 1)->SetModified(modified);
    return 0;
}

//! Handles Enumeration:set_modified_by(modified_by).
static int EnumerationSetModifiedBy(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_modified_by expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    EnumerationBindings::Check(L, 1)->SetModifiedBy(Lua::CheckString(L, 2));
    return 0;
}

//! Handles Enumeration:set_name(name).
static int EnumerationSetName(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_name expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto repo = Lua::CheckGame(L).GetEnumerations();
    auto enumeration = EnumerationBindings::Check(L, 1);
    if (repo->Contains(enumeration)) {
	enumeration.reset();
	return luaL_error(L, "cannot set_name on a live enumeration");
    }
    const auto name = Lua::CheckString(L, 2);
    if (Scratch::Algorithm::StringCompareCi(
	    Scratch::Algorithm::StringSanitizeCopy(name), name)) {
	enumeration.reset();
	return luaL_error(L, "invalid enumeration id");
    }
    enumeration->SetName(name);
    return 0;
}

//! Resolves an Enumeration userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the enumeration
EnumerationPtr EnumerationBindings::Check(
	lua_State* L,
	const int index) {
    EnumerationPtr enumeration = CheckWeakEnumerationPtr(L, index).lock();
    if (!enumeration)
	luaL_argerror(L, index, "invalid enumeration");
    return enumeration;
}

//! Resolves an EnumerationRepository userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the enumeration repository
EnumerationRepository& EnumerationBindings::CheckRepository(
	lua_State* L,
	const int index) {
    EnumerationRepositoryPtr repo =
	CheckWeakEnumerationRepositoryPtr(L, index).lock();
    if (!repo)
	luaL_argerror(L, index, "invalid enumeration repository");
    return *repo;
}

//! Pushes an Enumeration userdata, or nil.
//! \param lua the Lua facade
//! \param enumeration the enumeration to push
void EnumerationBindings::Push(
	Lua& lua,
	EnumerationPtr enumeration) {
    lua.PushUserdata(std::move(enumeration), MetaName);
}

//! Registers Enumeration userdata bindings.
//! \param L the \c lua_State
static void RegisterEnumerationMeta(lua_State* L) {
    luaL_newmetatable(L, EnumerationBindings::MetaName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    static const luaL_Reg methods[] = {
	{"__gc", EnumerationGc},
	{"add_member", EnumerationAddMember},
	{"erase_member", EnumerationEraseMember},
	{"get_created", EnumerationGetCreated},
	{"get_created_by", EnumerationGetCreatedBy},
	{"get_members", EnumerationGetMembers},
	{"get_modified", EnumerationGetModified},
	{"get_modified_by", EnumerationGetModifiedBy},
	{"get_name", EnumerationGetName},
	{"has_member", EnumerationHasMember},
	{"set_created", EnumerationSetCreated},
	{"set_created_by", EnumerationSetCreatedBy},
	{"set_members", EnumerationSetMembers},
	{"set_modified", EnumerationSetModified},
	{"set_modified_by", EnumerationSetModifiedBy},
	{"set_name", EnumerationSetName},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

//! Returns a copy of the weak EnumerationRepository handle stored in userdata.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
static WeakEnumerationRepositoryPtr CheckWeakEnumerationRepositoryPtr(
	lua_State* L,
	const int index) {
    return *static_cast<WeakEnumerationRepositoryPtr*>(
	luaL_checkudata(L, index, EnumerationBindings::RepositoryMetaName));
}

//! Handles EnumerationRepository:clear().
static int EnumerationRepositoryClear(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "clear expects no arguments");
    EnumerationBindings::CheckRepository(L).Clear();
    return 0;
}

//! Handles EnumerationRepository:erase(name).
static int EnumerationRepositoryErase(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "erase expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    const bool erased = EnumerationBindings::CheckRepository(L).Erase(
	Lua::CheckString(L, 2));
    lua.PushBool(erased);
    return 1;
}

//! Handles EnumerationRepository userdata garbage collection.
static int EnumerationRepositoryGc(lua_State* L) {
    static_cast<WeakEnumerationRepositoryPtr*>(
	luaL_checkudata(L, 1, EnumerationBindings::RepositoryMetaName))
	    ->~WeakEnumerationRepositoryPtr();
    return 0;
}

//! Handles EnumerationRepository:get(name).
static int EnumerationRepositoryGet(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "get expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    EnumerationBindings::Push(
	lua,
	EnumerationBindings::CheckRepository(L).Get(Lua::CheckString(L, 2)));
    return 1;
}

//! Handles EnumerationRepository:get_ids().
static int EnumerationRepositoryGetIds(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_ids expects no arguments");
    auto& lua = Lua::CheckLua(L);
    lua.PushStringSet(EnumerationBindings::CheckRepository(L).GetIds());
    return 1;
}

//! Handles EnumerationRepository:load(name).
static int EnumerationRepositoryLoad(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "load expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    lua.PushBool(EnumerationBindings::CheckRepository(L).Load(
	Lua::CheckString(L, 2)));
    return 1;
}

//! Handles EnumerationRepository:load_index().
static int EnumerationRepositoryLoadIndex(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "load_index expects no arguments");
    auto& lua = Lua::CheckLua(L);
    lua.PushBool(EnumerationBindings::CheckRepository(L).LoadIndex());
    return 1;
}

//! Handles EnumerationRepository:save(name).
static int EnumerationRepositorySave(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "save expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    const bool saved = EnumerationBindings::CheckRepository(L).Save(
	Lua::CheckString(L, 2));
    lua.PushBool(saved);
    return 1;
}

//! Handles EnumerationRepository:save_index().
static int EnumerationRepositorySaveIndex(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "save_index expects no arguments");
    auto& lua = Lua::CheckLua(L);
    lua.PushBool(EnumerationBindings::CheckRepository(L).SaveIndex());
    return 1;
}

//! Handles EnumerationRepository:store(name, enumeration).
static int EnumerationRepositoryStore(lua_State* L) {
    if (lua_gettop(L) != 3)
	return luaL_error(L, "store expects 2 arguments");
    luaL_checktype(L, 2, LUA_TSTRING);
    const auto name = Lua::CheckString(L, 2);
    if (Scratch::Algorithm::StringCompareCi(
	    Scratch::Algorithm::StringSanitizeCopy(name), name))
	return luaL_error(L, "invalid enumeration id");
    auto& repo = EnumerationBindings::CheckRepository(L);
    auto enumeration = EnumerationBindings::Check(L, 3);
    repo.Store(name, enumeration);
    return 0;
}

//! Pushes the enumeration repository userdata.
//! \param lua the Lua facade
void EnumerationBindings::PushRepository(Lua& lua) {
    lua.PushUserdata(lua.GetGame().GetEnumerations(), RepositoryMetaName);
}

//! Registers EnumerationRepository userdata bindings.
//! \param L the \c lua_State
static void RegisterEnumerationRepositoryMeta(lua_State* L) {
    luaL_newmetatable(L, EnumerationBindings::RepositoryMetaName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    static const luaL_Reg methods[] = {
	{"__gc", EnumerationRepositoryGc},
	{"clear", EnumerationRepositoryClear},
	{"erase", EnumerationRepositoryErase},
	{"get", EnumerationRepositoryGet},
	{"get_ids", EnumerationRepositoryGetIds},
	{"load", EnumerationRepositoryLoad},
	{"load_index", EnumerationRepositoryLoadIndex},
	{"save", EnumerationRepositorySave},
	{"save_index", EnumerationRepositorySaveIndex},
	{"store", EnumerationRepositoryStore},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

//! Registers Enumeration and EnumerationRepository metatables.
//! \param lua the Lua facade
void EnumerationBindings::Register(Lua& lua) {
    auto* L = lua.GetState();
    RegisterEnumerationMeta(L);
    RegisterEnumerationRepositoryMeta(L);
}

}; // namespace Scripting
}; // namespace Scratch
