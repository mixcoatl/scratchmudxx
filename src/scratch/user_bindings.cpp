//! \file user_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_USER_BINDINGS_CPP_

#include <scratch/game.hpp>
#include <scratch/lua.hpp>
#include <scratch/repository.hpp>
#include <scratch/scratch.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/string.hpp>
#include <scratch/user.hpp>
#include <scratch/user_bindings.hpp>

namespace Scratch {
namespace Scripting {

//! Metatable name for User userdata.
const char UserBindings::MetaName[] = "Scratch.User";

//! Metatable name for user repository userdata.
const char UserBindings::RepositoryMetaName[] = "Scratch.UserRepository";

//! ScratchMUD types.
using UserRepositoryPtr = Scratch::Core::UserRepositoryPtr;
using WeakUserPtr = std::weak_ptr<User>;
using WeakUserRepositoryPtr = std::weak_ptr<UserRepository>;

static WeakUserRepositoryPtr CheckWeakUserRepositoryPtr(
	lua_State* L,
	const int index = 1);

//! Returns a copy of the weak User handle stored in userdata.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
static WeakUserPtr CheckWeakUserPtr(
	lua_State* L,
	const int index) {
    return *static_cast<WeakUserPtr*>(
	luaL_checkudata(L, index, UserBindings::MetaName));
}

//! Handles User userdata garbage collection.
static int UserGc(lua_State* L) {
    static_cast<WeakUserPtr*>(
	luaL_checkudata(L, 1, UserBindings::MetaName))->~WeakUserPtr();
    return 0;
}

//! Handles User:get_created().
static int UserGetCreated(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto user = UserBindings::Check(L, 1);
    const auto created = user->GetCreated();
    user.reset();
    lua.PushInt(static_cast<lua_Integer>(created));
    return 1;
}

//! Handles User:get_created_by().
static int UserGetCreatedBy(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto user = UserBindings::Check(L, 1);
    auto createdBy = user->GetCreatedBy();
    user.reset();
    lua.PushString(std::move(createdBy));
    return 1;
}

//! Handles User:get_email().
static int UserGetEmail(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto user = UserBindings::Check(L, 1);
    auto email = user->GetEmail();
    user.reset();
    lua.PushString(std::move(email));
    return 1;
}

//! Handles User:get_gender().
static int UserGetGender(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto user = UserBindings::Check(L, 1);
    auto gender = user->GetGender();
    user.reset();
    lua.PushString(std::move(gender));
    return 1;
}

//! Handles User:get_last_login().
static int UserGetLastLogin(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto user = UserBindings::Check(L, 1);
    const auto lastLogin = user->GetLastLogin();
    user.reset();
    lua.PushInt(static_cast<lua_Integer>(lastLogin));
    return 1;
}

//! Handles User:get_last_logout().
static int UserGetLastLogout(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto user = UserBindings::Check(L, 1);
    const auto lastLogout = user->GetLastLogout();
    user.reset();
    lua.PushInt(static_cast<lua_Integer>(lastLogout));
    return 1;
}

//! Handles User:get_modified().
static int UserGetModified(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto user = UserBindings::Check(L, 1);
    const auto modified = user->GetModified();
    user.reset();
    lua.PushInt(static_cast<lua_Integer>(modified));
    return 1;
}

//! Handles User:get_modified_by().
static int UserGetModifiedBy(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto user = UserBindings::Check(L, 1);
    auto modifiedBy = user->GetModifiedBy();
    user.reset();
    lua.PushString(std::move(modifiedBy));
    return 1;
}

//! Handles User:get_name().
static int UserGetName(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto user = UserBindings::Check(L, 1);
    auto name = user->GetName();
    user.reset();
    lua.PushString(std::move(name));
    return 1;
}

//! Handles User:get_password().
static int UserGetPassword(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto user = UserBindings::Check(L, 1);
    auto password = user->GetPassword();
    user.reset();
    lua.PushString(std::move(password));
    return 1;
}

//! Handles User:set_created(created).
static int UserSetCreated(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_created expects 1 argument");
    const auto created = static_cast<std::time_t>(luaL_checkinteger(L, 2));
    auto user = UserBindings::Check(L, 1);
    user->SetCreated(created);
    return 0;
}

//! Handles User:set_created_by(created_by).
static int UserSetCreatedBy(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_created_by expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto user = UserBindings::Check(L, 1);
    user->SetCreatedBy(Lua::CheckString(L, 2));
    return 0;
}

//! Handles User:set_email(email).
static int UserSetEmail(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_email expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto user = UserBindings::Check(L, 1);
    user->SetEmail(Lua::CheckString(L, 2));
    return 0;
}

//! Handles User:set_gender(gender).
static int UserSetGender(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_gender expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto user = UserBindings::Check(L, 1);
    user->SetGender(Lua::CheckString(L, 2));
    return 0;
}

//! Handles User:set_modified(modified).
static int UserSetModified(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_modified expects 1 argument");
    const auto modified = static_cast<std::time_t>(luaL_checkinteger(L, 2));
    auto user = UserBindings::Check(L, 1);
    user->SetModified(modified);
    return 0;
}

//! Handles User:set_modified_by(modified_by).
static int UserSetModifiedBy(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_modified_by expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto user = UserBindings::Check(L, 1);
    user->SetModifiedBy(Lua::CheckString(L, 2));
    return 0;
}

//! Handles User:set_name(name).
static int UserSetName(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_name expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto& repo = Lua::CheckGame(L).GetUsers();
    auto user = UserBindings::Check(L, 1);
    if (repo.Contains(user)) {
	user.reset();
	return luaL_error(L, "cannot set_name on a live user");
    }
    const auto name = Lua::CheckString(L, 2);
    if (Scratch::Algorithm::StringCompareCi(
	    Scratch::Algorithm::StringSanitizeCopy(name), name)) {
	user.reset();
	return luaL_error(L, "invalid user id");
    }
    user->SetName(name);
    return 0;
}

//! Handles User:set_password(plain).
static int UserSetPassword(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_password expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto& lua = Lua::CheckLua(L);
    auto user = UserBindings::Check(L, 1);
    const bool ok = user->SetPassword(Lua::CheckString(L, 2));
    user.reset();
    lua.PushBool(ok);
    return 1;
}

//! Resolves a User userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the user
UserPtr UserBindings::Check(
	lua_State* L,
	const int index) {
    UserPtr user = CheckWeakUserPtr(L, index).lock();
    if (!user)
	luaL_argerror(L, index, "invalid user");
    return user;
}

//! Pushes a User userdata, or nil.
//! \param lua the Lua facade
//! \param user the user to push
void UserBindings::Push(
	Lua& lua,
	UserPtr user) {
    lua.PushUserdata(std::move(user), MetaName);
}

//! Registers User userdata bindings.
//! \param L the \c lua_State
static void RegisterUserMeta(lua_State* L) {
    luaL_newmetatable(L, UserBindings::MetaName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    static const luaL_Reg methods[] = {
	{"__gc", UserGc},
	{"get_created", UserGetCreated},
	{"get_created_by", UserGetCreatedBy},
	{"get_email", UserGetEmail},
	{"get_gender", UserGetGender},
	{"get_last_login", UserGetLastLogin},
	{"get_last_logout", UserGetLastLogout},
	{"get_modified", UserGetModified},
	{"get_modified_by", UserGetModifiedBy},
	{"get_name", UserGetName},
	{"get_password", UserGetPassword},
	{"set_created", UserSetCreated},
	{"set_created_by", UserSetCreatedBy},
	{"set_email", UserSetEmail},
	{"set_gender", UserSetGender},
	{"set_modified", UserSetModified},
	{"set_modified_by", UserSetModifiedBy},
	{"set_name", UserSetName},
	{"set_password", UserSetPassword},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

//! Resolves a UserRepository userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the repository owned by Game
UserRepository& UserBindings::CheckRepository(
	lua_State* L,
	const int index) {
    UserRepositoryPtr repo = CheckWeakUserRepositoryPtr(L, index).lock();
    if (!repo)
	luaL_argerror(L, index, "invalid user repository");
    return *repo;
}

//! Returns a copy of the weak UserRepository handle stored in userdata.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
static WeakUserRepositoryPtr CheckWeakUserRepositoryPtr(
	lua_State* L,
	const int index) {
    return *static_cast<WeakUserRepositoryPtr*>(
	luaL_checkudata(L, index, UserBindings::RepositoryMetaName));
}

//! Handles UserRepository:clear().
static int UserRepositoryClear(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "clear expects no arguments");
    UserBindings::CheckRepository(L).Clear();
    return 0;
}

//! Handles UserRepository:erase(name).
static int UserRepositoryErase(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "erase expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    const bool erased = UserBindings::CheckRepository(L).Erase(
	Lua::CheckString(L, 2));
    lua.PushBool(erased);
    return 1;
}

//! Handles UserRepository userdata garbage collection.
static int UserRepositoryGc(lua_State* L) {
    static_cast<WeakUserRepositoryPtr*>(
	luaL_checkudata(L, 1, UserBindings::RepositoryMetaName))->~WeakUserRepositoryPtr();
    return 0;
}

//! Handles UserRepository:get(name).
static int UserRepositoryGet(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "get expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    UserBindings::Push(lua, UserBindings::CheckRepository(L).Get(Lua::CheckString(L, 2)));
    return 1;
}

//! Handles UserRepository:get_ids().
static int UserRepositoryGetIds(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_ids expects no arguments");
    auto& lua = Lua::CheckLua(L);
    lua.PushStringSet(UserBindings::CheckRepository(L).GetIds());
    return 1;
}

//! Handles UserRepository:load(name).
static int UserRepositoryLoad(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "load expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    lua.PushBool(UserBindings::CheckRepository(L).Load(Lua::CheckString(L, 2)));
    return 1;
}

//! Handles UserRepository:load_index().
static int UserRepositoryLoadIndex(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "load_index expects no arguments");
    auto& lua = Lua::CheckLua(L);
    lua.PushBool(UserBindings::CheckRepository(L).LoadIndex());
    return 1;
}

//! Handles UserRepository:save(name).
static int UserRepositorySave(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "save expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    const bool saved = UserBindings::CheckRepository(L).Save(
	Lua::CheckString(L, 2));
    lua.PushBool(saved);
    return 1;
}

//! Handles UserRepository:save_index().
static int UserRepositorySaveIndex(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "save_index expects no arguments");
    auto& lua = Lua::CheckLua(L);
    lua.PushBool(UserBindings::CheckRepository(L).SaveIndex());
    return 1;
}

