//! \file trust_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_TRUST_BINDINGS_CPP_

#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>
#include <scratch/trust.hpp>
#include <scratch/trust_bindings.hpp>

namespace Scratch {
namespace Scripting {

using Trust = Scratch::Core::Trust;

//! Handles lua get_trust_names.
static int GetTrustNamesProxy(lua_State* L) {
    if (lua_gettop(L) != 0)
	return luaL_error(L, "get_trust_names expects no arguments");
    return PushEnumNames<Trust>(L);
}

//! Handles lua trust_allows(granted, required).
static int TrustAllowsProxy(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "trust_allows expects 2 arguments");
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TSTRING);
    auto& lua = Lua::CheckLua(L);
    const auto granted = Trust::ByName(Lua::CheckString(L, 1));
    const auto required = Trust::ByName(Lua::CheckString(L, 2));
    lua.PushBool(Trust::Allows(granted, required));
    return 1;
}

//! Registers Trust free functions on \p lua.
//! \param lua the Lua facade
void TrustBindings::Register(Lua& lua) {
    lua.SetSafe("get_trust_names", GetTrustNamesProxy);
    lua.SetSafe("trust_allows", TrustAllowsProxy);
}

}; // namespace Scripting
}; // namespace Scratch
