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
#include <scratch/descriptor.hpp>
#include <scratch/descriptor_bindings.hpp>
#include <scratch/editor.hpp>
#include <scratch/editor_bindings.hpp>
#include <scratch/lua.hpp>
#include <scratch/menu.hpp>
#include <scratch/scratch.hpp>
#include <scratch/state.hpp>
#include <scratch/state_bindings.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Scripting {

using Descriptor = Scratch::Net::Descriptor;

using WeakDescriptorPtr = std::weak_ptr<Descriptor>;

//! Metatable name for Descriptor userdata.
const char DescriptorBindings::MetaName[] = "Scratch.Descriptor";

using Color = Scratch::Net::Color;
using Menu = Scratch::Net::Menu;

static WeakDescriptorPtr CheckWeakDescriptorPtr(
	lua_State* L,
	const int index = 1);

//! Raises if the descriptor's editor is intercepting input.
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

//! Handles Descriptor:clear_menu().
static int DescriptorClearMenu(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "clear_menu expects no arguments");
    auto d = DescriptorBindings::Check(L);
    RequireNoEditorActive(L, d);
    d->ClearMenu();
    return 0;
}


static Color::ColorEnum CheckMetaColorName(
	lua_State* L,
	const int index) {
    const auto name = Lua::CheckString(L, index);
    const auto color = Color::ByName(name);
    if (!Color::IsMetaColor(color))
	luaL_argerror(L, index, "invalid metacolor");
    return color;
}

//! Opens a menu section; \p named requires a title argument.
//! \param L the \c lua_State
//! \param kind the section kind
//! \param named whether a title is required
//! \param verb the Lua method name (for errors)
static int DescriptorMenuOpenSection(
	lua_State* L,
	const Menu::SectionKind kind,
	const bool named,
	const char *const verb) {
    const int argc = lua_gettop(L);
    bool fold = false;
    String title;
    if (named) {
	if (argc != 2 && argc != 3)
	    return luaL_error(L, "%s expects 1 or 2 arguments", verb);
	luaL_checktype(L, 2, LUA_TSTRING);
	title = Lua::CheckString(L, 2);
	if (argc == 3) {
	    luaL_checktype(L, 3, LUA_TBOOLEAN);
	    fold = lua_toboolean(L, 3) != 0;
	}
    } else {
	if (argc != 1 && argc != 2)
	    return luaL_error(L, "%s expects 0 or 1 arguments", verb);
	if (argc == 2) {
	    luaL_checktype(L, 2, LUA_TBOOLEAN);
	    fold = lua_toboolean(L, 2) != 0;
	}
    }
    auto d = DescriptorBindings::Check(L);
    RequireNoEditorActive(L, d);
    d->EnsureMenu()->AddSection(fold, kind, title);
    return 0;
}

//! Shared field/block arg parse; \p block selects layout.
//! \param L the \c lua_State
//! \param block whether to add a block field
static int DescriptorMenuFieldOrBlock(
	lua_State* L,
	const bool block) {
    const char *const verb = block ? "menu_block" : "menu_field";
    const int argc = lua_gettop(L);
    if (argc < 4 || argc > 7)
	return luaL_error(L, "%s expects 3 to 6 arguments", verb);
    luaL_checktype(L, 2, LUA_TSTRING);
    luaL_checktype(L, 3, LUA_TSTRING);
    luaL_checktype(L, 4, LUA_TSTRING);
    String key;
    if (!Menu::CanonicalizeKey(Lua::CheckString(L, 2), key))
	return luaL_argerror(L, 2, "invalid menu key");
    const auto field = Lua::CheckString(L, 3);
    const auto value = Lua::CheckString(L, 4);
    auto valueColor = Color::C_TEXT;
    String empty = "<Blank>";
    String shown;
    if (argc >= 5) {
	luaL_checktype(L, 5, LUA_TSTRING);
	valueColor = CheckMetaColorName(L, 5);
    }
    if (argc >= 6) {
	luaL_checktype(L, 6, LUA_TSTRING);
	empty = Lua::CheckString(L, 6);
    }
    if (argc >= 7) {
	luaL_checktype(L, 7, LUA_TSTRING);
	shown = Lua::CheckString(L, 7);
    }
    auto d = DescriptorBindings::Check(L);
    RequireNoEditorActive(L, d);
    auto menu = d->EnsureMenu();
    if (!menu->HasSection()) {
	d.reset();
	menu.reset();
	return luaL_error(L, "%s requires a menu section first", verb);
    }
    const bool ok = block ?
	menu->AddBlock(key, field, value, valueColor, empty, shown) :
	menu->AddField(key, field, value, valueColor, empty, shown);
    if (!ok) {
	d.reset();
	menu.reset();
	if (block)
	    return luaL_error(L,
		"menu_block is not allowed in folded sections");
	return luaL_argerror(L, 2, "duplicate choice key");
    }
    return 0;
}