//! Handles UserRepository:store(name, user).
static int UserRepositoryStore(lua_State* L) {
    if (lua_gettop(L) != 3)
	return luaL_error(L, "store expects 2 arguments");
    luaL_checktype(L, 2, LUA_TSTRING);
    const auto name = Lua::CheckString(L, 2);
    if (Scratch::Algorithm::StringCompareCi(
	    Scratch::Algorithm::StringSanitizeCopy(name), name))
	return luaL_error(L, "invalid user id");
    auto& repo = UserBindings::CheckRepository(L);
    auto user = UserBindings::Check(L, 3);
    repo.Store(name, user);
    return 0;
}

//! Pushes the user repository userdata.
//! \param lua the Lua facade
void UserBindings::PushRepository(Lua& lua) {
    lua.PushUserdata(lua.GetGame().GetUsersWeak(), RepositoryMetaName);
}

//! Registers user repository userdata bindings.
//! \param L the \c lua_State
static void RegisterUserRepositoryMeta(lua_State* L) {
    luaL_newmetatable(L, UserBindings::RepositoryMetaName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    static const luaL_Reg methods[] = {
	{"__gc", UserRepositoryGc},
	{"clear", UserRepositoryClear},
	{"erase", UserRepositoryErase},
	{"get", UserRepositoryGet},
	{"get_ids", UserRepositoryGetIds},
	{"load", UserRepositoryLoad},
	{"load_index", UserRepositoryLoadIndex},
	{"save", UserRepositorySave},
	{"save_index", UserRepositorySaveIndex},
	{"store", UserRepositoryStore},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

//! Registers User and UserRepository metatables.
//! \param lua the Lua facade
void UserBindings::Register(Lua& lua) {
    auto* L = lua.GetState();
    RegisterUserMeta(L);
    RegisterUserRepositoryMeta(L);
}

}; // namespace Scripting
}; // namespace Scratch
