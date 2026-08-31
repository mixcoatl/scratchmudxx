//! \file descriptor_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_DESCRIPTOR_BINDINGS_CPP_

#include <scratch/color.hpp>
#include <scratch/color_bindings.hpp>
#include <scratch/command.hpp>
#include <scratch/command_bindings.hpp>
#include <scratch/descriptor.hpp>
#include <scratch/descriptor_bindings.hpp>
#include <scratch/editor.hpp>
#include <scratch/lua.hpp>
#include <scratch/menu.hpp>
#include <scratch/player_bindings.hpp>
#include <scratch/scratch.hpp>
#include <scratch/state.hpp>
#include <scratch/state_bindings.hpp>
#include <scratch/string.hpp>
#include <scratch/user.hpp>
#include <scratch/user_bindings.hpp>

namespace Scratch {
namespace Scripting {

// ScratchMUD types.
using Color = Scratch::Net::Color;
using EditorPtr = Scratch::Net::EditorPtr;
using Menu = Scratch::Net::Menu;

//! Metatable name for Descriptor userdata.
const char DescriptorBindings::MetaName[] = "Scratch.Descriptor";

using WeakDescriptorPtr = std::weak_ptr<Descriptor>;

static WeakDescriptorPtr CheckWeakDescriptorPtr(
	lua_State* L,
	const int index = 1);

//! Errors if the editor is intercepting input.
//! \param L the \c lua_State
//! \param d the descriptor
//! \remark Resets \a d before \c luaL_error.
static void RequireNoEditorActive(
	lua_State* L,
	DescriptorPtr& d) {
    if (d && d->IsEditorActive()) {
	d.reset();
	luaL_error(L, "editor is active");
    }
}

//! Enforces the FocusLost navigation ban.
//! \param L the \c lua_State
//! \param method the Lua method name for the error text
//! \remark Raises if the Caller identity ends with \c :FocusLost.
static void ForbidNavigationFromFocusLost(
	lua_State* L,
	const char* method) {
    static const char focusLostSuffix[] = ":FocusLost";
    const auto caller = Lua::CheckCaller(L);
    if (caller.size() >= sizeof(focusLostSuffix) - 1 &&
	caller.compare(
	    caller.size() - (sizeof(focusLostSuffix) - 1),
	    sizeof(focusLostSuffix) - 1,
	    focusLostSuffix) == 0)
	luaL_error(L, "%s cannot be called from FocusLost", method);
}

//! Resolves a Descriptor userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return a temporary shared pointer; Game remains the owner
//! \remark Do not hold across \c luaL_error (skips C++ destructors).
DescriptorPtr DescriptorBindings::Check(
	lua_State* L,
	const int index) {
    return Lua::CheckWeakUserdata<Descriptor>(
	L, MetaName, "invalid descriptor", index);
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

//! Handles Descriptor:login(user).
static int DescriptorLogin(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "login expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    auto self = DescriptorBindings::Check(L);
    RequireNoEditorActive(L, self);
    auto user = UserBindings::Check(L, 2);
    self->Login(user);
    user.reset();
    Descriptor& desc = *self;
    self.reset();
    ColorBindings::RefillGlobalQ(L, lua, desc);
    return 0;
}

//! Parses a metacolor name for menu value coloring.
//! \param L the \c lua_State
//! \param index the stack index of the color name
static Color::ColorEnum CheckMetaColorName(
	lua_State* L,
	const int index) {
    const auto name = Lua::CheckString(L, index);
    const auto color = Color::ByName(name);
    if (!Color::IsMetaColor(color))
	luaL_argerror(L, index, "invalid metacolor");
    return color;
}

//! Handles Descriptor:menu_item(key, label).
static int DescriptorMenuItem(lua_State* L) {
    if (lua_gettop(L) != 3)
	return luaL_error(L, "menu_item expects 2 arguments");
    luaL_checktype(L, 2, LUA_TSTRING);
    luaL_checktype(L, 3, LUA_TSTRING);
    String key;
    if (!Menu::CanonicalizeKey(Lua::CheckString(L, 2), key))
	return luaL_argerror(L, 2, "invalid menu key");
    const auto label = Lua::CheckString(L, 3);
    auto menu = Lua::CheckWeakUserdata<Menu>(
	L, "Scratch.Menu", "invalid menu");
    if (!menu->HasSection()) {
	menu.reset();
	return luaL_error(L, "menu_item requires a menu section first");
    }
    if (!menu->AddItem(key, label)) {
	menu.reset();
	return luaL_argerror(L, 2, "duplicate choice key");
    }
    return 0;
}

//! Handles Descriptor:menu_prompt(text).
static int DescriptorMenuPrompt(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "menu_prompt expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    const auto prompt = Lua::CheckString(L, 2);
    if (prompt.empty())
	return luaL_argerror(L, 2, "prompt must not be empty");
    auto menu = Lua::CheckWeakUserdata<Menu>(
	L, "Scratch.Menu", "invalid menu");
    menu->SetPrompt(prompt);
    return 0;
}

//! Handles Descriptor:menu_title(...).
//! \remark \c menu_title(text) or
//!     \c menu_title(field, value [, color [, empty]]).
static int DescriptorMenuTitle(lua_State* L) {
    const int argc = lua_gettop(L);
    if (argc == 2) {
	luaL_checktype(L, 2, LUA_TSTRING);
	auto menu = Lua::CheckWeakUserdata<Menu>(
	    L, "Scratch.Menu", "invalid menu");
	menu->SetTitle(
	    Lua::CheckString(L, 2));
	return 0;
    }
    if (argc < 3 || argc > 5)
	return luaL_error(L,
	    "menu_title expects 1 or 2 to 4 arguments");
    luaL_checktype(L, 2, LUA_TSTRING);
    luaL_checktype(L, 3, LUA_TSTRING);
    auto valueColor = Color::C_TEXT;
    if (argc >= 4) {
	luaL_checktype(L, 4, LUA_TSTRING);
	valueColor = CheckMetaColorName(L, 4);
    }
    if (argc >= 5)
	luaL_checktype(L, 5, LUA_TSTRING);
    auto menu = Lua::CheckWeakUserdata<Menu>(
	L, "Scratch.Menu", "invalid menu");
    const auto field = Lua::CheckString(L, 2);
    const auto value = Lua::CheckString(L, 3);
    String empty = "<Blank>";
    if (argc >= 5) {
	empty = Lua::CheckString(L, 5);
    }
    menu->SetTitle(
	field, value, valueColor, empty);
    return 0;
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

//! Handles Descriptor:print_format(fmt, ...).
static int DescriptorPrintFormat(lua_State* L) {
    DescriptorBindings::Check(L);
    const int howMany = lua_gettop(L);
    if (howMany < 2)
	return luaL_argerror(L, 2, "format string expected");
    lua_getglobal(L, "string");
    if (!lua_istable(L, -1)) {
	lua_pop(L, 1);
	return luaL_error(L, "string library missing");
    }
    lua_getfield(L, -1, "format");
    lua_remove(L, -2);
    if (!lua_isfunction(L, -1)) {
	lua_pop(L, 1);
	return luaL_error(L, "string.format missing");
    }
    // Stack: self, fmt, args..., format.
    lua_insert(L, 2);
    // Stack: self, format, fmt, args...
    lua_call(L, howMany - 1, 1);
    // Stack: self, result.
    const bool ok = TryDescriptorPrint(L, Lua::CheckString(L, -1));
    lua_pop(L, 1);
    if (!ok)
	return luaL_error(L, "invalid descriptor");
    return 0;
}

//! Handles Descriptor:print_menu().
static int DescriptorPrintMenu(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "print_menu expects no arguments");
    auto d = DescriptorBindings::Check(L);
    RequireNoEditorActive(L, d);
    auto menu = d->GetMenu();
    if (!menu) {
	d.reset();
	return luaL_error(L, "print_menu requires a menu");
    }
    if (!menu->HasPrompt()) {
	d.reset();
	menu.reset();
	return luaL_error(L, "print_menu requires menu_prompt");
    }
    if (!d->PrintMenu()) {
	d.reset();
	menu.reset();
	return luaL_error(L, "print_menu failed");
    }
    return 0;
}

//! Handles Descriptor:set_color(color).
static int DescriptorSetColor(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_color expects 1 argument");
    luaL_checktype(L, 2, LUA_TBOOLEAN);
    auto& lua = Lua::CheckLua(L);
    auto self = DescriptorBindings::Check(L);
    RequireNoEditorActive(L, self);
    self->SetColorBit(lua_toboolean(L, 2) != 0);
    Descriptor& desc = *self;
    self.reset();
    ColorBindings::RefillGlobalQ(L, lua, desc);
    return 0;
}

//! Handles Descriptor:set_edit_string(value).
static int DescriptorSetEditString(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_edit_string expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto d = DescriptorBindings::Check(L);
    RequireNoEditorActive(L, d);
    d->SetEditString(Lua::CheckString(L, 2));
    return 0;
}

//! Handles Descriptor:set_prompt(prompt).
static int DescriptorSetPrompt(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_prompt expects 1 argument");
    luaL_checktype(L, 2, LUA_TBOOLEAN);
    auto d = DescriptorBindings::Check(L);
    RequireNoEditorActive(L, d);
    d->SetPromptBit(lua_toboolean(L, 2) != 0);
    return 0;
}

//! Handles Descriptor:pop_state().
static int DescriptorPopState(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "pop_state expects no arguments");
    ForbidNavigationFromFocusLost(L, "pop_state");
    auto d = DescriptorBindings::Check(L);
    RequireNoEditorActive(L, d);
    d->PopState();
    return 0;
}

//! Handles Descriptor:pop_state_until(name|state).
static int DescriptorPopStateUntil(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "pop_state_until expects 1 argument");
    ForbidNavigationFromFocusLost(L, "pop_state_until");
    const auto weakD = CheckWeakDescriptorPtr(L);
    if (weakD.expired())
	return luaL_error(L, "invalid descriptor");
    {
	auto d = weakD.lock();
	RequireNoEditorActive(L, d);
    }
    if (luaL_testudata(L, 2, StateBindings::MetaName)) {
	auto state = Lua::CheckWeakUserdata<State>(L, "Scratch.State", "invalid state", 2);
	auto d = weakD.lock();
	if (!d) {
	    state.reset();
	    return luaL_error(L, "invalid descriptor");
	}
	d->PopStateUntil(state);
    } else {
	const auto stateName = Lua::CheckString(L, 2);
	auto d = weakD.lock();
	if (!d)
	    return luaL_error(L, "invalid descriptor");
	d->PopStateUntilByName(stateName);
    }
    return 0;
}

//! Handles Descriptor:push_state(name|state).
static int DescriptorPushState(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "push_state expects 1 argument");
    ForbidNavigationFromFocusLost(L, "push_state");
    const auto weakD = CheckWeakDescriptorPtr(L);
    if (weakD.expired())
	return luaL_error(L, "invalid descriptor");
    {
	auto d = weakD.lock();
	RequireNoEditorActive(L, d);
    }
    if (luaL_testudata(L, 2, StateBindings::MetaName)) {
	auto state = Lua::CheckWeakUserdata<State>(L, "Scratch.State", "invalid state", 2);
	auto d = weakD.lock();
	if (!d) {
	    state.reset();
	    return luaL_error(L, "invalid descriptor");
	}
	d->PushState(state);
    } else {
	const auto stateName = Lua::CheckString(L, 2);
	auto d = weakD.lock();
	if (!d)
	    return luaL_error(L, "invalid descriptor");
	d->PushStateByName(stateName);
    }
    return 0;
}

