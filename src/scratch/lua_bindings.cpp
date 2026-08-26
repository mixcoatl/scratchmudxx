//! \file lua_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.

#include <scratch/config.hpp>
#include <scratch/descriptor_bindings.hpp>
#include <scratch/game_bindings.hpp>
#include <scratch/lua_bindings.hpp>
#include <scratch/state_bindings.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Scripting {

using Color = Scratch::Net::Color;
using Config = Scratch::Core::Config;
using StateRepository = Scratch::Core::StateRepository;
using Strings = Scratch::Algorithm::Strings;

//! Handles StateRepository:erase(name).
static int StateRepositoryErase(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "erase expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    const auto name = Lua::CheckString(L, 2);
    const auto& bootstrap =
	    Lua::CheckGame(L).GetConfig()->GetBootstrapState();
    if (!bootstrap.empty() &&
	!Strings::CompareCi(name, bootstrap))
	return luaL_error(L, "cannot erase bootstrap state");
    const bool erased = StateBindings::CheckRepository(L).Erase(name);
    lua.PushBool(erased);
    return 1;
}

//! Returns the real color names.
static std::vector<String> GetColorNames() {
    std::vector<String> names;
    Color::ForEach(
	[](Color::ColorEnum value) noexcept {
	    return !Color::IsMetaColor(value);
	},
	[&](Color::ColorEnum, const String& name) {
	    names.push_back(name);
	});
    return names;
}

//! Returns the metacolor names.
static std::vector<String> GetMetaColorNames() {
    std::vector<String> names;
    Color::ForEach(
	Color::IsMetaColor,
	[&](Color::ColorEnum, const String& name) {
	    names.push_back(name);
	});
    return names;
}

void LuaBindings::Register(Lua& lua) {
    lua.Function("get_color_names", GetColorNames);
    lua.Function("get_metacolor_names", GetMetaColorNames);
    lua.Class<Config>("Scratch.Config").
	Function("get_address", &Config::GetAddress).
	Function("get_bootstrap_state", &Config::GetBootstrapState).
	Function("get_metacolor", &Config::GetMetaColorProxy).
	Function("get_metacolors", &Config::GetMetaColors).
	Function("get_port", &Config::GetPort);
    DescriptorBindings::Register(lua);
    GameBindings::Register(lua);
    StateBindings::Register(lua);
    lua.Class<StateRepository>(StateBindings::RepositoryMetaName).
	RawFunction("erase", StateRepositoryErase).
	Function("get", &StateRepository::Get).
	Function("get_ids", &StateRepository::GetIds).
	Function("load", &StateRepository::Load).
	Function("load_index", &StateRepository::LoadIndex).
	Function("save", &StateRepository::Save).
	Function("save_index", &StateRepository::SaveIndex).
	Function("store", &StateRepository::Store);
}

}; // namespace Scripting
}; // namespace Scratch
