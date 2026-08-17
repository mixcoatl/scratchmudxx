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

//! Metatable name for Parser userdata.
const char ParserBindings::MetaName[] = "Scratch.Parser";

//! Metatable name for Parser::Phrase userdata.
const char ParserBindings::PhraseMetaName[] = "Scratch.ParserPhrase";

//! Returns Parser userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the parser
static Parser* CheckParser(
	lua_State* L,
	const int index) {
    return static_cast<Parser*>(
	luaL_checkudata(L, index, ParserBindings::MetaName));
}

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

//! Handles Parser userdata garbage collection.
static int ParserGc(lua_State* L) {
    CheckParser(L, 1)->~Parser();
    return 0;
}

//! Handles Parser::Phrase userdata garbage collection.
static int PhraseGc(lua_State* L) {
    CheckPhrase(L, 1)->~Phrase();
    return 0;
}

//! Handles Parser:get_phrase(index).
static int ParserGetPhrase(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "get_phrase expects 1 argument");
    const auto index = luaL_checkinteger(L, 2);
    if (index < 0)
	return luaL_argerror(L, 2, "index must not be negative");

    const auto& phrase = CheckParser(L, 1)->GetPhrase(
	static_cast<std::size_t>(index));
    void* memory = lua_newuserdata(L, sizeof(Parser::Phrase));
    new (memory) Parser::Phrase(phrase);
    luaL_setmetatable(L, ParserBindings::PhraseMetaName);
    return 1;
}

//! Handles Parser:get_size().
static int ParserGetSize(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_size expects no arguments");
    lua_pushinteger(
	L, static_cast<lua_Integer>(CheckParser(L, 1)->GetSize()));
    return 1;
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

//! Handles lua parse(line [, delimiters]).
static int ParseProxy(lua_State* L) {
    const int argc = lua_gettop(L);
    if (argc != 1 && argc != 2)
	return luaL_error(L, "parse expects 1 or 2 arguments");
    luaL_checktype(L, 1, LUA_TSTRING);
    const auto line = Lua::CheckString(L, 1);

    StringSetCi delimiters;
    if (argc == 2) {
	luaL_checktype(L, 2, LUA_TTABLE);
	const int length = static_cast<int>(luaL_len(L, 2));
	for (int i = 1; i <= length; ++i) {
	    lua_rawgeti(L, 2, i);
	    if (!lua_isstring(L, -1)) {
		lua_pop(L, 1);
		delimiters.clear();
		return luaL_argerror(L, 2, "delimiters must be strings");
	    }
	    delimiters.insert(Lua::CheckString(L, -1));
	    lua_pop(L, 1);
	}
    }

    void* memory = lua_newuserdata(L, sizeof(Parser));
    auto* parser = new (memory) Parser();
    const bool valid = argc == 1
	? parser->Parse(line)
	: parser->Parse(line, delimiters);
    delimiters.clear();
    if (!valid) {
	parser->~Parser();
	lua_pop(L, 1);
	lua_pushnil(L);
	return 1;
    }

    luaL_setmetatable(L, ParserBindings::MetaName);
    return 1;
}

//! Registers Parser userdata.
//! \param L the \c lua_State
static void RegisterParserMeta(lua_State* L) {
    luaL_newmetatable(L, ParserBindings::MetaName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    static const luaL_Reg methods[] = {
	{"__gc", ParserGc},
	{"get_phrase", ParserGetPhrase},
	{"get_size", ParserGetSize},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

//! Registers Parser::Phrase userdata.
//! \param L the \c lua_State
static void RegisterPhraseMeta(lua_State* L) {
    luaL_newmetatable(L, ParserBindings::PhraseMetaName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    static const luaL_Reg methods[] = {
	{"__gc", PhraseGc},
	{"get_count", PhraseGetCount},
	{"get_delimiter", PhraseGetDelimiter},
	{"get_nth", PhraseGetNth},
	{"get_words", PhraseGetWords},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

//! Registers Parser metatables and functions.
//! \param lua the Lua facade
void ParserBindings::Register(Lua& lua) {
    auto* L = lua.GetState();
    RegisterParserMeta(L);
    RegisterPhraseMeta(L);
    lua.PushFunction(ParseProxy);
    lua.SetSafe("parse");
}

}; // namespace Scripting
}; // namespace Scratch
