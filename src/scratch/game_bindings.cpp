//! \file game_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_GAME_BINDINGS_CPP_

#include <scratch/descriptor.hpp>
#include <scratch/descriptor_bindings.hpp>
#include <scratch/game.hpp>
#include <scratch/game_bindings.hpp>
#include <scratch/logger.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>
#include <scratch/state_bindings.hpp>
#include <scratch/string.hpp>
#include <scratch/user_bindings.hpp>

namespace Scratch {
namespace Scripting {

//! Handles lua broadcast.
//! \param L the \c lua_State
static int BroadcastProxy(lua_State* L) {
    auto& game = Lua::CheckGame(L);
    const int howMany = lua_gettop(L);
    luaL_Buffer buffer;
    luaL_buffinit(L, &buffer);
    for (auto n = 1; n <= howMany; ++n) {
	if (n > 1)
	    luaL_addchar(&buffer, '\t');
	luaL_tolstring(L, n, nullptr);
	luaL_addvalue(&buffer);
    }
    luaL_pushresult(&buffer);
    const String message = Lua::CheckString(L, -1);
    lua_pop(L, 1);

    for (auto& d: game.GetDescriptors()) {
	if (d && !d->Closed())
	    d->Print(message);
    }
    return 0;
}

//! Handles lua get_descriptor_names.
//! \param L the \c lua_State
static int DescriptorNamesProxy(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto& game = Lua::CheckGame(L);
    StringSetCi names;
    for (auto& d: game.GetDescriptors()) {
	if (d && !d->Closed()) {
	    names.insert(d->GetName());
	}
    }
    lua.PushStringSet(std::move(names));
    return 1;
}

//! Handles lua get_descriptor.
//! \param L the \c lua_State
static int GetDescriptorProxy(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto& game = Lua::CheckGame(L);
    auto descriptor = game.GetDescriptor(Lua::CheckString(L, 1));
    DescriptorBindings::Push(lua, std::move(descriptor));
    return 1;
}

//! Handles lua get_states.
//! \param L the \c lua_State
static int GetStatesProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "get_states expects no arguments");
    auto& lua = Lua::CheckLua(L);
    StateBindings::PushRepository(lua);
    return 1;
}

//! Handles lua get_users.
//! \param L the \c lua_State
static int GetUsersProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "get_users expects no arguments");
    auto& lua = Lua::CheckLua(L);
    UserBindings::PushRepository(lua);
    return 1;
}

//! Handles lua now — current Unix time.
//! \param L the \c lua_State
static int NowProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "now expects no arguments");
    auto& lua = Lua::CheckLua(L);
    lua.PushInt(static_cast<lua_Integer>(std::time(nullptr)));
    return 1;
}

//! Handles lua date(timestamp) — formats a Unix time for display.
//! \param L the \c lua_State
static int DateProxy(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "date expects 1 argument");
    const auto value = static_cast<std::time_t>(luaL_checkinteger(L, 1));
    auto& lua = Lua::CheckLua(L);

    struct tm time;
    if (localtime_r(&value, &time) != &time) {
	LOGGER_ASSERT() << "localtime_r() failed: errno=" << errno << ".";
	lua.PushString(String());
	return 1;
    }
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &time);
    lua.PushString(String(buffer));
    return 1;
}

//! Handles lua print — writes to LOGGER_LUA.
//! \param L the \c lua_State
static int PrintProxy(lua_State* L) {
    const int howMany = lua_gettop(L);
    luaL_Buffer buffer;
    luaL_buffinit(L, &buffer);
    for (auto n = 1; n <= howMany; ++n) {
	if (n > 1)
	    luaL_addchar(&buffer, '\t');
	luaL_tolstring(L, n, nullptr);
	luaL_addvalue(&buffer);
    }
    luaL_pushresult(&buffer);
    LOGGER_LUA() << Lua::CheckString(L, -1);
    lua_pop(L, 1);
    return 0;
}

//! Handles lua shutdown.
//! \param L the \c lua_State
static int ShutdownProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "shutdown expects no arguments");

    Lua::CheckGame(L).SetShutdown(true);
    return 0;
}

//! Handles lua crypt(plaintext [, salt]).
//! \param L the \c lua_State
static int CryptProxy(lua_State* L) {
    const int argc = lua_gettop(L);
    if (argc != 1 && argc != 2)
	return luaL_error(L, "crypt expects 1 or 2 arguments");
    luaL_checktype(L, 1, LUA_TSTRING);
    const auto plaintext = Lua::CheckString(L, 1);
    String salt;
    if (argc == 2) {
	luaL_checktype(L, 2, LUA_TSTRING);
	salt = Lua::CheckString(L, 2);
    }
    auto& lua = Lua::CheckLua(L);
    lua.PushString(Scratch::Algorithm::StringCryptCopy(plaintext, salt));
    return 1;
}

//! Registers Game free functions on \p lua.
//! \param lua the Lua facade
void GameBindings::Register(Lua& lua) {
    lua.Register("broadcast", BroadcastProxy);
    lua.Register("crypt", CryptProxy);
    lua.Register("date", DateProxy);
    lua.Register("get_descriptor", GetDescriptorProxy);
    lua.Register("get_descriptor_names", DescriptorNamesProxy);
    lua.Register("get_states", GetStatesProxy);
    lua.Register("get_users", GetUsersProxy);
    lua.Register("now", NowProxy);
    lua.Register("print", PrintProxy);
    lua.Register("shutdown", ShutdownProxy);
}

}; // namespace Scripting
}; // namespace Scratch
