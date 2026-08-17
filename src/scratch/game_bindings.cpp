//! \file game_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_GAME_BINDINGS_CPP_

#include <scratch/config_bindings.hpp>
#include <scratch/descriptor.hpp>
#include <scratch/descriptor_bindings.hpp>
#include <scratch/enumeration_bindings.hpp>
#include <scratch/game.hpp>
#include <scratch/game_bindings.hpp>
#include <scratch/instance_bindings.hpp>
#include <scratch/logger.hpp>
#include <scratch/lua.hpp>
#include <scratch/player_bindings.hpp>
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

//! Handles lua get_config.
//! \param L the \c lua_State
static int GetConfigProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "get_config expects no arguments");
    auto& lua = Lua::CheckLua(L);
    ConfigBindings::Push(lua, lua.GetGame().GetConfig());
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

//! Handles lua get_enumerations.
//! \param L the \c lua_State
static int GetEnumerationsProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "get_enumerations expects no arguments");
    auto& lua = Lua::CheckLua(L);
    EnumerationBindings::PushRepository(lua);
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

//! Handles lua erase_instance(instance).
//! \param L the \c lua_State
static int EraseInstanceProxy(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "erase_instance expects 1 argument");
    auto& game = Lua::CheckGame(L);
    InstancePtr instance;
    if (!lua_isnil(L, 1))
	instance = InstanceBindings::Check(L, 1);
    game.EraseInstance(instance);
    return 0;
}

//! Handles lua get_instance_for(player).
//! \param L the \c lua_State
static int GetInstanceForProxy(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_instance_for expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    auto& game = Lua::CheckGame(L);
    auto player = PlayerBindings::Check(L, 1);
    InstanceBindings::Push(lua, game.GetInstanceFor(player));
    return 1;
}

//! Handles lua get_players.
//! \param L the \c lua_State
static int GetPlayersProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "get_players expects no arguments");
    auto& lua = Lua::CheckLua(L);
    PlayerBindings::PushRepository(lua);
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
    lua.PushFunction(BroadcastProxy);
    lua.SetSafe("broadcast");
    lua.PushFunction(CryptProxy);
    lua.SetSafe("crypt");
    lua.PushFunction(EraseInstanceProxy);
    lua.SetSafe("erase_instance");
    lua.PushFunction(GetConfigProxy);
    lua.SetSafe("get_config");
    lua.PushFunction(GetDescriptorProxy);
    lua.SetSafe("get_descriptor");
    lua.PushFunction(DescriptorNamesProxy);
    lua.SetSafe("get_descriptor_names");
    lua.PushFunction(GetEnumerationsProxy);
    lua.SetSafe("get_enumerations");
    lua.PushFunction(GetInstanceForProxy);
    lua.SetSafe("get_instance_for");
    lua.PushFunction(GetPlayersProxy);
    lua.SetSafe("get_players");
    lua.PushFunction(GetStatesProxy);
    lua.SetSafe("get_states");
    lua.PushFunction(GetUsersProxy);
    lua.SetSafe("get_users");
    lua.PushFunction(PrintProxy);
    lua.SetSafe("print");
    lua.PushFunction(ShutdownProxy);
    lua.SetSafe("shutdown");
}

}; // namespace Scripting
}; // namespace Scratch
