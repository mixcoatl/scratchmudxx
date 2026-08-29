//! \file parser_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_PARSER_BINDINGS_CPP_

#include <scratch/lua.hpp>
#include <scratch/parser_bindings.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Scripting {

// ScratchMUD types.
using Strings = Scratch::Algorithm::Strings;

//! Metatable name for Parser::Phrase userdata.
const char ParserBindings::PhraseMetaName[] = "Scratch.ParserPhrase";

//! Returns Parser::Phrase userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the phrase
static Parser::Phrase* CheckPhrase(
	lua_State* L,
	const int index) {
    return static_cast<Parser::Phrase*>(
	luaL_checkudata(L, index, ParserBindings::PhraseMetaName));
}

//! Handles Parser::Phrase userdata garbage collection.
static int PhraseGc(lua_State* L) {
    CheckPhrase(L, 1)->~Phrase();
    return 0;
}

//! Pushes a Parser::Phrase userdata.
static void PushPhrase(
	lua_State* L,
	Parser::Phrase phrase) {
    void* memory = lua_newuserdata(L, sizeof(Parser::Phrase));
    new (memory) Parser::Phrase(std::move(phrase));
    luaL_setmetatable(L, ParserBindings::PhraseMetaName);
}

//! Pushes parsed phrases as a Lua array.
static void PushPhrases(
	lua_State* L,
	const Parser& parser) {
    const auto size = parser.GetSize();
    lua_createtable(L, static_cast<int>(size), 0);
    for (std::size_t i = 0; i < size; ++i) {
	PushPhrase(L, parser.GetPhrase(i));
	lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
    }
}

//! Handles Parser::Phrase:get_count().
static int PhraseGetCount(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_count expects no arguments");
    lua_pushinteger(
	L, static_cast<lua_Integer>(CheckPhrase(L, 1)->GetCount()));
    return 1;
}

//! Handles Parser::Phrase:get_delimiter().
static int PhraseGetDelimiter(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_delimiter expects no arguments");
    Lua::CheckLua(L).PushString(CheckPhrase(L, 1)->GetDelimiter());
    return 1;
}

//! Handles Parser::Phrase:get_nth().
static int PhraseGetNth(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_nth expects no arguments");
    lua_pushinteger(
	L, static_cast<lua_Integer>(CheckPhrase(L, 1)->GetNth()));
    return 1;
}

//! Handles Parser::Phrase:get_words().
static int PhraseGetWords(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_words expects no arguments");

    auto& lua = Lua::CheckLua(L);
    const auto& words = CheckPhrase(L, 1)->GetWords();
    lua_createtable(L, static_cast<int>(words.size()), 0);
    int index = 1;
    for (const auto& word: words) {
	lua.PushString(word);
	lua_rawseti(L, -2, index++);
    }
    return 1;
}

//! Handles Parser::Phrase:is_single().
static int PhraseIsSingle(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "is_single expects no arguments");
    lua_pushboolean(L, CheckPhrase(L, 1)->IsSingle());
    return 1;
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
	auto* phrase = CheckPhrase(L, -1);
	if (!Strings::CompareCi(
		phrase->GetDelimiter(), Lua::CheckString(L, 2))) {
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

	PushPhrases(L, parser);
	return 1;
    } catch (const std::exception& ex) {
	return luaL_error(L, "parse failed: %s", ex.what());
    } catch (...) {
	return luaL_error(L, "parse failed");
    }
}

//! Registers Parser::Phrase userdata.
//! \param L the \c lua_State
static void RegisterPhraseMeta(lua_State* L) {
    Lua::RegisterMetatable(L, ParserBindings::PhraseMetaName);
    static const luaL_Reg methods[] = {
	{"__gc", PhraseGc},
	{"get_count", PhraseGetCount},
	{"get_delimiter", PhraseGetDelimiter},
	{"get_nth", PhraseGetNth},
	{"get_words", PhraseGetWords},
	{"is_single", PhraseIsSingle},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

//! Registers Parser metatables and functions.
//! \param lua the Lua facade
void ParserBindings::Register(Lua& lua) {
    auto* L = lua.GetState();
    RegisterPhraseMeta(L);
    lua.SetSafe("match_phrase", MatchPhraseProxy);
    lua.SetSafe("parse", ParseProxy);
}

}; // namespace Scripting
}; // namespace Scratch
