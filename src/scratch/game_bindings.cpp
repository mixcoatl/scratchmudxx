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
#include <scratch/logger.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/string.hpp>
#include <scratch/world.hpp>

namespace Scratch {
namespace Scripting {

using Game = Scratch::Core::Game;
using InstancePtr = Scratch::Core::InstancePtr;
using PruneInfo = Scratch::Core::PruneInfo;
using Scheduler = Scratch::Core::Scheduler;
using Strings = Scratch::Algorithm::Strings;
using World = Scratch::Core::World;
using WorldPtr = Scratch::Core::WorldPtr;

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

//! Handles lua prune_world(world [, force]).
//! \param L the \c lua_State
static int PruneWorldProxy(lua_State* L) {
    const int argc = lua_gettop(L);
    if (argc != 1 && argc != 2)
	return luaL_error(L, "prune_world expects 1 or 2 arguments");
    auto& game = Lua::CheckGame(L);
    auto world = Detail::LuaValue<WorldPtr>::Check(L, 1);
    bool force = false;
    if (argc == 2)
	force = lua_toboolean(L, 2) != 0;
    Lua::CheckLua(L).PushBool(game.PruneWorld(world, force));
    return 1;
}

//! Handles lua shutdown.
//! \param L the \c lua_State
static int ShutdownProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "shutdown expects no arguments");

    Lua::CheckGame(L).SetShutdown(true);
    return 0;
}

//! Handles World:get_prune().
//! \param L the \c lua_State
static int WorldGetPruneProxy(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_prune expects no arguments");
    auto world = Detail::LuaValue<WorldPtr>::Check(L, 1);
    if (!world) {
	lua_pushnil(L);
	return 1;
    }
    const auto info = world->GetPrune(
	Scheduler::Task::Clock::now());
    lua_createtable(L, 0, 5);
    lua_pushboolean(L, info.graceArmed);
    lua_setfield(L, -2, "grace_armed");
    if (info.graceArmed) {
	auto& lua = Lua::CheckLua(L);
	lua.PushString(Strings::FormatDuration(info.graceRemaining));
	lua_setfield(L, -2, "grace_remaining");
    } else {
	lua_pushnil(L);
	lua_setfield(L, -2, "grace_remaining");
    }
    lua_pushinteger(L, static_cast<lua_Integer>(info.occupants));
    lua_setfield(L, -2, "occupants");
    lua_pushboolean(L, info.protected_);
    lua_setfield(L, -2, "protected");
    lua_pushboolean(L, info.IsReady());
    lua_setfield(L, -2, "ready");
    return 1;
}

//! Registers Game free functions on \p lua.
//! \param lua the Lua facade
void GameBindings::Register(Lua& lua) {
    lua.Class<World>("Scratch.World").
	Function("add_instance", &World::AddInstance).
	Function("get_id", &World::GetId).
	Function("get_instance", &World::GetInstance).
	Function("get_instances", &World::GetInstances).
	Function("get_room_instance", &World::GetRoomInstance).
	Function("get_source_zone", &World::GetSourceZone).
	RawFunction("get_prune", WorldGetPruneProxy).
	Function("remove_instance", &World::RemoveInstance);

    lua.RawFunction("broadcast", BroadcastProxy);
    lua.Function("create_world", &Game::CreateWorld, Optional(String()));
    lua.RawFunction("crypt", CryptProxy);
    lua.Function("get_config", &Game::GetConfig);
    lua.Function("get_descriptor", &Game::GetDescriptor);
    lua.Function("get_descriptor_names", &Game::GetDescriptorNames);
    lua.Function("get_instance_for", &Game::GetInstanceFor);
    lua.Function("get_players", &Game::GetPlayers);
    lua.Function("get_room", &Game::GetRoom, Optional(InstancePtr()));
    lua.Function("get_room_instance", &Game::GetRoomInstance, Optional(InstancePtr()));
    lua.Function("get_start_room", &Game::GetStartRoom);
    lua.Function("get_states", &Game::GetStates);
    lua.Function("get_users", &Game::GetUsers);
    lua.Function("get_world", &Game::GetWorld);
    lua.Function("get_worlds", &Game::GetWorlds);
    lua.Function("get_zones", &Game::GetZones);
    lua.RawFunction("print", PrintProxy);
    lua.RawFunction("prune_world", PruneWorldProxy);
    lua.RawFunction("shutdown", ShutdownProxy);
}

}; // namespace Scripting
}; // namespace Scratch
