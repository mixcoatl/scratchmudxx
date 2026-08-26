//! \file editor_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_EDITOR_BINDINGS_CPP_

#include <scratch/editor.hpp>
#include <scratch/editor_bindings.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Scripting {

//! Metatable name for Editor userdata.
const char EditorBindings::MetaName[] = "Scratch.Editor";


//! Handles Editor userdata garbage collection.
static int EditorGc(lua_State* L) {
    return Lua::DestroyWeakUserdata<Editor>(
	L, EditorBindings::MetaName);
}

//! Handles Editor:get_length().
static int EditorGetLength(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto editor = EditorBindings::Check(L, 1);
    const auto length = editor->GetLength();
    editor.reset();
    lua.PushInt(static_cast<lua_Integer>(length));
    return 1;
}

//! Handles Editor:get_max_length().
static int EditorGetMaxLength(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto editor = EditorBindings::Check(L, 1);
    const auto maxLength = editor->GetMaxLength();
    editor.reset();
    lua.PushInt(static_cast<lua_Integer>(maxLength));
    return 1;
}

//! Handles Editor:get_tag().
static int EditorGetTag(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto editor = EditorBindings::Check(L, 1);
    auto tag = editor->GetTag();
    editor.reset();
    lua.PushString(std::move(tag));
    return 1;
}

//! Handles Editor:get_text().
static int EditorGetText(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto editor = EditorBindings::Check(L, 1);
    auto text = editor->GetText();
    editor.reset();
    lua.PushString(std::move(text));
    return 1;
}

//! Handles Editor:is_aborted().
static int EditorIsAborted(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto editor = EditorBindings::Check(L, 1);
    const auto aborted = editor->IsAborted();
    editor.reset();
    lua.PushBool(aborted);
    return 1;
}

//! Handles Editor:is_saved().
static int EditorIsSaved(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto editor = EditorBindings::Check(L, 1);
    const auto saved = editor->IsSaved();
    editor.reset();
    lua.PushBool(saved);
    return 1;
}

//! Resolves an Editor userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the editor
EditorPtr EditorBindings::Check(
	lua_State* L,
	const int index) {
    return Lua::CheckWeakUserdata<Editor>(
	L, MetaName, "invalid editor", index);
}

//! Pushes an Editor userdata, or nil.
//! \param lua the Lua facade
//! \param editor the editor to push
void EditorBindings::Push(
	Lua& lua,
	EditorPtr editor) {
    lua.PushUserdata(std::move(editor), MetaName);
}

//! Registers the Editor metatable.
//! \param L the \c lua_State
static void RegisterEditorMeta(lua_State* L) {
    Lua::RegisterMetatable(L, EditorBindings::MetaName);

    static const luaL_Reg methods[] = {
	{"__gc", EditorGc},
	{"get_length", EditorGetLength},
	{"get_max_length", EditorGetMaxLength},
	{"get_tag", EditorGetTag},
	{"get_text", EditorGetText},
	{"is_aborted", EditorIsAborted},
	{"is_saved", EditorIsSaved},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

//! Registers the Editor metatable.
//! \param lua the Lua facade
void EditorBindings::Register(Lua& lua) {
    RegisterEditorMeta(lua.GetState());
}

}; // namespace Scripting
}; // namespace Scratch
