//! \file lua.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_LUA_CPP_

#include <scratch/color_bindings.hpp>
#include <scratch/command_bindings.hpp>
#include <scratch/config_bindings.hpp>
#include <scratch/descriptor_bindings.hpp>
#include <scratch/editor_bindings.hpp>
#include <scratch/game.hpp>
#include <scratch/game_bindings.hpp>
#include <scratch/gender_bindings.hpp>
#include <scratch/instance_bindings.hpp>
#include <scratch/logger.hpp>
#include <scratch/lua.hpp>
#include <scratch/parser_bindings.hpp>
#include <scratch/scratch.hpp>
#include <scratch/state_bindings.hpp>
#include <scratch/string.hpp>
#include <scratch/trust_bindings.hpp>
#include <scratch/user_bindings.hpp>
#include <scratch/world_bindings.hpp>

namespace Scratch {
namespace Scripting {

//! Registry key for the owning Lua pointer.
static char luaRegistryKey;

//! Registry key for the safe table.
static char safeRegistryKey;

//! Rejects writes to a frozen table.
//! \param L the \c lua_State
static int FrozenNewIndex(lua_State* L) {
    return luaL_error(L, "cannot modify frozen table");
}

//! Constructor.
//! \param lua the Lua facade
//! \param caller the caller identity to push
Lua::Caller::Caller(
	Lua& lua,
	const String& caller) :
	active_(false),
	lua_(lua) {
    if (lua_.callers_.size() >= lua_.executeDepth_) {
	LOGGER_LUA() << "Lua execute nesting exceeded; caller=" << caller;
	return;
    }
    lua_.callers_.push_back(caller);
    lua_.PushSandboxEnv();
    active_ = true;
}

//! Destructor.
Lua::Caller::~Caller() {
    if (!active_)
	return;
    lua_.PopSandboxEnv();
    lua_.callers_.pop_back();
}

//! Constructor.
//! \param game the game state
Lua::Lua(Game& game) :
	callers_(),
	envs_(),
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

    // Stash Lua wrapper in registry.
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

    this->InitSafe();

    ColorBindings::Register(*this);
    CommandBindings::Register(*this);
    ConfigBindings::Register(*this);
    DescriptorBindings::Register(*this);
    EditorBindings::Register(*this);
    GameBindings::Register(*this);
    GenderBindings::Register(*this);
    InstanceBindings::Register(*this);
    ParserBindings::Register(*this);
    StateBindings::Register(*this);
    TrustBindings::Register(*this);
    UserBindings::Register(*this);
    WorldBindings::Register(*this);

    // Strip loaders from real _G.
    static const char* const denied[] = {
	"load", "loadfile", "dofile", nullptr
    };
    for (const char* const* name = denied; *name; ++name) {
	lua_pushnil(lua_);
	lua_setglobal(lua_, *name);
    }

