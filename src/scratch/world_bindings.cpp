//! \file world_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_WORLD_BINDINGS_CPP_

#include <scratch/instance_bindings.hpp>
#include <scratch/lua.hpp>
#include <scratch/world.hpp>
#include <scratch/world_bindings.hpp>

namespace Scratch {
namespace Scripting {

//! Metatable name for World userdata.
const char WorldBindings::MetaName[] = "Scratch.World";

//! Handles World userdata garbage collection.
static int WorldGc(lua_State* L) {
    return Lua::DestroyWeakUserdata<World>(
	L, WorldBindings::MetaName);
}

//! Handles World:get_id().
static int WorldGetId(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_id expects no arguments");
    auto& lua = Lua::CheckLua(L);
    auto world = WorldBindings::Check(L, 1);
    auto id = world->GetId();
    world.reset();
    lua.PushString(std::move(id));
    return 1;
}

//! Handles World:get_instance(name).
static int WorldGetInstance(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "get_instance expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto& lua = Lua::CheckLua(L);
    auto world = WorldBindings::Check(L, 1);
    auto instance = world->GetInstance(Lua::CheckString(L, 2));
    world.reset();
    InstanceBindings::Push(lua, std::move(instance));
    return 1;
}

//! Handles World:get_instances().
static int WorldGetInstances(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_instances expects no arguments");
    auto& lua = Lua::CheckLua(L);
    auto world = WorldBindings::Check(L, 1);
    const auto instances = world->GetInstances();
    world.reset();
    lua_createtable(L, static_cast<int>(instances.size()), 0);
    lua_Integer index = 1;
    for (auto& instance: instances) {
	InstanceBindings::Push(lua, instance);
	lua_rawseti(L, -2, index++);
    }
    return 1;
}

//! Resolves a World userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the world
WorldPtr WorldBindings::Check(
	lua_State* L,
	const int index) {
    return Lua::CheckWeakUserdata<World>(
	L, MetaName, "invalid world", index);
}

//! Pushes a World userdata, or nil.
//! \param lua the Lua facade
//! \param world the world to push
void WorldBindings::Push(
	Lua& lua,
	WorldPtr world) {
    lua.PushUserdata(std::move(world), MetaName);
}

//! Registers World userdata bindings.
//! \param lua the Lua facade
static void RegisterWorldMeta(lua_State* L) {
    Lua::RegisterMetatable(L, WorldBindings::MetaName);
    static const luaL_Reg methods[] = {
	{"__gc", WorldGc},
	{"get_id", WorldGetId},
	{"get_instance", WorldGetInstance},
	{"get_instances", WorldGetInstances},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

//! Registers World metatable.
//! \param lua the Lua facade
void WorldBindings::Register(Lua& lua) {
    RegisterWorldMeta(lua.GetState());
}

}; // namespace Scripting
}; // namespace Scratch
