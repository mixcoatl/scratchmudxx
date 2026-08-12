//! \file state_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_STATE_BINDINGS_CPP_

#include <scratch/game.hpp>
#include <scratch/lua.hpp>
#include <scratch/repository.hpp>
#include <scratch/scratch.hpp>
#include <scratch/state.hpp>
#include <scratch/state_bindings.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Scripting {

//! Metatable name for connection State userdata.
const char StateBindings::MetaName[] = "Scratch.State";

//! Metatable name for connection-state repository userdata.
const char StateBindings::RepositoryMetaName[] = "Scratch.StateRepository";

//! ScratchMUD types.
using StateRepositoryPtr = Scratch::Core::StateRepositoryPtr;
using WeakStatePtr = std::weak_ptr<State>;
using WeakStateRepositoryPtr = std::weak_ptr<StateRepository>;

static WeakStateRepositoryPtr CheckWeakStateRepositoryPtr(
	lua_State* L,
	const int index = 1);

//! Returns a copy of the weak State handle stored in userdata.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
static WeakStatePtr CheckWeakStatePtr(
	lua_State* L,
	const int index) {
    return *static_cast<WeakStatePtr*>(
	luaL_checkudata(L, index, StateBindings::MetaName));
}

//! Handles State userdata garbage collection.
static int StateGc(lua_State* L) {
    static_cast<WeakStatePtr*>(
	luaL_checkudata(L, 1, StateBindings::MetaName))->~WeakStatePtr();
    return 0;
}

//! Handles State:get_created().
static int StateGetCreated(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto state = StateBindings::Check(L, 1);
    const auto created = state->GetCreated();
    state.reset();
    lua.PushInt(static_cast<lua_Integer>(created));
    return 1;
}

//! Handles State:get_created_by().
static int StateGetCreatedBy(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto state = StateBindings::Check(L, 1);
    auto createdBy = state->GetCreatedBy();
    state.reset();
    lua.PushString(std::move(createdBy));
    return 1;
}

//! Handles State:get_focus_hook().
static int StateGetFocusHook(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto state = StateBindings::Check(L, 1);
    auto focus = state->GetFocus();
    state.reset();
    lua.PushString(std::move(focus));
    return 1;
}

//! Handles State:get_focus_lost_hook().
static int StateGetFocusLostHook(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto state = StateBindings::Check(L, 1);
    auto focusLost = state->GetFocusLost();
    state.reset();
    lua.PushString(std::move(focusLost));
    return 1;
}

//! Handles State:get_modified().
static int StateGetModified(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto state = StateBindings::Check(L, 1);
    const auto modified = state->GetModified();
    state.reset();
    lua.PushInt(static_cast<lua_Integer>(modified));
    return 1;
}

//! Handles State:get_modified_by().
static int StateGetModifiedBy(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto state = StateBindings::Check(L, 1);
    auto modifiedBy = state->GetModifiedBy();
    state.reset();
    lua.PushString(std::move(modifiedBy));
    return 1;
}

//! Handles State:get_name().
static int StateGetName(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto state = StateBindings::Check(L, 1);
    auto name = state->GetName();
    state.reset();
    lua.PushString(std::move(name));
    return 1;
}

//! Handles State:get_received_hook().
static int StateGetReceivedHook(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto state = StateBindings::Check(L, 1);
    auto received = state->GetReceived();
    state.reset();
    lua.PushString(std::move(received));
    return 1;
}

//! Handles State:is_prompt().
static int StateIsPrompt(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto state = StateBindings::Check(L, 1);
    const bool prompt = state->GetPromptBit();
    state.reset();
    lua.PushBool(prompt);
    return 1;
}

//! Handles State:is_quiet().
static int StateIsQuiet(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto state = StateBindings::Check(L, 1);
    const bool quiet = state->GetQuietBit();
    state.reset();
    lua.PushBool(quiet);
    return 1;
}

//! Handles State:set_created(created).
static int StateSetCreated(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_created expects 1 argument");
    const auto created = static_cast<std::time_t>(luaL_checkinteger(L, 2));
    auto state = StateBindings::Check(L, 1);
    state->SetCreated(created);
    return 0;
}

//! Handles State:set_created_by(created_by).
static int StateSetCreatedBy(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_created_by expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto state = StateBindings::Check(L, 1);
    state->SetCreatedBy(Lua::CheckString(L, 2));
    return 0;
}

//! Handles State:set_focus_hook(focus).
static int StateSetFocusHook(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_focus_hook expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto state = StateBindings::Check(L, 1);
    state->SetFocus(Lua::CheckString(L, 2));
    return 0;
}

