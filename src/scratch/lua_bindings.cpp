//! \file lua_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.

#include <scratch/command_bindings.hpp>
#include <scratch/config.hpp>
#include <scratch/descriptor_bindings.hpp>
#include <scratch/editor.hpp>
#include <scratch/game_bindings.hpp>
#include <scratch/gender.hpp>
#include <scratch/instance.hpp>
#include <scratch/lua_bindings.hpp>
#include <scratch/menu.hpp>
#include <scratch/parser.hpp>
#include <scratch/state_bindings.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/string.hpp>
#include <scratch/trust.hpp>
#include <scratch/user_bindings.hpp>

namespace Scratch {
namespace Scripting {

using Color = Scratch::Net::Color;
using CommandRepository = Scratch::Core::CommandRepository;
using Config = Scratch::Core::Config;
using Editor = Scratch::Net::Editor;
using Gender = Scratch::Core::Gender;
using Instance = Scratch::Core::Instance;
using Menu = Scratch::Net::Menu;
using Parser = Scratch::Core::Parser;
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

//! Handles lua match_phrase(phrases, delimiter [, ordinal]).
static int MatchPhraseProxy(lua_State* L) {
    const int argc = lua_gettop(L);
    if (argc != 2 && argc != 3)
	return luaL_error(L, "match_phrase expects 2 or 3 arguments");
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TSTRING);
    lua_Integer ordinal = 1;
    if (argc == 3)
	ordinal = luaL_checkinteger(L, 3);
    if (ordinal < 1)
	return luaL_argerror(L, 3, "ordinal must be positive");

    std::size_t found = 0;
    const auto length = static_cast<int>(luaL_len(L, 1));
    for (int index = 1; index <= length; ++index) {
	lua_rawgeti(L, 1, index);
	auto phrase = Detail::LuaValue<Parser::Phrase>::Check(L, -1);
	if (!Strings::CompareCi(
		phrase.GetDelimiter(), Lua::CheckString(L, 2))) {
	    if (found++ == static_cast<std::size_t>(ordinal - 1))
		return 1;
	}
	lua_pop(L, 1);
    }
    lua_pushnil(L);
    return 1;
}

//! Handles lua parse(line [, delimiters]).
static int ParseProxy(lua_State* L) {
    const int argc = lua_gettop(L);
    if (argc != 1 && argc != 2)
	return luaL_error(L, "parse expects 1 or 2 arguments");
    luaL_checktype(L, 1, LUA_TSTRING);

    try {
	StringSetCi delimiters;
	if (argc == 2)
	    Lua::CheckStringSet(
		L, delimiters, 2, "delimiters must be strings");
	auto line = Lua::CheckString(L, 1);

	Parser parser;
	const auto valid = parser.Parse(line, delimiters);
	delimiters.clear();
	line.clear();
	if (!valid) {
	    lua_pushnil(L);
	    return 1;
	}

	std::vector<Parser::Phrase> phrases;
	for (std::size_t i = 0; i < parser.GetSize(); ++i)
	    phrases.push_back(parser.GetPhrase(i));
	Detail::LuaValue<std::vector<Parser::Phrase>>::Push(
	    L, std::move(phrases));
	return 1;
    } catch (const std::exception& ex) {
	return luaL_error(L, "parse failed: %s", ex.what());
    } catch (...) {
	return luaL_error(L, "parse failed");
    }
}

void LuaBindings::Register(Lua& lua) {
    lua.Function("parse_gender", &Gender::ByName);
    lua.Function("parse_trust", &Trust::ByName);
    lua.Function("get_color_names", GetColorNames);
    lua.Function(
	"get_gender_names",
	&Detail::GetEnumNames<Gender>);
    lua.Function("get_metacolor_names", GetMetaColorNames);
    CommandBindings::Register(lua);
    lua.Class<CommandRepository>(CommandBindings::RepositoryMetaName).
	Function("erase", &CommandRepository::Erase).
	Function("get", &CommandRepository::Get).
	Function("get_ids", &CommandRepository::GetIds).
	Function("load", &CommandRepository::Load).
	Function("load_index", &CommandRepository::LoadIndex).
	Function("save", &CommandRepository::Save).
	Function("save_index", &CommandRepository::SaveIndex).
	Function("store", &CommandRepository::Store);
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
    lua.Class<Instance>("Scratch.Instance").
	Function("add_child", &Instance::AddChild).
	Function("get_contents", &Instance::GetContents).
	Function("get_contents_weight", &Instance::GetContentsWeight).
	Function("get_name", &Instance::GetName).
	Function("get_parent", &Instance::GetParent).
	Function("get_total_weight", &Instance::GetTotalWeight).
	Function("get_weight", &Instance::GetWeight).
	Function("get_world", &Instance::GetWorld).
	Function("remove", &Instance::Remove).
	Function("remove_child", &Instance::RemoveChild).
	Function("set_weight", &Instance::SetWeight);
    lua.Class<Parser::Phrase>("Scratch.ParserPhrase").
	Value().
	Function("get_count", &Parser::Phrase::GetCount).
	Function("get_delimiter", &Parser::Phrase::GetDelimiter).
	Function("get_nth", &Parser::Phrase::GetNth).
	Function("get_words", &Parser::Phrase::GetWords).
	Function("is_single", &Parser::Phrase::IsSingle);
    lua.RawFunction("match_phrase", MatchPhraseProxy);
    lua.RawFunction("parse", ParseProxy);
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
