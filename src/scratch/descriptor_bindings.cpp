//! \file descriptor_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_DESCRIPTOR_BINDINGS_CPP_

#include <scratch/descriptor.hpp>
#include <scratch/descriptor_bindings.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>
#include <scratch/state.hpp>
#include <scratch/state_bindings.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Scripting {

//! Metatable name for Descriptor userdata.
const char DescriptorBindings::MetaName[] = "Scratch.Descriptor";

//! Non-owning handle stored in Descriptor userdata.
using WeakDescriptorPtr = std::weak_ptr<Descriptor>;

static WeakDescriptorPtr CheckWeakDescriptorPtr(
	lua_State* L,
	const int index = 1);

//! Resolves a Descriptor userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return a temporary shared pointer; Game remains the owner
//! \remark Do not hold across \c luaL_error (skips C++ destructors).
DescriptorPtr DescriptorBindings::Check(
	lua_State* L,
	const int index) {
    DescriptorPtr d = CheckWeakDescriptorPtr(L, index).lock();
    if (!d)
	luaL_argerror(L, index, "invalid descriptor");
    return d;
}

//! Returns a copy of the weak Descriptor handle stored in userdata.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
static WeakDescriptorPtr CheckWeakDescriptorPtr(
	lua_State* L,
	const int index) {
    return *static_cast<WeakDescriptorPtr*>(
	luaL_checkudata(L, index, DescriptorBindings::MetaName));
}

//! Handles Descriptor:clear_edit_state().
static int DescriptorClearEditState(lua_State* L) {
    DescriptorBindings::Check(L)->SetEditState(nullptr);
    return 0;
}

//! Handles Descriptor:close().
static int DescriptorClose(lua_State* L) {
    DescriptorBindings::Check(L)->Close();
    return 0;
}

//! Handles Descriptor userdata garbage collection.
static int DescriptorGc(lua_State* L) {
    // Destroy the userdata in place; a by-value Check would only
    // destroy a copy.
    static_cast<WeakDescriptorPtr*>(
	luaL_checkudata(L, 1, DescriptorBindings::MetaName))->~WeakDescriptorPtr();
    return 0;
}

//! Handles Descriptor:get_edit_state().
static int DescriptorGetEditState(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    StateBindings::Push(lua, DescriptorBindings::Check(L)->GetEditState());
    return 1;
}

//! Handles Descriptor:get_name().
static int DescriptorGetName(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto name = DescriptorBindings::Check(L)->GetName();
    lua.PushString(std::move(name));
    return 1;
}

//! Handles Descriptor:get_state().
static int DescriptorGetState(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    StateBindings::Push(lua, DescriptorBindings::Check(L)->GetState());
    return 1;
}

//! Handles Descriptor:get_terminal_type().
static int DescriptorGetTerminalType(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto terminalType = DescriptorBindings::Check(L)->GetTerminalType();
    lua.PushString(std::move(terminalType));
    return 1;
}

//! Handles Descriptor:get_window_height().
static int DescriptorGetWindowHeight(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    const auto height = DescriptorBindings::Check(L)->GetWindowHeight();
    lua.PushInt(height);
    return 1;
}

//! Handles Descriptor:get_window_width().
static int DescriptorGetWindowWidth(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    const auto width = DescriptorBindings::Check(L)->GetWindowWidth();
    lua.PushInt(width);
    return 1;
}

//! Handles Descriptor:is_closed().
static int DescriptorIsClosed(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto d = CheckWeakDescriptorPtr(L).lock();
    const bool closed = !d || d->Closed();
    d.reset();
    lua.PushBool(closed);
    return 1;
}

//! Handles Descriptor:is_color().
static int DescriptorIsColor(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "is_color expects no arguments");
    auto& lua = Lua::CheckLua(L);
    const bool color = DescriptorBindings::Check(L)->GetColorBit();
    lua.PushBool(color);
    return 1;
}

//! Handles Descriptor:is_prompt().
static int DescriptorIsPrompt(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "is_prompt expects no arguments");
    auto& lua = Lua::CheckLua(L);
    const bool prompt = DescriptorBindings::Check(L)->GetPromptBit();
    lua.PushBool(prompt);
    return 1;
}

//! Prints \p message to the Descriptor at stack index 1.
//! \return \c true if the Descriptor was valid
static bool TryDescriptorPrint(
	lua_State* L,
	const String& message) {
    DescriptorPtr d = CheckWeakDescriptorPtr(L).lock();
    if (!d)
	return false;
    d->Print(message);
    return true;
}

