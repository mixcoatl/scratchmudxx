//! \file gender_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_GENDER_BINDINGS_CPP_

#include <scratch/gender.hpp>
#include <scratch/gender_bindings.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Scripting {

using Gender = Scratch::Core::Gender;

//! Pushes an ordered array of gender names.
//! \param L the \c lua_State
static int PushGenderNames(lua_State* L) {
    Lua::CheckLua(L);
    lua_createtable(L, Gender::MAX_GENDER_TYPES, 0);
    int index = 1;
    for (int value = Gender::GENDER_UNDEFINED + 1;
	    value < Gender::MAX_GENDER_TYPES; ++value) {
	auto name = Gender::ToString(static_cast<Gender::GenderEnum>(value));
	if (name.empty())
	    continue;
	lua_pushlstring(L, name.c_str(), name.length());
	lua_rawseti(L, -2, index++);
    }
    return 1;
}

//! Handles lua get_gender_names.
static int GetGenderNamesProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "get_gender_names expects no arguments");
    return PushGenderNames(L);
}

//! Registers Gender free functions on \p lua.
//! \param lua the Lua facade
void GenderBindings::Register(Lua& lua) {
    lua.PushFunction(GetGenderNamesProxy);
    lua.SetSafe("get_gender_names");
}

}; // namespace Scripting
}; // namespace Scratch
