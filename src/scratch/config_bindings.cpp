//! \file config_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_CONFIG_BINDINGS_CPP_

#include <scratch/color.hpp>
#include <scratch/config.hpp>
#include <scratch/config_bindings.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Scripting {

//! Metatable name for Config userdata.
const char ConfigBindings::MetaName[] = "Scratch.Config";

// ScratchMUD types.
using Color = Scratch::Net::Color;
using WeakConfigPtr = std::weak_ptr<Config>;

//! Returns a copy of the weak Config handle stored in userdata.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
static WeakConfigPtr CheckWeakConfigPtr(
	lua_State* L,
	const int index) {
    return *static_cast<WeakConfigPtr*>(
	luaL_checkudata(L, index, ConfigBindings::MetaName));
}

//! Handles Config userdata garbage collection.
static int ConfigGc(lua_State* L) {
    static_cast<WeakConfigPtr*>(
	luaL_checkudata(L, 1, ConfigBindings::MetaName))->~WeakConfigPtr();
    return 0;
}

//! Handles Config:get_address().
static int ConfigGetAddress(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_address expects no arguments");
    auto& lua = Lua::CheckLua(L);
    auto config = ConfigBindings::Check(L, 1);
    auto address = config->GetAddress();
    config.reset();
    lua.PushString(std::move(address));
    return 1;
}

//! Handles Config:get_bootstrap_state().
static int ConfigGetBootstrapState(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_bootstrap_state expects no arguments");
    auto& lua = Lua::CheckLua(L);
    auto config = ConfigBindings::Check(L, 1);
    auto bootstrapState = config->GetBootstrapState();
    config.reset();
    lua.PushString(std::move(bootstrapState));
    return 1;
}

//! Handles Config:get_metacolor(name).
static int ConfigGetMetaColor(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "get_metacolor expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto& lua = Lua::CheckLua(L);
    auto config = ConfigBindings::Check(L, 1);
    const auto name = Lua::CheckString(L, 2);
    const auto meta = Color::ByName(name);
    if (!Color::IsMeta(meta)) {
	config.reset();
	return luaL_argerror(L, 2, "invalid metacolor");
    }
    const auto& colors = config->GetMetaColors();
    auto found = colors.find(meta);
    if (found == colors.end()) {
	config.reset();
	lua_pushnil(L);
	return 1;
    }
    auto realName = Color::ToString(found->second);
    config.reset();
    lua.PushString(std::move(realName));
    return 1;
}

//! Handles Config:get_metacolors().
static int ConfigGetMetaColors(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_metacolors expects no arguments");
    auto& lua = Lua::CheckLua(L);
    auto config = ConfigBindings::Check(L, 1);
    StringMapCi<String> colors;
    for (const auto& entry: config->GetMetaColors()) {
	auto metaName = Color::ToString(entry.first);
	auto realName = Color::ToString(entry.second);
	if (metaName.empty() || realName.empty())
	    continue;
	colors[std::move(metaName)] = std::move(realName);
    }
    config.reset();
    lua.PushStringMap(std::move(colors));
    return 1;
}

//! Handles Config:get_port().
static int ConfigGetPort(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_port expects no arguments");
    auto& lua = Lua::CheckLua(L);
    auto config = ConfigBindings::Check(L, 1);
    const auto port = config->GetPort();
    config.reset();
    lua.PushInt(static_cast<lua_Integer>(port));
    return 1;
}

//! Resolves a Config userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the configuration
ConfigPtr ConfigBindings::Check(
	lua_State* L,
	const int index) {
    ConfigPtr config = CheckWeakConfigPtr(L, index).lock();
    if (!config)
	luaL_argerror(L, index, "invalid config");
    return config;
}

//! Pushes a Config userdata, or nil.
//! \param lua the Lua facade
//! \param config the configuration to push
void ConfigBindings::Push(
	Lua& lua,
	ConfigPtr config) {
    lua.PushUserdata(std::move(config), MetaName);
}

//! Registers Config userdata bindings.
//! \param lua the Lua facade
void ConfigBindings::Register(Lua& lua) {
    auto* L = lua.GetState();
    luaL_newmetatable(L, MetaName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    static const luaL_Reg methods[] = {
	{"__gc", ConfigGc},
	{"get_address", ConfigGetAddress},
	{"get_bootstrap_state", ConfigGetBootstrapState},
	{"get_metacolor", ConfigGetMetaColor},
	{"get_metacolors", ConfigGetMetaColors},
	{"get_port", ConfigGetPort},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

}; // namespace Scripting
}; // namespace Scratch
