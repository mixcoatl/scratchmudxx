//! \file instance_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_INSTANCE_BINDINGS_CPP_

#include <scratch/descriptor_bindings.hpp>
#include <scratch/game.hpp>
#include <scratch/gender.hpp>
#include <scratch/instance.hpp>
#include <scratch/instance_bindings.hpp>
#include <scratch/lua.hpp>
#include <scratch/player_bindings.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Scripting {

//! Metatable name for Instance userdata.
const char InstanceBindings::MetaName[] = "Scratch.Instance";

//! ScratchMUD types.
using Gender = Scratch::Core::Gender;
using WeakInstancePtr = std::weak_ptr<Instance>;

//! Returns a copy of the weak Instance handle stored in userdata.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
static WeakInstancePtr CheckWeakInstancePtr(
	lua_State* L,
	const int index) {
    return *static_cast<WeakInstancePtr*>(
	luaL_checkudata(L, index, InstanceBindings::MetaName));
}

//! Handles Instance userdata garbage collection.
static int InstanceGc(lua_State* L) {
    static_cast<WeakInstancePtr*>(
	luaL_checkudata(L, 1, InstanceBindings::MetaName))->~WeakInstancePtr();
    return 0;
}

//! Handles Instance:find(line).
static int InstanceFind(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "find expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto& lua = Lua::CheckLua(L);
    auto& game = Lua::CheckGame(L);
    auto instance = InstanceBindings::Check(L, 1);
    auto found = instance->Find(game, Lua::CheckString(L, 2));
    instance.reset();
    InstanceBindings::Push(lua, std::move(found));
    return 1;
}

//! Handles Instance:get_descriptor().
static int InstanceGetDescriptor(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto instance = InstanceBindings::Check(L, 1);
    auto d = instance->GetDescriptor();
    instance.reset();
    DescriptorBindings::Push(lua, std::move(d));
    return 1;
}

//! Handles Instance:get_gender().
static int InstanceGetGender(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto instance = InstanceBindings::Check(L, 1);
    auto gender = Gender::ToString(instance->GetGender());
    instance.reset();
    lua.PushString(std::move(gender));
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

//! Handles Instance:get_player().
static int InstanceGetPlayer(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto instance = InstanceBindings::Check(L, 1);
    auto player = instance->GetPlayer();
    instance.reset();
    PlayerBindings::Push(lua, std::move(player));
    return 1;
}

//! Handles Instance:matches(word) and Instance:matches(seeker, word).
static int InstanceMatches(lua_State* L) {
    const auto top = lua_gettop(L);
    if (top != 2 && top != 3)
	return luaL_error(L, "matches expects 1 or 2 arguments");
    int wordIndex = 2;
    if (top == 3) {
	if (!lua_isnil(L, 2))
	    luaL_checkudata(L, 2, InstanceBindings::MetaName);
	wordIndex = 3;
    }
    luaL_checktype(L, wordIndex, LUA_TSTRING);
    auto& lua = Lua::CheckLua(L);
    auto instance = InstanceBindings::Check(L, 1);
    InstancePtr seeker;
    if (top == 3 && !lua_isnil(L, 2))
	seeker = InstanceBindings::Check(L, 2);
    const bool matched = instance->Matches(
	    seeker, Lua::CheckString(L, wordIndex));
    instance.reset();
    seeker.reset();
    lua.PushBool(matched);
    return 1;
}

//! Handles Instance:set_gender(gender).
static int InstanceSetGender(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_gender expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto instance = InstanceBindings::Check(L, 1);
    const auto name = Lua::CheckString(L, 2);
    if (name.empty()) {
	instance->SetGender(Gender::GENDER_UNDEFINED);
	return 0;
    }
    const auto gender = Gender::ByName(name);
    if (!Gender::IsDefined(gender)) {
	instance.reset();
	return luaL_argerror(L, 2, "unknown gender");
    }
    instance->SetGender(gender);
    return 0;
}

//! Resolves an Instance userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the instance
InstancePtr InstanceBindings::Check(
	lua_State* L,
	const int index) {
    InstancePtr instance = CheckWeakInstancePtr(L, index).lock();
    if (!instance)
	luaL_argerror(L, index, "invalid instance");
    return instance;
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
//! \param L the \c lua_State
static void RegisterInstanceMeta(lua_State* L) {
    luaL_newmetatable(L, InstanceBindings::MetaName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    static const luaL_Reg methods[] = {
	{"__gc", InstanceGc},
	{"find", InstanceFind},
	{"get_descriptor", InstanceGetDescriptor},
	{"get_gender", InstanceGetGender},
	{"get_name", InstanceGetName},
	{"get_player", InstanceGetPlayer},
	{"matches", InstanceMatches},
	{"set_gender", InstanceSetGender},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

//! Registers Instance metatable.
//! \param lua the Lua facade
void InstanceBindings::Register(Lua& lua) {
    RegisterInstanceMeta(lua.GetState());
}

}; // namespace Scripting
}; // namespace Scratch