//! Handles Descriptor:menu_block(key, field, value [, color [, empty [, shown]]]).
static int DescriptorMenuBlock(lua_State* L) {
    return DescriptorMenuFieldOrBlock(L, true);
}

//! Handles Descriptor:menu_choices([fold]).
static int DescriptorMenuChoices(lua_State* L) {
    return DescriptorMenuOpenSection(
	L, Menu::SectionKind::Choices, false, "menu_choices");
}

//! Handles Descriptor:menu_field(key, field, value [, color [, empty [, shown]]]).
static int DescriptorMenuField(lua_State* L) {
    return DescriptorMenuFieldOrBlock(L, false);
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
    auto d = DescriptorBindings::Check(L);
    RequireNoEditorActive(L, d);
    auto menu = d->EnsureMenu();
    if (!menu->HasSection()) {
	d.reset();
	menu.reset();
	return luaL_error(L, "menu_item requires a menu section first");
    }
    if (!menu->AddItem(key, label)) {
	d.reset();
	menu.reset();
	return luaL_argerror(L, 2, "duplicate choice key");
    }
    return 0;
}

//! Handles Descriptor:menu_listing([fold]).
static int DescriptorMenuListing(lua_State* L) {
    return DescriptorMenuOpenSection(
	L, Menu::SectionKind::Listing, false, "menu_listing");
}

//! Handles Descriptor:menu_match(line).
static int DescriptorMenuMatch(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "menu_match expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto& lua = Lua::CheckLua(L);
    auto d = DescriptorBindings::Check(L);
    RequireNoEditorActive(L, d);
    auto menu = d->GetMenu();
    if (!menu) {
	lua_pushnil(L);
	return 1;
    }
    String key;
    if (!menu->Match(Lua::CheckString(L, 2), key)) {
	lua_pushnil(L);
	return 1;
    }
    lua.PushString(std::move(key));
    return 1;
}

//! Handles Descriptor:menu_named_choices(title [, fold]).
static int DescriptorMenuNamedChoices(lua_State* L) {
    return DescriptorMenuOpenSection(
	L, Menu::SectionKind::Choices, true, "menu_named_choices");
}

//! Handles Descriptor:menu_named_listing(title [, fold]).
static int DescriptorMenuNamedListing(lua_State* L) {
    return DescriptorMenuOpenSection(
	L, Menu::SectionKind::Listing, true, "menu_named_listing");
}

//! Handles Descriptor:menu_prompt(text).
static int DescriptorMenuPrompt(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "menu_prompt expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    const auto prompt = Lua::CheckString(L, 2);
    if (prompt.empty())
	return luaL_argerror(L, 2, "prompt must not be empty");
    auto d = DescriptorBindings::Check(L);
    RequireNoEditorActive(L, d);
    d->EnsureMenu()->SetPrompt(prompt);
    return 0;
}

