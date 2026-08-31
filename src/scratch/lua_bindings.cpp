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
#include <scratch/instance.hpp>
#include <scratch/lua_bindings.hpp>
#include <scratch/menu.hpp>
#include <scratch/parser.hpp>
#include <scratch/player_bindings.hpp>
#include <scratch/room.hpp>
#include <scratch/room_exit.hpp>
#include <scratch/state_bindings.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/user_bindings.hpp>
#include <scratch/zone.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Scripting {

using Color = Scratch::Net::Color;
using Menu = Scratch::Net::Menu;

//! Handles StateRepository:erase(name).
static int StateRepositoryErase(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "erase expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    const auto name = Lua::CheckString(L, 2);
    const auto& bootstrap =
	    Lua::CheckGame(L).GetConfig()->GetBootstrapState();
    if (!bootstrap.empty() &&
	!Scratch::Algorithm::Strings::CompareCi(name, bootstrap))
	return luaL_error(L, "cannot erase bootstrap state");
    const bool erased = StateBindings::CheckRepository(L).Erase(name);
    lua.PushBool(erased);
    return 1;
}

//! Returns the real color names.
static std::vector<String> GetColorNames() {
    std::vector<String> names;
    Scratch::Net::Color::ForEach(
	[](Scratch::Net::Color::ColorEnum value) noexcept {
	    return !Scratch::Net::Color::IsMetaColor(value);
	},
	[&](Scratch::Net::Color::ColorEnum, const String& name) {
	    names.push_back(name);
	});
    return names;
}

//! Returns the metacolor names.
static std::vector<String> GetMetaColorNames() {
    std::vector<String> names;
    Scratch::Net::Color::ForEach(
	Scratch::Net::Color::IsMetaColor,
	[&](Scratch::Net::Color::ColorEnum, const String& name) {
	    names.push_back(name);
	});
    return names;
}

using Parser = Scratch::Core::Parser;
using Strings = Scratch::Algorithm::Strings;

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

//! Sets a Zone name.
static void ZoneSetNameProxy(
    std::shared_ptr<Scratch::Core::Zone> zone,
    Scratch::Core::Game& game,
    String name) {
    auto repo = game.GetZones();
    if (!repo->IsValidThingId(name))
	throw std::invalid_argument("invalid zone id");
    if (repo->Contains(zone))
	throw std::runtime_error("cannot set_name on a live zone");
    zone->SetName(name);
}

