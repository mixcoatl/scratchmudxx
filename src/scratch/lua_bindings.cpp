//! \file lua_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.

#include <scratch/config.hpp>
#include <scratch/descriptor_bindings.hpp>
#include <scratch/editor.hpp>
#include <scratch/game_bindings.hpp>
#include <scratch/gender.hpp>
#include <scratch/lua_bindings.hpp>
#include <scratch/menu.hpp>
#include <scratch/state_bindings.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/string.hpp>
#include <scratch/trust.hpp>
#include <scratch/user_bindings.hpp>

namespace Scratch {
namespace Scripting {

using Color = Scratch::Net::Color;
using Config = Scratch::Core::Config;
using Editor = Scratch::Net::Editor;
using Gender = Scratch::Core::Gender;
using Menu = Scratch::Net::Menu;
using StateRepository = Scratch::Core::StateRepository;
using Strings = Scratch::Algorithm::Strings;
using Trust = Scratch::Core::Trust;
using UserRepository = Scratch::Core::UserRepository;

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
    lua.Function("parse_gender", &Gender::ByName);
    lua.Function("parse_trust", &Trust::ByName);
    lua.Function("get_color_names", GetColorNames);
    lua.Function(
	"get_gender_names",
	&Detail::GetEnumNames<Gender>);
    lua.Function("get_metacolor_names", GetMetaColorNames);
    lua.Class<Config>("Scratch.Config").
	Function("get_address", &Config::GetAddress).
	Function("get_bootstrap_state", &Config::GetBootstrapState).
	Function("get_metacolor", &Config::GetMetaColorProxy).
	Function("get_metacolors", &Config::GetMetaColors).
	Function("get_port", &Config::GetPort);
    lua.Class<Menu>("Scratch.Menu").
	Function("add_block", &Menu::AddBlock, Optional(Color::C_TEXT), Optional(String("<Blank>")), Optional()).
	Function("add_choices", &Menu::AddChoices, Optional()).
	Function("add_field", &Menu::AddField, Optional(Color::C_TEXT), Optional(String("<Blank>")), Optional()).
	Function("add_item", &Menu::AddItem).
	Function("add_listing", &Menu::AddListing, Optional()).
	Function("add_named_choices", &Menu::AddNamedChoices, Optional()).
	Function("add_named_listing", &Menu::AddNamedListing, Optional()).
	Function("clear", &Menu::Clear).
	Function("has_prompt", &Menu::HasPrompt).
	Function("has_section", &Menu::HasSection).
	Function("match_key", &Menu::MatchKey).
	Function("set_field_title", static_cast<void (Menu::*)(const String&, const String&, Color::ColorEnum, const String&)>(&Menu::SetTitle)).
	Function("set_prompt", &Menu::SetPrompt).
	Function("set_title", static_cast<void (Menu::*)(const String&)>(&Menu::SetTitle));
    DescriptorBindings::Register(lua);
    lua.Class<Editor>("Scratch.Editor").
	Function("get_length", &Editor::GetLength).
	Function("get_max_length", &Editor::GetMaxLength).
	Function("get_tag", &Editor::GetTag).
	Function("get_text", &Editor::GetText).
	Function("is_aborted", &Editor::IsAborted).
	Function("is_saved", &Editor::IsSaved);
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
    lua.Function("get_trust_names", &Detail::GetEnumNames<Trust>);
    lua.Function("trust_allows", &Trust::Allows);
    UserBindings::Register(lua);
    lua.Class<UserRepository>(UserBindings::RepositoryMetaName).
	Function("erase", &UserRepository::Erase).
	Function("get", &UserRepository::Get).
	Function("get_ids", &UserRepository::GetIds).
	Function("load", &UserRepository::Load).
	Function("load_index", &UserRepository::LoadIndex).
	Function("save", &UserRepository::Save).
	Function("save_index", &UserRepository::SaveIndex).
	Function("store", &UserRepository::Store);
}

}; // namespace Scripting
}; // namespace Scratch