//! Handles Descriptor:menu_title(...).
//! \remark \c menu_title(text) or
//!     \c menu_title(field, value [, color [, empty]]).
static int DescriptorMenuTitle(lua_State* L) {
    const int argc = lua_gettop(L);
    if (argc == 2) {
	luaL_checktype(L, 2, LUA_TSTRING);
	auto d = DescriptorBindings::Check(L);
	RequireNoEditorActive(L, d);
	d->EnsureMenu()->SetTitle(
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
    auto d = DescriptorBindings::Check(L);
    RequireNoEditorActive(L, d);
    const auto field = Lua::CheckString(L, 2);
    const auto value = Lua::CheckString(L, 3);
    String empty = "<Blank>";
    if (argc >= 5) {
	empty = Lua::CheckString(L, 5);
    }
    d->EnsureMenu()->SetTitle(
	field, value, valueColor, empty);
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

//! Handles Descriptor:clear_edit_state().
static int DescriptorClearEditState(lua_State* L) {
    auto d = DescriptorBindings::Check(L);
    RequireNoEditorActive(L, d);
    d->SetEditState(nullptr);
    return 0;
}

//! Handles Descriptor:clear_editor().
static int DescriptorClearEditor(lua_State* L) {
    DescriptorBindings::Check(L)->ClearEditor();
    return 0;
}

//! Handles Descriptor:close().
static int DescriptorClose(lua_State* L) {
    DescriptorBindings::Check(L)->Close();
    return 0;
}

//! Handles Descriptor userdata garbage collection.
static int DescriptorGc(lua_State* L) {
    return Lua::DestroyWeakUserdata<Descriptor>(
	L, DescriptorBindings::MetaName);
}

//! Handles Descriptor:get_edit_state().
static int DescriptorGetEditState(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    StateBindings::Push(lua, DescriptorBindings::Check(L)->GetEditState());
    return 1;
}

//! Handles Descriptor:get_editor().
static int DescriptorGetEditor(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto d = DescriptorBindings::Check(L);
    auto editor = d->GetEditor();
    d.reset();
    if (!editor || editor->IsActive()) {
	editor.reset();
	lua_pushnil(L);
	return 1;
    }
    EditorBindings::Push(lua, std::move(editor));
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
	auto state = StateBindings::Check(L, 2);
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

//! Handles Descriptor:print_columns(cells).
static int DescriptorPrintColumns(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "print_columns expects 1 argument");
    std::vector<String> cells;
    Lua::CheckStringArray(L, cells, 2, "string array expected");
    auto d = DescriptorBindings::Check(L);
    d->PrintColumns(cells);
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
	auto state = StateBindings::Check(L, 2);
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

//! Handles Descriptor:set_edit_state([state]).
//! \remark Draft copy of \p state, or blank when omitted.
static int DescriptorSetEditState(lua_State* L) {
    const int argc = lua_gettop(L);
    if (argc != 1 && argc != 2)
	return luaL_error(L, "set_edit_state expects 0 or 1 arguments");
    const auto weakD = CheckWeakDescriptorPtr(L);
    if (weakD.expired())
	return luaL_error(L, "invalid descriptor");
    {
	auto d = weakD.lock();
	RequireNoEditorActive(L, d);
    }
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
    luaL_checktype(L, 2, LUA_TBOOLEAN);
    auto d = DescriptorBindings::Check(L);
    RequireNoEditorActive(L, d);
    d->SetPromptBit(lua_toboolean(L, 2) != 0);
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

//! Registers the Descriptor metatable.
//! \param lua the Lua facade
void DescriptorBindings::Register(Lua& lua) {
    lua_State* L = lua.GetState();
    Lua::RegisterMetatable(L, MetaName);

    static const luaL_Reg methods[] = {
	{"__gc", DescriptorGc},
	{"clear_edit_state", DescriptorClearEditState},
	{"clear_editor", DescriptorClearEditor},
	{"clear_menu", DescriptorClearMenu},
	{"close", DescriptorClose},
	{"get_edit_state", DescriptorGetEditState},
	{"get_editor", DescriptorGetEditor},
	{"get_name", DescriptorGetName},
	{"get_state", DescriptorGetState},
	{"get_terminal_type", DescriptorGetTerminalType},
	{"get_window_height", DescriptorGetWindowHeight},
	{"get_window_width", DescriptorGetWindowWidth},
	{"is_closed", DescriptorIsClosed},
	{"is_color", DescriptorIsColor},
	{"is_prompt", DescriptorIsPrompt},
	{"menu_block", DescriptorMenuBlock},
	{"menu_choices", DescriptorMenuChoices},
	{"menu_field", DescriptorMenuField},
	{"menu_item", DescriptorMenuItem},
	{"menu_listing", DescriptorMenuListing},
	{"menu_match", DescriptorMenuMatch},
	{"menu_named_choices", DescriptorMenuNamedChoices},
	{"menu_named_listing", DescriptorMenuNamedListing},
	{"menu_prompt", DescriptorMenuPrompt},
	{"menu_title", DescriptorMenuTitle},
	{"pop_state", DescriptorPopState},
	{"pop_state_until", DescriptorPopStateUntil},
	{"print", DescriptorPrint},
	{"print_columns", DescriptorPrintColumns},
	{"print_format", DescriptorPrintFormat},
	{"print_menu", DescriptorPrintMenu},
	{"push_state", DescriptorPushState},
	{"set_color", DescriptorSetColor},
	{"set_edit_state", DescriptorSetEditState},
	{"set_prompt", DescriptorSetPrompt},
	{"set_state", DescriptorSetState},
	{"start_editor", DescriptorStartEditor},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

}; // namespace Scripting
}; // namespace Scratch