void LuaBindings::Register(Lua& lua) {
    lua.Function("get_color_names", GetColorNames);
    lua.Function("get_metacolor_names", GetMetaColorNames);
    CommandBindings::Register(lua);
    lua.Class<Scratch::Core::CommandRepository>(CommandBindings::RepositoryMetaName).
	Function("erase", &Scratch::Core::CommandRepository::Erase).
	Function("get", &Scratch::Core::CommandRepository::Get).
	Function("get_ids", &Scratch::Core::CommandRepository::GetIds).
	Function("load", &Scratch::Core::CommandRepository::Load).
	Function("load_index", &Scratch::Core::CommandRepository::LoadIndex).
	Function("save", &Scratch::Core::CommandRepository::Save).
	Function("save_index", &Scratch::Core::CommandRepository::SaveIndex).
	Function("store", &Scratch::Core::CommandRepository::Store);
    lua.Class<Scratch::Core::Config>("Scratch.Config").
	Function("get_address", &Scratch::Core::Config::GetAddress).
	Function("get_bootstrap_state", &Scratch::Core::Config::GetBootstrapState).
	Function("get_metacolor", &Scratch::Core::Config::GetMetaColorProxy).
	Function("get_metacolors", &Scratch::Core::Config::GetMetaColors).
	Function("get_port", &Scratch::Core::Config::GetPort);
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
    lua.Class<Scratch::Net::Editor>("Scratch.Editor").
	Function("get_length", &Scratch::Net::Editor::GetLength).
	Function("get_max_length", &Scratch::Net::Editor::GetMaxLength).
	Function("get_tag", &Scratch::Net::Editor::GetTag).
	Function("get_text", &Scratch::Net::Editor::GetText).
	Function("is_aborted", &Scratch::Net::Editor::IsAborted).
	Function("is_saved", &Scratch::Net::Editor::IsSaved);
    GameBindings::Register(lua);
    lua.Function(
	"get_gender_names",
	&Detail::GetEnumNames<Scratch::Core::Gender>);
    lua.Class<Scratch::Core::Instance>("Scratch.Instance").
	Function("add_child", &Scratch::Core::Instance::AddChild).
	Function("get_contents", &Scratch::Core::Instance::GetContents).
	Function("get_contents_weight", &Scratch::Core::Instance::GetContentsWeight).
	Function("get_descriptor", &Scratch::Core::Instance::GetDescriptor).
	Function("get_gender", &Scratch::Core::Instance::GetGender).
	Function("get_name", &Scratch::Core::Instance::GetName).
	Function("get_parent", &Scratch::Core::Instance::GetParent).
	Function("get_player", &Scratch::Core::Instance::GetPlayer).
	Function("get_total_weight", &Scratch::Core::Instance::GetTotalWeight).
	Function("get_weight", &Scratch::Core::Instance::GetWeight).
	Function("get_world", &Scratch::Core::Instance::GetWorld).
	Function("matches", &Scratch::Core::Instance::Matches, Optional()).
	Function("remove", &Scratch::Core::Instance::Remove).
	Function("remove_child", &Scratch::Core::Instance::RemoveChild).
	Function("set_gender", &Scratch::Core::Instance::SetGender).
	Function("set_weight", &Scratch::Core::Instance::SetWeight);
    lua.Class<Parser::Phrase>("Scratch.ParserPhrase").
	Value().
	Function("get_count", &Parser::Phrase::GetCount).
	Function("get_delimiter", &Parser::Phrase::GetDelimiter).
	Function("get_nth", &Parser::Phrase::GetNth).
	Function("get_words", &Parser::Phrase::GetWords).
	Function("is_single", &Parser::Phrase::IsSingle);
    lua.RawFunction("match_phrase", MatchPhraseProxy);
    lua.RawFunction("parse", ParseProxy);
    PlayerBindings::Register(lua);
    lua.Class<Scratch::Core::PlayerRepository>(PlayerBindings::RepositoryMetaName).
	Function("erase", &Scratch::Core::PlayerRepository::Erase).
	Function("get", &Scratch::Core::PlayerRepository::Get).
	Function("get_ids", &Scratch::Core::PlayerRepository::GetIds).
	Function("load", &Scratch::Core::PlayerRepository::Load).
	Function("load_index", &Scratch::Core::PlayerRepository::LoadIndex).
	Function("save", &Scratch::Core::PlayerRepository::Save).
	Function("save_index", &Scratch::Core::PlayerRepository::SaveIndex).
	Function("store", &Scratch::Core::PlayerRepository::Store);
    lua.Function("get_preference_names", &Detail::GetEnumNames<Scratch::Core::Preference>);
    lua.Class<Scratch::Core::RoomExit>("Scratch.RoomExit").
	Function("get_direction", &Scratch::Core::RoomExit::GetDirection).
	Function("get_target", &Scratch::Core::RoomExit::GetTarget).
	Function("set_target", &Scratch::Core::RoomExit::SetTarget);
    lua.Class<Scratch::Core::Room>("Scratch.Room").
	Function("get_description", &Scratch::Core::Room::GetDescription).
	Function("get_exit", &Scratch::Core::Room::GetExit).
	Function("get_exits", &Scratch::Core::Room::GetExits).
	Function("get_name", &Scratch::Core::Thing::GetName).
	Function("get_qualified_name", &Scratch::Core::Room::GetQualifiedName).
	Function("get_title", &Scratch::Core::Room::GetTitle).
	Function("set_description", &Scratch::Core::Room::SetDescription).
	Function("set_exit", &Scratch::Core::Room::SetExit).
	Function("set_name", &Scratch::Core::Room::SetName).
	Function("set_title", &Scratch::Core::Room::SetTitle);
    StateBindings::Register(lua);
    lua.Class<Scratch::Core::StateRepository>(StateBindings::RepositoryMetaName).
	RawFunction("erase", StateRepositoryErase).
	Function("get", &Scratch::Core::StateRepository::Get).
	Function("get_ids", &Scratch::Core::StateRepository::GetIds).
	Function("load", &Scratch::Core::StateRepository::Load).
	Function("load_index", &Scratch::Core::StateRepository::LoadIndex).
	Function("save", &Scratch::Core::StateRepository::Save).
	Function("save_index", &Scratch::Core::StateRepository::SaveIndex).
	Function("store", &Scratch::Core::StateRepository::Store);
    lua.Function("get_trust_names", &Detail::GetEnumNames<Scratch::Core::Trust>);
    lua.Function("trust_allows", &Scratch::Core::Trust::Allows);
    UserBindings::Register(lua);
    lua.Class<Scratch::Core::UserRepository>(UserBindings::RepositoryMetaName).
	Function("erase", &Scratch::Core::UserRepository::Erase).
	Function("get", &Scratch::Core::UserRepository::Get).
	Function("get_ids", &Scratch::Core::UserRepository::GetIds).
	Function("load", &Scratch::Core::UserRepository::Load).
	Function("load_index", &Scratch::Core::UserRepository::LoadIndex).
	Function("save", &Scratch::Core::UserRepository::Save).
	Function("save_index", &Scratch::Core::UserRepository::SaveIndex).
	Function("store", &Scratch::Core::UserRepository::Store);
    lua.Class<Scratch::Core::Zone>("Scratch.Zone").
	Function("create_room", &Scratch::Core::Zone::CreateRoom).
	Function("erase_room", &Scratch::Core::Zone::EraseRoom).
	Function("get_name", &Scratch::Core::Zone::GetName).
	Function("get_room", &Scratch::Core::Zone::GetRoom).
	Function("get_room_names", &Scratch::Core::Zone::GetRoomNames).
	Function("set_name", ZoneSetNameProxy, Injected<Scratch::Core::Game>()).
	Function("store_room", &Scratch::Core::Zone::StoreRoom);
    lua.Class<Scratch::Core::ZoneRepository>("Scratch.ZoneRepository").
	Function("erase", &Scratch::Core::ZoneRepository::Erase).
	Function("get", &Scratch::Core::ZoneRepository::Get).
	Function("get_ids", &Scratch::Core::ZoneRepository::GetIds).
	Function("load", &Scratch::Core::ZoneRepository::Load).
	Function("load_index", &Scratch::Core::ZoneRepository::LoadIndex).
	Function("save", &Scratch::Core::ZoneRepository::Save).
	Function("save_index", &Scratch::Core::ZoneRepository::SaveIndex).
	Function("store", &Scratch::Core::ZoneRepository::Store);
}

}; // namespace Scripting
}; // namespace Scratch
