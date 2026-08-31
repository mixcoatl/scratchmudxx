//! \file game_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_GAME_BINDINGS_CPP_

#include <scratch/descriptor.hpp>
#include <scratch/game.hpp>
#include <scratch/game_bindings.hpp>
#include <scratch/instance.hpp>
#include <scratch/logger.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/string.hpp>
#include <scratch/world.hpp>

namespace Scratch {
namespace Scripting {

using World = Scratch::Core::World;
using Instance = Scratch::Core::Instance;
using InstancePtr = Scratch::Core::InstancePtr;

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

//! Handles lua get_room(name).
static int GetRoomProxy(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_room expects 1 argument");
    luaL_checktype(L, 1, LUA_TSTRING);
    auto& lua = Lua::CheckLua(L);
    auto& game = Lua::CheckGame(L);
    InstancePtr perspective;
    lua.PushCallerEnv();
    if (lua_istable(L, -1)) {
	lua_getfield(L, -1, "actor");
	if (lua_isuserdata(L, -1))
	    perspective = Lua::CheckWeakUserdata<Instance>(
		L, "Scratch.Instance", "invalid instance", -1);
	lua_pop(L, 1);
    }
    lua_pop(L, 1);
    lua.PushUserdata(game.GetRoom(
	Lua::CheckString(L, 1), perspective), "Scratch.Room");
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
    lua.PushString(Scratch::Algorithm::Strings::CryptCopy(plaintext, salt));
    return 1;
}

//! Registers Game free functions on \p lua.
//! \param lua the Lua facade
void GameBindings::Register(Lua& lua) {
    lua.Class<World>("Scratch.World").
	Function("get_id", &World::GetId).
	Function("get_instance", &World::GetInstance).
	Function("get_instances", &World::GetInstances);

    lua.Function("get_config", &Game::GetConfig);
    lua.RawFunction("broadcast", BroadcastProxy);
    lua.RawFunction("crypt", CryptProxy);
    lua.Function("get_descriptor", &Game::GetDescriptor);
    lua.Function("get_descriptor_names", &Game::GetDescriptorNames);
    lua.Function("get_instance_for", &Game::GetInstanceFor);
    lua.Function("get_players", &Game::GetPlayers);
    lua.RawFunction("get_room", GetRoomProxy);
    lua.Function("get_states", &Game::GetStates);
    lua.Function("get_users", &Game::GetUsers);
    lua.Function("get_world", &Game::GetWorld);
    lua.Function("get_worlds", &Game::GetWorlds);
    lua.Function("get_zones", &Game::GetZones);
    lua.RawFunction("print", PrintProxy);
    lua.RawFunction("shutdown", ShutdownProxy);
}

}; // namespace Scripting
}; // namespace Scratch