//! Handles State:set_focus_lost_hook(focus_lost).
static int StateSetFocusLostHook(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_focus_lost_hook expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto state = StateBindings::Check(L, 1);
    state->SetFocusLost(Lua::CheckString(L, 2));
    return 0;
}

//! Handles State:set_modified(modified).
static int StateSetModified(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_modified expects 1 argument");
    const auto modified = static_cast<std::time_t>(luaL_checkinteger(L, 2));
    auto state = StateBindings::Check(L, 1);
    state->SetModified(modified);
    return 0;
}

//! Handles State:set_modified_by(modified_by).
static int StateSetModifiedBy(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_modified_by expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto state = StateBindings::Check(L, 1);
    state->SetModifiedBy(Lua::CheckString(L, 2));
    return 0;
}

//! Handles State:set_name(name).
static int StateSetName(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_name expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto repo = Lua::CheckGame(L).GetStates();
    auto state = StateBindings::Check(L, 1);
    if (repo->Contains(state)) {
	state.reset();
	return luaL_error(L, "cannot set_name on a live connection state");
    }
    state->SetName(Lua::CheckString(L, 2));
    return 0;
}

//! Handles State:set_prompt(bool).
static int StateSetPrompt(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_prompt expects 1 argument");
    const bool prompt = lua_toboolean(L, 2) != 0;
    auto& game = Lua::CheckGame(L);
    auto state = StateBindings::Check(L, 1);
    state->SetPromptBit(prompt);
    game.ApplyStateBits(state);
    return 0;
}

//! Handles State:set_quiet(bool).
static int StateSetQuiet(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_quiet expects 1 argument");
    const bool quiet = lua_toboolean(L, 2) != 0;
    auto& game = Lua::CheckGame(L);
    auto state = StateBindings::Check(L, 1);
    state->SetQuietBit(quiet);
    game.ApplyStateBits(state);
    return 0;
}

//! Handles State:set_received_hook(received).
static int StateSetReceivedHook(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_received_hook expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto state = StateBindings::Check(L, 1);
    state->SetReceived(Lua::CheckString(L, 2));
    return 0;
}

//! Resolves a State userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the connection state
StatePtr StateBindings::Check(
	lua_State* L,
	const int index) {
    StatePtr state = CheckWeakStatePtr(L, index).lock();
    if (!state)
	luaL_argerror(L, index, "invalid state");
    return state;
}

//! Pushes a State userdata, or nil.
//! \param lua the Lua facade
//! \param state the state to push
void StateBindings::Push(
	Lua& lua,
	StatePtr state) {
    lua.PushUserdata(std::move(state), MetaName);
}

