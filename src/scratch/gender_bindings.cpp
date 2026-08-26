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

//! Handles lua get_gender_names.
static int GetGenderNamesProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "get_gender_names expects no arguments");

    return PushEnumNames<Gender>(L);
}

//! Registers Gender free functions on \p lua.
//! \param lua the Lua facade
void GenderBindings::Register(Lua& lua) {
    lua.SetSafe("get_gender_names", GetGenderNamesProxy);
}

}; // namespace Scripting
}; // namespace Scratch