    // Freeze library tables and safe.
    static const char* const freezeNames[] = {
	"string", "table", "math", "os", nullptr
    };
    for (const char* const* name = freezeNames; *name; ++name) {
	lua_getglobal(lua_, *name);
	if (lua_istable(lua_, -1)) {
	    this->FreezeTable(-1);
	    lua_pushvalue(lua_, -1);
	    lua_setglobal(lua_, *name);
	    this->PushSafe();
	    lua_pushvalue(lua_, -2);
	    lua_pushlstring(lua_, *name, std::strlen(*name));
	    lua_insert(lua_, -2);
	    lua_rawset(lua_, -3);
	    lua_pop(lua_, 1);
	}
	lua_pop(lua_, 1);
    }
    lua_pushliteral(lua_, "");
    if (lua_getmetatable(lua_, -1)) {
	lua_getglobal(lua_, "string");
	lua_setfield(lua_, -2, "__index");
	lua_pop(lua_, 1);
    }
    lua_pop(lua_, 1);
    this->PushSafe();
    this->FreezeTable(-1);
    lua_pushlightuserdata(lua_, &safeRegistryKey);
    lua_pushvalue(lua_, -2);
    lua_settable(lua_, LUA_REGISTRYINDEX);
    lua_pop(lua_, 1);
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

//! Reads Lua string array at \p index into \p out.
//! \param L the \c lua_State
//! \param out the destination array
//! \param index the stack index of the table
//! \param error the element error
void Lua::CheckStringArray(
	lua_State* L,
	std::vector<String>& out,
	const int index,
	const char* error) {
    luaL_checktype(L, index, LUA_TTABLE);
    out.clear();
    const int length = static_cast<int>(luaL_len(L, index));
    out.reserve(static_cast<std::size_t>(std::max(0, length)));
    for (int i = 1; i <= length; ++i) {
	lua_rawgeti(L, index, i);
	if (!lua_isstring(L, -1)) {
	    lua_pop(L, 1);
	    out.clear();
	    luaL_argerror(L, index, error);
	}
	out.push_back(CheckString(L, -1));
	lua_pop(L, 1);
    }
}

//! Reads Lua string set at \p index into \p out.
//! \param L the \c lua_State
//! \param out the destination set
//! \param index the stack index of the table
//! \param error the element error
void Lua::CheckStringSet(
	lua_State* L,
	StringSetCi& out,
	const int index,
	const char* error) {
    luaL_checktype(L, index, LUA_TTABLE);
    out.clear();
    const int length = static_cast<int>(luaL_len(L, index));
    for (int i = 1; i <= length; ++i) {
	lua_rawgeti(L, index, i);
	if (!lua_isstring(L, -1)) {
	    lua_pop(L, 1);
	    out.clear();
	    luaL_argerror(L, index, error);
	}
	out.insert(CheckString(L, -1));
	lua_pop(L, 1);
    }
}

//! Executes a string.
//! \param str the Lua code to execute
//! \return \c true if the string executed successfully
//! \sa #Execute(const String&, const String&)
bool Lua::Execute(const String& str) {
    if (!str.length())
	return false;
    if (callers_.empty() || envs_.empty()) {
	LOGGER_LUA() << "Lua execute without Caller";
	return false;
    }

    const int stackTop = lua_gettop(lua_);
    const String& caller = callers_.back();

    auto result = luaL_loadbuffer(
	lua_,
	str.data(),
	str.length(),
	caller.c_str());

    if (result == LUA_OK) {
	lua_rawgeti(lua_, LUA_REGISTRYINDEX, envs_.back());
	if (!lua_setupvalue(lua_, -2, 1)) {
	    // Unused env value.
	    lua_pop(lua_, 1);
	}
	result = lua_pcall(lua_, 0, LUA_MULTRET, 0);
    }

    if (result != LUA_OK) {
	const char* error = lua_tostring(lua_, -1);
	LOGGER_LUA() << (error ? error : "Unknown Lua error.");
    }

    // Discard returns and errors.
    lua_settop(lua_, stackTop);
    return result == LUA_OK;
}

//! Executes a string.
//! \param caller the caller identity
//! \param str the Lua code to execute
//! \return \c true if the string executed successfully
//! \sa #Execute(const String&)
bool Lua::Execute(
	const String& caller,
	const String& str) {
    Caller frame(*this, caller);
    if (!frame.IsActive())
	return false;
    return this->Execute(str);
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

//! Pushes the current env, or nil.
void Lua::PushCallerEnv() {
    if (envs_.empty()) {
	lua_pushnil(lua_);
	return;
    }
    lua_rawgeti(lua_, LUA_REGISTRYINDEX, envs_.back());
}

//! Pushes a C function.
//! \param func the C function to push
//! \remark May longjmp on stack overflow.
void Lua::PushFunction(const lua_CFunction func) {
    lua_pushcfunction(lua_, func);
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

//! Registers a metatable shell.
//! \param L the \c lua_State
//! \param name the metatable name
void Lua::RegisterMetatable(
	lua_State* L,
	const char* name) {
    luaL_newmetatable(L, name);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
}

//! Sets an environment field.
//! \param name the field name
//! \sa #SetSafe(const String&)
void Lua::SetEnv(const String& name) {
    if (envs_.empty()) {
	LOGGER_LUA() << "Lua SetEnv without Caller; name=" << name;
	lua_pop(lua_, 1);
	return;
    }
    lua_rawgeti(lua_, LUA_REGISTRYINDEX, envs_.back());
    lua_pushvalue(lua_, -2);
    lua_setfield(lua_, -2, name.c_str());
    lua_pop(lua_, 2);
}

//! Sets a safe field.
//! \param name the field name
//! \sa #SetEnv(const String&)
void Lua::SetSafe(const String& name) {
    this->PushSafe();
    lua_pushvalue(lua_, -2);
    lua_pushlstring(lua_, name.c_str(), name.length());
    lua_insert(lua_, -2);
    lua_rawset(lua_, -3);
    lua_pop(lua_, 2);
}

//! Sets a safe C function.
//! \param name the field name
//! \param function the C function
void Lua::SetSafe(
	const String& name,
	lua_CFunction function) {
    this->PushFunction(function);
    this->SetSafe(name);
}

//! Stores the value at the top of the stack in the safe table.
//! \param name the field name
void Lua::SetSafeValue(const String& name) {
    this->PushSafe();
    lua_pushvalue(lua_, -2);
    lua_pushlstring(lua_, name.c_str(), name.length());
    lua_insert(lua_, -2);
    lua_rawset(lua_, -3);
    lua_pop(lua_, 2);
}

//! Registers a raw Lua function as a free function.
//! \param name the function name
//! \param function the Lua function
void Lua::RawFunction(
	const char* name,
	lua_CFunction function) {
    this->PushFunction(function);
    this->SetSafe(name);
}

//! Freezes a table.
//! \param index the stack index of the table
//! \remark Replaces the table at \p index with a read-only proxy.
void Lua::FreezeTable(const int index) {
    const int abs = lua_absindex(lua_, index);
    if (!lua_istable(lua_, abs))
	return;

    lua_newtable(lua_);
    const int proxy = lua_absindex(lua_, -1);
    lua_newtable(lua_);
    lua_pushvalue(lua_, abs);
    lua_setfield(lua_, -2, "__index");
    lua_pushcfunction(lua_, FrozenNewIndex);
    lua_setfield(lua_, -2, "__newindex");
    lua_pushliteral(lua_, "frozen");
    lua_setfield(lua_, -2, "__metatable");
    lua_setmetatable(lua_, proxy);
    lua_replace(lua_, abs);
}

//! Initializes the safe table.
void Lua::InitSafe() {
    lua_newtable(lua_);
    const int safe = lua_absindex(lua_, -1);

    static const char* const baseNames[] = {
	"_VERSION",
	"assert",
	"collectgarbage",
	"error",
	"getmetatable",
	"ipairs",
	"next",
	"pairs",
	"pcall",
	"print",
	"rawequal",
	"rawget",
	"rawlen",
	"rawset",
	"select",
	"setmetatable",
	"tonumber",
	"tostring",
	"type",
	"xpcall",
	nullptr
    };
    for (const char* const* name = baseNames; *name; ++name) {
	lua_getglobal(lua_, *name);
	if (!lua_isnil(lua_, -1))
	    lua_setfield(lua_, safe, *name);
	else
	    lua_pop(lua_, 1);
    }

    static const char* const libNames[] = {
	"string", "table", "math", nullptr
    };
    for (const char* const* name = libNames; *name; ++name) {
	lua_getglobal(lua_, *name);
	if (lua_istable(lua_, -1))
	    lua_setfield(lua_, safe, *name);
	else
	    lua_pop(lua_, 1);
    }

    // Trimmed os: date/time only.
    luaL_requiref(lua_, LUA_OSLIBNAME, luaopen_os, 0);
    lua_newtable(lua_);
    const int trimmedOs = lua_absindex(lua_, -1);
    static const char* const osNames[] = {
	"clock", "date", "difftime", "time", nullptr
    };
    for (const char* const* name = osNames; *name; ++name) {
	lua_getfield(lua_, -2, *name);
	if (!lua_isnil(lua_, -1))
	    lua_setfield(lua_, trimmedOs, *name);
	else
	    lua_pop(lua_, 1);
    }
    lua_remove(lua_, -2);
    lua_pushvalue(lua_, -1);
    lua_setfield(lua_, safe, "os");
    lua_setglobal(lua_, "os");

    lua_pushlightuserdata(lua_, &safeRegistryKey);
    lua_pushvalue(lua_, safe);
    lua_settable(lua_, LUA_REGISTRYINDEX);
    lua_pop(lua_, 1);
}

//! Pops the current sandbox env.
void Lua::PopSandboxEnv() {
    if (envs_.empty())
	return;
    luaL_unref(lua_, LUA_REGISTRYINDEX, envs_.back());
    envs_.pop_back();
}

//! Pushes the safe table.
void Lua::PushSafe() {
    lua_pushlightuserdata(lua_, &safeRegistryKey);
    lua_gettable(lua_, LUA_REGISTRYINDEX);
}

//! Creates a sandbox env.
void Lua::PushSandboxEnv() {
    lua_newtable(lua_);
    const int env = lua_absindex(lua_, -1);

    // __index = safe.
    lua_newtable(lua_);
    this->PushSafe();
    lua_setfield(lua_, -2, "__index");
    lua_setmetatable(lua_, env);

    // _G = env.
    lua_pushvalue(lua_, env);
    lua_setfield(lua_, env, "_G");

    lua_pushvalue(lua_, env);
    const int ref = luaL_ref(lua_, LUA_REGISTRYINDEX);
    envs_.push_back(ref);
    lua_pop(lua_, 1);
}

}; // namespace Scripting
}; // namespace Scratch