//! Registers State userdata bindings.
//! \param L the \c lua_State
static void RegisterStateMeta(lua_State* L) {
    luaL_newmetatable(L, StateBindings::MetaName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    static const luaL_Reg methods[] = {
	{"__gc", StateGc},
	{"get_created", StateGetCreated},
	{"get_created_by", StateGetCreatedBy},
	{"get_focus_hook", StateGetFocusHook},
	{"get_focus_lost_hook", StateGetFocusLostHook},
	{"get_modified", StateGetModified},
	{"get_modified_by", StateGetModifiedBy},
	{"get_name", StateGetName},
	{"get_received_hook", StateGetReceivedHook},
	{"is_prompt", StateIsPrompt},
	{"is_quiet", StateIsQuiet},
	{"set_created", StateSetCreated},
	{"set_created_by", StateSetCreatedBy},
	{"set_focus_hook", StateSetFocusHook},
	{"set_focus_lost_hook", StateSetFocusLostHook},
	{"set_modified", StateSetModified},
	{"set_modified_by", StateSetModifiedBy},
	{"set_name", StateSetName},
	{"set_prompt", StateSetPrompt},
	{"set_quiet", StateSetQuiet},
	{"set_received_hook", StateSetReceivedHook},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

//! Resolves a StateRepository userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the repository owned by Game
StateRepository& StateBindings::CheckRepository(
	lua_State* L,
	const int index) {
    StateRepositoryPtr repo = CheckWeakStateRepositoryPtr(L, index).lock();
    if (!repo)
	luaL_argerror(L, index, "invalid state repository");
    return *repo;
}

//! Returns a copy of the weak StateRepository handle stored in userdata.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
static WeakStateRepositoryPtr CheckWeakStateRepositoryPtr(
	lua_State* L,
	const int index) {
    return *static_cast<WeakStateRepositoryPtr*>(
	luaL_checkudata(L, index, StateBindings::RepositoryMetaName));
}

//! Handles StateRepository:clear().
static int StateRepositoryClear(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "clear expects no arguments");
    StateBindings::CheckRepository(L).Clear();
    return 0;
}

//! Handles StateRepository:erase(name).
static int StateRepositoryErase(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "erase expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    const bool erased = StateBindings::CheckRepository(L).Erase(
	Lua::CheckString(L, 2));
    lua.PushBool(erased);
    return 1;
}

//! Handles StateRepository userdata garbage collection.
static int StateRepositoryGc(lua_State* L) {
    static_cast<WeakStateRepositoryPtr*>(
	luaL_checkudata(L, 1, StateBindings::RepositoryMetaName))->~WeakStateRepositoryPtr();
    return 0;
}

//! Handles StateRepository:get(name).
static int StateRepositoryGet(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "get expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    StateBindings::Push(lua, StateBindings::CheckRepository(L).Get(Lua::CheckString(L, 2)));
    return 1;
}

//! Handles StateRepository:get_ids().
static int StateRepositoryGetIds(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_ids expects no arguments");
    auto& lua = Lua::CheckLua(L);
    lua.PushStringSet(StateBindings::CheckRepository(L).GetIds());
    return 1;
}

//! Handles StateRepository:load(name).
static int StateRepositoryLoad(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "load expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    auto& game = Lua::CheckGame(L);
    auto& repo = StateBindings::CheckRepository(L);
    const auto name = Lua::CheckString(L, 2);
    const bool loaded = repo.Load(name);
    if (loaded)
	game.ApplyStateBits(repo.Get(name));
    lua.PushBool(loaded);
    return 1;
}

//! Handles StateRepository:load_index().
static int StateRepositoryLoadIndex(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "load_index expects no arguments");
    auto& lua = Lua::CheckLua(L);
    auto& game = Lua::CheckGame(L);
    auto& repo = StateBindings::CheckRepository(L);
    const bool loaded = repo.LoadIndex();
    if (loaded) {
	for (const auto& id: repo.GetIds())
	    game.ApplyStateBits(repo.Get(id));
    }
    lua.PushBool(loaded);
    return 1;
}

//! Handles StateRepository:save(name).
static int StateRepositorySave(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "save expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    const bool saved = StateBindings::CheckRepository(L).Save(
	Lua::CheckString(L, 2));
    lua.PushBool(saved);
    return 1;
}

//! Handles StateRepository:save_index().
static int StateRepositorySaveIndex(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "save_index expects no arguments");
    auto& lua = Lua::CheckLua(L);
    lua.PushBool(StateBindings::CheckRepository(L).SaveIndex());
    return 1;
}

//! Handles StateRepository:store(name, state).
static int StateRepositoryStore(lua_State* L) {
    if (lua_gettop(L) != 3)
	return luaL_error(L, "store expects 2 arguments");
    luaL_checktype(L, 2, LUA_TSTRING);
    const auto name = Lua::CheckString(L, 2);
    if (Scratch::Algorithm::StringCompareCi(
	    Scratch::Algorithm::StringSanitizeCopy(name), name))
	return luaL_error(L, "invalid state id");
    auto& game = Lua::CheckGame(L);
    auto& repo = StateBindings::CheckRepository(L);
    auto state = StateBindings::Check(L, 3);
    repo.Store(name, state);
    game.ApplyStateBits(repo.Get(name));
    return 0;
}

//! Pushes the connection-state repository userdata.
//! \param lua the Lua facade
void StateBindings::PushRepository(Lua& lua) {
    lua.PushUserdata(lua.GetGame().GetStates(), RepositoryMetaName);
}

//! Registers connection-state repository userdata bindings.
//! \param L the \c lua_State
static void RegisterStateRepositoryMeta(lua_State* L) {
    luaL_newmetatable(L, StateBindings::RepositoryMetaName);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    static const luaL_Reg methods[] = {
	{"__gc", StateRepositoryGc},
	{"clear", StateRepositoryClear},
	{"erase", StateRepositoryErase},
	{"get", StateRepositoryGet},
	{"get_ids", StateRepositoryGetIds},
	{"load", StateRepositoryLoad},
	{"load_index", StateRepositoryLoadIndex},
	{"save", StateRepositorySave},
	{"save_index", StateRepositorySaveIndex},
	{"store", StateRepositoryStore},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

//! Registers State and StateRepository metatables.
//! \param lua the Lua facade
void StateBindings::Register(Lua& lua) {
    auto* L = lua.GetState();
    RegisterStateMeta(L);
    RegisterStateRepositoryMeta(L);
}

}; // namespace Scripting
}; // namespace Scratch