//! Handles Descriptor:print(...).
static int DescriptorPrint(lua_State* L) {
    DescriptorBindings::Check(L);
    const int howMany = lua_gettop(L);
    luaL_Buffer buffer;
    luaL_buffinit(L, &buffer);
    for (auto n = 2; n <= howMany; ++n) {
	if (n > 2)
	    luaL_addchar(&buffer, '\t');
	luaL_tolstring(L, n, nullptr);
	luaL_addvalue(&buffer);
    }
    luaL_pushresult(&buffer);
    const bool ok = TryDescriptorPrint(L, Lua::CheckString(L, -1));
    lua_pop(L, 1);
    if (!ok)
	return luaL_error(L, "invalid descriptor");
    return 0;
}

//! Handles Descriptor:set_color(color).
static int DescriptorSetColor(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_color expects 1 argument");
    DescriptorBindings::Check(L)->SetColorBit(lua_toboolean(L, 2) != 0);
    return 0;
}

//! Handles Descriptor:set_edit_state([state]).
//! \remark Draft copy of \p state, or blank when omitted.
static int DescriptorSetEditState(lua_State* L) {
    const int argc = lua_gettop(L);
    if (argc != 1 && argc != 2)
	return luaL_error(L, "set_edit_state expects 0 or 1 arguments");
    const auto weakD = CheckWeakDescriptorPtr(L);
    if (weakD.expired())
	return luaL_error(L, "invalid descriptor");
    auto& lua = Lua::CheckLua(L);
    StatePtr editState;
    if (argc == 2) {
	auto source = StateBindings::Check(L, 2);
	editState = std::make_shared<State>(*source);
	source.reset();
    } else {
	editState = std::make_shared<State>();
    }
    auto d = weakD.lock();
    if (!d) {
	editState.reset();
	return luaL_error(L, "invalid descriptor");
    }
    d->SetEditState(editState);
    d.reset();
    StateBindings::Push(lua, std::move(editState));
    return 1;
}

//! Handles Descriptor:set_prompt(prompt).
static int DescriptorSetPrompt(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_prompt expects 1 argument");
    DescriptorBindings::Check(L)->SetPromptBit(lua_toboolean(L, 2) != 0);
    return 0;
}

//! Handles Descriptor:set_state(name|state).
static int DescriptorSetState(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_state expects 1 argument");
    // FocusLost is cleanup-only; navigating from it re-enters the leave hook.
    static const char focusLostSuffix[] = ":FocusLost";
    const auto caller = Lua::CheckCaller(L);
    if (caller.size() >= sizeof(focusLostSuffix) - 1 &&
	caller.compare(
	    caller.size() - (sizeof(focusLostSuffix) - 1),
	    sizeof(focusLostSuffix) - 1,
	    focusLostSuffix) == 0)
	return luaL_error(L, "set_state cannot be called from FocusLost");
    const auto weakD = CheckWeakDescriptorPtr(L);
    if (weakD.expired())
	return luaL_error(L, "invalid descriptor");
    if (luaL_testudata(L, 2, StateBindings::MetaName)) {
	auto state = StateBindings::Check(L, 2);
	auto d = weakD.lock();
	if (!d) {
	    state.reset();
	    return luaL_error(L, "invalid descriptor");
	}
	d->SetState(state);
    } else {
	const auto stateName = Lua::CheckString(L, 2);
	auto d = weakD.lock();
	if (!d)
	    return luaL_error(L, "invalid descriptor");
	d->SetStateByName(stateName);
    }
    return 0;
}

//! Pushes a Descriptor userdata, or nil.
//! \param lua the Lua facade
//! \param d the descriptor to push
void DescriptorBindings::Push(
	Lua& lua,
	DescriptorPtr d) {
    lua.PushUserdata(std::move(d), MetaName);
}

//! Registers the Descriptor metatable.
//! \param lua the Lua facade
void DescriptorBindings::Register(Lua& lua) {
    lua_State* L = lua.GetState();
    luaL_newmetatable(L, MetaName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    static const luaL_Reg methods[] = {
	{"__gc", DescriptorGc},
	{"clear_edit_state", DescriptorClearEditState},
	{"close", DescriptorClose},
	{"get_edit_state", DescriptorGetEditState},
	{"get_name", DescriptorGetName},
	{"get_state", DescriptorGetState},
	{"get_terminal_type", DescriptorGetTerminalType},
	{"get_window_height", DescriptorGetWindowHeight},
	{"get_window_width", DescriptorGetWindowWidth},
	{"is_closed", DescriptorIsClosed},
	{"is_color", DescriptorIsColor},
	{"is_prompt", DescriptorIsPrompt},
	{"print", DescriptorPrint},
	{"set_color", DescriptorSetColor},
	{"set_edit_state", DescriptorSetEditState},
	{"set_prompt", DescriptorSetPrompt},
	{"set_state", DescriptorSetState},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

}; // namespace Scripting
}; // namespace Scratch
