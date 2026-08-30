//! \file preference_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_PREFERENCE_BINDINGS_CPP_

#include <scratch/lua.hpp>
#include <scratch/preference.hpp>
#include <scratch/preference_bindings.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Scripting {

using Preference = Scratch::Core::Preference;

//! Handles lua get_preference_names.
static int GetPreferenceNamesProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "get_preference_names expects no arguments");
    return PushEnumNames<Preference>(L);
}

//! Registers Preference free functions on \p lua.
//! \param lua the Lua facade
void PreferenceBindings::Register(Lua& lua) {
    lua.SetSafe("get_preference_names", GetPreferenceNamesProxy);
}

}; // namespace Scripting
}; // namespace Scratch
