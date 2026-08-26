//! \file lua_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.

#include <scratch/config.hpp>
#include <scratch/descriptor_bindings.hpp>
#include <scratch/game_bindings.hpp>
#include <scratch/lua_bindings.hpp>

namespace Scratch {
namespace Scripting {

using Config = Scratch::Core::Config;

void LuaBindings::Register(Lua& lua) {
    lua.Class<Config>("Scratch.Config").
	Function("get_address", &Config::GetAddress).
	Function("get_metacolor", &Config::GetMetaColorProxy).
	Function("get_metacolors", &Config::GetMetaColors).
	Function("get_port", &Config::GetPort);
    DescriptorBindings::Register(lua);
    GameBindings::Register(lua);
}

}; // namespace Scripting
}; // namespace Scratch