//! Handles Descriptor:set_state(name|state).
static int DescriptorSetState(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_state expects 1 argument");
    ForbidNavigationFromFocusLost(L, "set_state");
    const auto weakD = CheckWeakDescriptorPtr(L);
    if (weakD.expired())
	return luaL_error(L, "invalid descriptor");
    {
	auto d = weakD.lock();
	RequireNoEditorActive(L, d);
    }
    if (luaL_testudata(L, 2, StateBindings::MetaName)) {
	auto state = Lua::CheckWeakUserdata<State>(L, "Scratch.State", "invalid state", 2);
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

//! Handles Descriptor:start_editor(text [, tag [, maxLength]]).
static int DescriptorStartEditor(lua_State* L) {
    const int argc = lua_gettop(L);
    if (argc < 2 || argc > 4)
	return luaL_error(L, "start_editor expects 1 to 3 arguments");
    luaL_checktype(L, 2, LUA_TSTRING);
    const auto text = Lua::CheckString(L, 2);
    String tag;
    std::size_t maxLength = 1024;
    if (argc >= 3) {
	if (!lua_isnil(L, 3)) {
	    luaL_checktype(L, 3, LUA_TSTRING);
	    tag = Lua::CheckString(L, 3);
	}
    }
    if (argc >= 4) {
	if (!lua_isnil(L, 4)) {
	    luaL_checktype(L, 4, LUA_TNUMBER);
	    const auto n = lua_tointeger(L, 4);
	    if (n < 0)
		return luaL_argerror(L, 4, "maxLength must be non-negative");
	    maxLength = static_cast<std::size_t>(n);
	}
    }
    auto d = DescriptorBindings::Check(L);
    if (d->IsEditorActive()) {
	d.reset();
	return luaL_error(L, "editor is active");
    }
    d->EnsureEditor()->Start(text, tag, maxLength);
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

void DescriptorBindings::Register(Lua& lua) {
    lua.Class<Menu>("Scratch.Menu").
	RawFunction("add_item", DescriptorMenuItem).
	RawFunction("set_prompt", DescriptorMenuPrompt).
	RawFunction("set_title", DescriptorMenuTitle);
    lua.Class<Descriptor>(MetaName).
	Function("clear_edit_command", &Descriptor::ClearEditCommand).
	Function("clear_edit_exit", &Descriptor::ClearEditExit).
	Function("clear_edit_player", &Descriptor::ClearEditPlayer).
	Function("clear_edit_room", &Descriptor::ClearEditRoom).
	Function("clear_edit_state", &Descriptor::ClearEditState).
	Function("clear_edit_user", &Descriptor::ClearEditUser).
	Function("clear_edit_zone", &Descriptor::ClearEditZone).
	Function("clear_editor", &Descriptor::ClearEditor).
	Function("clear_menu", &Descriptor::ClearMenu).
	Function("close", &Descriptor::Close).
	Function("create_character", &Descriptor::CreateCharacter).
	Function("ensure_menu", &Descriptor::EnsureMenu).
	Function("get_character", &Descriptor::GetCharacter).
	Function("get_edit_command", &Descriptor::GetEditCommand).
	Function("get_edit_exit", &Descriptor::GetEditExit).
	Function("get_edit_name", &Descriptor::GetEditName).
	Function("get_edit_player", &Descriptor::GetEditPlayer).
	Function("get_edit_room", &Descriptor::GetEditRoom).
	Function("get_edit_state", &Descriptor::GetEditState).
	Function("get_edit_string", &Descriptor::GetEditString).
	Function("get_edit_user", &Descriptor::GetEditUser).
	Function("get_editor", &Descriptor::GetEditorProxy).
	Function("get_menu", &Descriptor::GetMenu).
	Function("get_name", &Descriptor::GetName).
	Function("get_state", &Descriptor::GetState).
	Function("get_terminal_type", &Descriptor::GetTerminalType).
	Function("get_user", &Descriptor::GetUser).
	Function("get_window_height", &Descriptor::GetWindowHeight).
	Function("get_window_width", &Descriptor::GetWindowWidth).
	Function("is_closed", &Descriptor::ClosedProxy).
	Function("is_color", &Descriptor::GetColorBit).
	Function("is_prompt", &Descriptor::GetPromptBit).
	RawFunction("login", DescriptorLogin).
	RawFunction("pop_state", DescriptorPopState).
	RawFunction("pop_state_until", DescriptorPopStateUntil).
	RawFunction("print", DescriptorPrint).
	Function("print_columns", &Descriptor::PrintColumns).
	RawFunction("print_format", DescriptorPrintFormat).
	RawFunction("print_menu", DescriptorPrintMenu).
	RawFunction("push_state", DescriptorPushState).
	Function("set_character", &Descriptor::SetCharacter).
	RawFunction("set_color", DescriptorSetColor).
	Function("set_edit_command", &Descriptor::SetEditCommand).
	Function("set_edit_exit", &Descriptor::SetEditExit).
	Function("set_edit_player", &Descriptor::SetEditPlayer).
	Function("set_edit_room", &Descriptor::SetEditRoom).
	Function("set_edit_state", &Descriptor::SetEditState).
	RawFunction("set_edit_string", DescriptorSetEditString).
	Function("set_edit_user", &Descriptor::SetEditUser).
	Function("set_edit_zone", &Descriptor::SetEditZone).
	RawFunction("set_prompt", DescriptorSetPrompt).
	RawFunction("set_state", DescriptorSetState).
	RawFunction("start_editor", DescriptorStartEditor);
}

}; // namespace Scripting
}; // namespace Scratch
