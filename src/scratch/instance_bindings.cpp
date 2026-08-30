//! \file instance_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_INSTANCE_BINDINGS_CPP_

#include <scratch/instance.hpp>
#include <scratch/instance_bindings.hpp>
#include <scratch/lua.hpp>
#include <scratch/world_bindings.hpp>

namespace Scratch {
namespace Scripting {

//! Metatable name for Instance userdata.
const char InstanceBindings::MetaName[] = "Scratch.Instance";

//! Handles Instance userdata garbage collection.
static int InstanceGc(lua_State* L) {
    return Lua::DestroyWeakUserdata<Instance>(
	L, InstanceBindings::MetaName);
}

//! Handles Instance:get_contents().
static int InstanceGetContents(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_contents expects no arguments");
    auto& lua = Lua::CheckLua(L);
    auto instance = InstanceBindings::Check(L, 1);
    const auto contents = instance->GetContents();
    instance.reset();
    lua_createtable(L, static_cast<int>(contents.size()), 0);
    lua_Integer index = 1;
    for (auto& child: contents) {
	InstanceBindings::Push(lua, child);
	lua_rawseti(L, -2, index++);
    }
    return 1;
}

//! Handles Instance:get_contents_weight().
static int InstanceGetContentsWeight(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_contents_weight expects no arguments");
    lua_pushnumber(
	L, InstanceBindings::Check(L, 1)->GetContentsWeight());
    return 1;
}

//! Handles Instance:get_name().
static int InstanceGetName(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto instance = InstanceBindings::Check(L, 1);
    auto name = instance->GetName();
    instance.reset();
    lua.PushString(std::move(name));
    return 1;
}

//! Handles Instance:get_parent().
static int InstanceGetParent(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_parent expects no arguments");
    auto& lua = Lua::CheckLua(L);
    auto instance = InstanceBindings::Check(L, 1);
    auto parent = instance->GetParent();
    instance.reset();
    InstanceBindings::Push(lua, std::move(parent));
    return 1;
}

//! Handles Instance:get_total_weight().
static int InstanceGetTotalWeight(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_total_weight expects no arguments");
    lua_pushnumber(L, InstanceBindings::Check(L, 1)->GetTotalWeight());
    return 1;
}

//! Handles Instance:get_weight().
static int InstanceGetWeight(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_weight expects no arguments");
    lua_pushnumber(L, InstanceBindings::Check(L, 1)->GetWeight());
    return 1;
}

//! Handles Instance:get_world().
static int InstanceGetWorld(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_world expects no arguments");
    auto& lua = Lua::CheckLua(L);
    auto instance = InstanceBindings::Check(L, 1);
    auto world = instance->GetWorld();
    instance.reset();
    WorldBindings::Push(lua, std::move(world));
    return 1;
}

//! Handles Instance:set_weight(weight).
static int InstanceSetWeight(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_weight expects 1 argument");
    luaL_checktype(L, 2, LUA_TNUMBER);
    auto instance = InstanceBindings::Check(L, 1);
    instance->SetWeight(lua_tonumber(L, 2));
    return 0;
}

//! Resolves an Instance userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the instance
InstancePtr InstanceBindings::Check(
	lua_State* L,
	const int index) {
    return Lua::CheckWeakUserdata<Instance>(
	L, MetaName, "invalid instance", index);
}

//! Pushes an Instance userdata, or nil.
//! \param lua the Lua facade
//! \param instance the instance to push
void InstanceBindings::Push(
	Lua& lua,
	InstancePtr instance) {
    lua.PushUserdata(std::move(instance), MetaName);
}

//! Registers Instance userdata bindings.
//! \param lua the Lua facade
void InstanceBindings::Register(Lua& lua) {
    Lua::RegisterMetatable(lua.GetState(), InstanceBindings::MetaName);
    static const luaL_Reg methods[] = {
	{"__gc", InstanceGc},
	{"get_contents", InstanceGetContents},
	{"get_contents_weight", InstanceGetContentsWeight},
	{"get_name", InstanceGetName},
	{"get_parent", InstanceGetParent},
	{"get_total_weight", InstanceGetTotalWeight},
	{"get_weight", InstanceGetWeight},
	{"get_world", InstanceGetWorld},
	{"set_weight", InstanceSetWeight},
	{nullptr, nullptr}
    };
    luaL_setfuncs(lua.GetState(), methods, 0);
    lua_pop(lua.GetState(), 1);
}

}; // namespace Scripting
}; // namespace Scratch
