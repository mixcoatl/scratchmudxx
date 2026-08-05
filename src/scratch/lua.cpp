//! \file lua.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_LUA_CPP_

#include <scratch/descriptor_bindings.hpp>
#include <scratch/game.hpp>
#include <scratch/game_bindings.hpp>
#include <scratch/logger.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>
#include <scratch/state_bindings.hpp>
#include <scratch/string.hpp>
#include <scratch/user_bindings.hpp>

namespace Scratch {
namespace Scripting {

//! Registry key for the owning Lua pointer.
static char luaRegistryKey;

//! Constructor.
//! \param game the game state
Lua::Lua(Game& game) :
	callers_(),
	executeDepth_(32),
	game_(game),
	lua_(nullptr),
	stagingPtr_(),
	stagingString_(),
	stagingStringMap_(),
	stagingStringSet_() {
    lua_ = luaL_newstate();
    if (!lua_) {
	throw std::bad_alloc();
    }

    // Stash this wrapper in the registry for C bindings.
    lua_pushlightuserdata(lua_, &luaRegistryKey);
    lua_pushlightuserdata(lua_, this);
    lua_settable(lua_, LUA_REGISTRYINDEX);

    static const luaL_Reg libs[] = {
	{"_G", luaopen_base},
	{LUA_TABLIBNAME, luaopen_table},
	{LUA_STRLIBNAME, luaopen_string},
	{LUA_MATHLIBNAME, luaopen_math},
	{nullptr, nullptr}
    };
    for (const luaL_Reg* lib = libs; lib->func; ++lib) {
	luaL_requiref(lua_, lib->name, lib->func, 1);
	lua_pop(lua_, 1);
    }

    DescriptorBindings::Register(*this);
    StateBindings::Register(*this);
    UserBindings::Register(*this);
    GameBindings::Register(*this);
}

//! Destructor.
Lua::~Lua() noexcept {
    if (lua_) {
	lua_close(lua_);
	lua_ = nullptr;
    }
}

//! Resolves a caller identity.
//! \param L the \c lua_State
//! \return the caller identity, or \c "Game"
String Lua::CheckCaller(lua_State* L) {
    auto& self = CheckLua(L);
    if (self.callers_.empty())
	return "Game";
    return self.callers_.back();
}

//! Resolves a game state.
//! \param L the \c lua_State
//! \return the game state
Game& Lua::CheckGame(lua_State* L) {
    return CheckLua(L).game_;
}

//! Resolves the Lua facade.
//! \param L the \c lua_State
Lua& Lua::CheckLua(lua_State* L) {
    lua_pushlightuserdata(L, &luaRegistryKey);
    lua_gettable(L, LUA_REGISTRYINDEX);
    auto self = static_cast<Lua*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (!self)
	luaL_error(L, "Lua not found in registry");
    return *self;
}

//! Resolves a Lua string at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the Lua string
//! \return a C++ copy of the Lua string
String Lua::CheckString(
	lua_State* L,
	const int index) {
    std::size_t length = 0;
    const char* chars = luaL_checklstring(L, index, &length);
    return String(chars, length);
}

//! Executes a string.
//! \param caller the caller identity
//! \param str the Lua code to execute
//! \return \c true if the string executed successfully
//! \sa #Execute(const String&)
bool Lua::Execute(
	const String& caller,
	const String& str) {
    if (!str.length())
	return false;
    if (callers_.size() >= executeDepth_) {
	LOGGER_LUA() << "Lua execute nesting exceeded; caller=" << caller;
	return false;
    }

    const int stackTop = lua_gettop(lua_);

    auto result = luaL_loadbuffer(
	lua_,
	str.data(),
	str.length(),
	caller.c_str());

    if (result == LUA_OK) {
	Caller frame(*this, caller);
	result = lua_pcall(lua_, 0, LUA_MULTRET, 0);
    }

    if (result != LUA_OK) {
	const char* error = lua_tostring(lua_, -1);
	LOGGER_LUA() << (error ? error : "Unknown Lua error.");
    }

    // Discard errors and values returned by the executed code.
    lua_settop(lua_, stackTop);
    return result == LUA_OK;
}

//! Pushes a Lua global.
//! \param name the global name
//! \sa #SetGlobal(const String&)
void Lua::GetGlobal(const String& name) {
    lua_getglobal(lua_, name.c_str());
}

//! Calls a C function.
//! \param function the C function to call
//! \param nArgs the number of arguments already on the stack for \p function
//! \param nReturns the number of results to leave on the stack
//! \return \c LUA_OK or a Lua error status; on failure an error object is on the stack
//! \remark Does not longjmp. Destroy C++ locals, then \c lua_error to propagate.
int Lua::PCall(
	const lua_CFunction function,
	const int nArgs,
	const int nReturns) {
    lua_pushcfunction(lua_, function);
    if (nArgs > 0)
	lua_insert(lua_, -(nArgs + 1));
    return lua_pcall(lua_, nArgs, nReturns, 0);
}

//! Pushes a boolean.
//! \param value the value to push
//! \remark May longjmp on stack overflow.
void Lua::PushBool(const bool value) {
    lua_pushboolean(lua_, value);
}

//! Pushes an integer.
//! \param value the value to push
//! \remark May longjmp on stack overflow.
void Lua::PushInt(const lua_Integer value) {
    lua_pushinteger(lua_, value);
}

//! Pushes a number.
//! \param value the value to push
//! \remark May longjmp on stack overflow.
void Lua::PushNumber(const double value) {
    lua_pushnumber(lua_, value);
}

//! Pushes a C++ string as a Lua string.
//! \param s the string to push
//! \remark Moves into #stagingString_ before push; may longjmp.
void Lua::PushString(String s) {
    stagingString_ = std::move(s);
    lua_pushlstring(lua_, stagingString_.c_str(), stagingString_.length());
    stagingString_.clear();
}

//! Pushes a string-to-string map as a Lua table.
//! \param entries the key/value pairs to push
//! \remark Moves into #stagingStringMap_ before allocation; may longjmp.
void Lua::PushStringMap(StringMapCi<String> entries) {
    stagingStringMap_ = std::move(entries);
    lua_createtable(lua_, 0, static_cast<int>(stagingStringMap_.size()));
    const int index = lua_absindex(lua_, -1);
    for (const auto& entry: stagingStringMap_) {
	this->PushString(entry.second);
	lua_setfield(lua_, index, entry.first.c_str());
    }
    stagingStringMap_.clear();
}

//! Pushes a #StringSetCi as a Lua array (1-based).
//! \param strings the strings to push
//! \remark Moves into #stagingStringSet_ before allocation; may longjmp.
void Lua::PushStringSet(StringSetCi strings) {
    stagingStringSet_ = std::move(strings);
    lua_createtable(lua_, static_cast<int>(stagingStringSet_.size()), 0);
    int index = 1;
    for (const auto& name: stagingStringSet_) {
	this->PushString(name);
	lua_rawseti(lua_, -2, index++);
    }
    stagingStringSet_.clear();
}

//! Registers a global free function.
//! \param name the global name
//! \param func the C function to register
void Lua::Register(
	const String& name,
	const lua_CFunction func) {
    lua_register(lua_, name.c_str(), func);
}

//! Sets a Lua global.
//! \param name the global name
//! \sa #GetGlobal(const String&)
void Lua::SetGlobal(const String& name) {
    lua_setglobal(lua_, name.c_str());
}

//! Unregisters a global name.
//! \param name the global name to clear
void Lua::Unregister(const String& name) {
    lua_pushnil(lua_);
    lua_setglobal(lua_, name.c_str());
}

}; // namespace Scripting
}; // namespace Scratch
