//! \file player_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_PLAYER_BINDINGS_CPP_

#include <scratch/preference.hpp>
#include <scratch/trust.hpp>
#include <scratch/game.hpp>
#include <scratch/gender.hpp>
#include <scratch/lua.hpp>
#include <scratch/player.hpp>
#include <scratch/player_bindings.hpp>
#include <scratch/repository.hpp>
#include <scratch/scratch.hpp>
#include <scratch/storage_file.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Scripting {

//! Metatable name for Player userdata.
const char PlayerBindings::MetaName[] = "Scratch.Player";

//! Metatable name for player repository userdata.
const char PlayerBindings::RepositoryMetaName[] = "Scratch.PlayerRepository";

//! ScratchMUD types.
using Gender = Scratch::Core::Gender;
using PlayerRepositoryPtr = std::shared_ptr<PlayerRepository>;
using Preference = Scratch::Core::Preference;
using Trust = Scratch::Core::Trust;

//! Handles Player userdata garbage collection.
static int PlayerGc(lua_State* L) {
    return Lua::DestroyWeakUserdata<Player>(L, PlayerBindings::MetaName);
}

//! Handles Player:add_preference(preference).
static int PlayerAddPreference(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "add_preference expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto player = PlayerBindings::Check(L, 1);
    const auto name = Lua::CheckString(L, 2);
    if (!Preference::IsDefined(Preference::ByName(name))) {
	player.reset();
	return luaL_argerror(L, 2, "unknown preference");
    }
    player->AddPreference(name);
    return 0;
}

//! Handles Player:erase_preference(preference).
static int PlayerErasePreference(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "erase_preference expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    PlayerBindings::Check(L, 1)->ErasePreference(Lua::CheckString(L, 2));
    return 0;
}

//! Handles Player:get_created().
static int PlayerGetCreated(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto player = PlayerBindings::Check(L, 1);
    const auto created = player->GetCreated();
    player.reset();
    lua.PushInt(static_cast<lua_Integer>(created));
    return 1;
}

//! Handles Player:get_created_by().
static int PlayerGetCreatedBy(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto player = PlayerBindings::Check(L, 1);
    auto createdBy = player->GetCreatedBy();
    player.reset();
    lua.PushString(std::move(createdBy));
    return 1;
}

//! Handles Player:get_gender().
static int PlayerGetGender(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto player = PlayerBindings::Check(L, 1);
    auto gender = Gender::ToString(player->GetGender());
    player.reset();
    lua.PushString(std::move(gender));
    return 1;
}

//! Handles Player:get_modified().
static int PlayerGetModified(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto player = PlayerBindings::Check(L, 1);
    const auto modified = player->GetModified();
    player.reset();
    lua.PushInt(static_cast<lua_Integer>(modified));
    return 1;
}

//! Handles Player:get_modified_by().
static int PlayerGetModifiedBy(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto player = PlayerBindings::Check(L, 1);
    auto modifiedBy = player->GetModifiedBy();
    player.reset();
    lua.PushString(std::move(modifiedBy));
    return 1;
}

//! Handles Player:get_name().
static int PlayerGetName(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto player = PlayerBindings::Check(L, 1);
    auto name = player->GetName();
    player.reset();
    lua.PushString(std::move(name));
    return 1;
}

//! Handles Player:get_owner().
static int PlayerGetOwner(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto player = PlayerBindings::Check(L, 1);
    auto owner = player->GetOwner();
    player.reset();
    lua.PushString(std::move(owner));
    return 1;
}

//! Handles Player:get_preferences().
static int PlayerGetPreferences(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto player = PlayerBindings::Check(L, 1);
    auto preferences = player->GetPreferences();
    player.reset();
    lua.PushStringSet(std::move(preferences));
    return 1;
}

//! Handles Player:get_trust().
static int PlayerGetTrust(lua_State* L) {
    auto& lua = Lua::CheckLua(L);
    auto player = PlayerBindings::Check(L, 1);
    const auto trust = player->GetTrust();
    player.reset();
    lua.PushString(Trust::ToString(trust));
    return 1;
}

//! Handles Player:has_preference(preference).
static int PlayerHasPreference(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "has_preference expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto& lua = Lua::CheckLua(L);
    auto player = PlayerBindings::Check(L, 1);
    const bool present = player->HasPreference(Lua::CheckString(L, 2));
    player.reset();
    lua.PushBool(present);
    return 1;
}

//! Handles Player:set_created(created).
static int PlayerSetCreated(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_created expects 1 argument");
    const auto created = static_cast<std::time_t>(luaL_checkinteger(L, 2));
    auto player = PlayerBindings::Check(L, 1);
    player->SetCreated(created);
    return 0;
}

//! Handles Player:set_created_by(created_by).
static int PlayerSetCreatedBy(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_created_by expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto player = PlayerBindings::Check(L, 1);
    player->SetCreatedBy(Lua::CheckString(L, 2));
    return 0;
}

//! Handles Player:set_gender(gender).
static int PlayerSetGender(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_gender expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto player = PlayerBindings::Check(L, 1);
    const auto name = Lua::CheckString(L, 2);
    if (name.empty()) {
	player->SetGender(Gender::GENDER_UNDEFINED);
	return 0;
    }
    const auto gender = Gender::ByName(name);
    if (!Gender::IsDefined(gender)) {
	player.reset();
	return luaL_argerror(L, 2, "unknown gender");
    }
    player->SetGender(gender);
    return 0;
}

//! Handles Player:set_modified(modified).
static int PlayerSetModified(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_modified expects 1 argument");
    const auto modified = static_cast<std::time_t>(luaL_checkinteger(L, 2));
    auto player = PlayerBindings::Check(L, 1);
    player->SetModified(modified);
    return 0;
}

//! Handles Player:set_modified_by(modified_by).
static int PlayerSetModifiedBy(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_modified_by expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto player = PlayerBindings::Check(L, 1);
    player->SetModifiedBy(Lua::CheckString(L, 2));
    return 0;
}

//! Handles Player:set_name(name).
static int PlayerSetName(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_name expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    const auto name = Lua::CheckString(L, 2);
    auto repo = Lua::CheckGame(L).GetPlayers();
    if (!repo->IsValidThingId(name))
	return luaL_error(L, "invalid player id");
    auto player = PlayerBindings::Check(L, 1);
    if (repo->Contains(player)) {
	repo.reset();
	player.reset();
	return luaL_error(L, "cannot set_name on a live player");
    }
    player->SetName(name);
    return 0;
}

//! Handles Player:set_owner(owner).
static int PlayerSetOwner(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_owner expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto player = PlayerBindings::Check(L, 1);
    player->SetOwner(Lua::CheckString(L, 2));
    return 0;
}

//! Handles Player:set_preferences(table).
static int PlayerSetPreferences(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_preferences expects 1 argument");
    StringSetCi preferences;
    Lua::CheckStringSet(L, preferences, 2);
    for (const auto& name: preferences) {
	if (!Preference::IsDefined(Preference::ByName(name))) {
	    preferences.clear();
	    return luaL_argerror(L, 2, "unknown preference");
	}
    }
    PlayerBindings::Check(L, 1)->SetPreferences(preferences);
    return 0;
}

//! Handles Player:set_trust(trust).
static int PlayerSetTrust(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "set_trust expects 1 argument");
    luaL_checktype(L, 2, LUA_TSTRING);
    auto player = PlayerBindings::Check(L, 1);
    const auto trust = Trust::ByName(Lua::CheckString(L, 2));
    if (!Trust::IsDefined(trust)) {
	player.reset();
	return luaL_argerror(L, 2, "unknown trust");
    }
    player->SetTrust(trust);
    return 0;
}

//! Resolves a Player userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the player
PlayerPtr PlayerBindings::Check(
	lua_State* L,
	const int index) {
    return Lua::CheckWeakUserdata<Player>(
	L, MetaName, "invalid player", index);
}

//! Pushes a Player userdata, or nil.
//! \param lua the Lua facade
//! \param player the player to push
void PlayerBindings::Push(
	Lua& lua,
	PlayerPtr player) {
    lua.PushUserdata(std::move(player), MetaName);
}

//! Registers Player userdata bindings.
//! \param L the \c lua_State
static void RegisterPlayerMeta(lua_State* L) {
    Lua::RegisterMetatable(L, PlayerBindings::MetaName);

    static const luaL_Reg methods[] = {
	{"__gc", PlayerGc},
	{"add_preference", PlayerAddPreference},
	{"erase_preference", PlayerErasePreference},
	{"get_created", PlayerGetCreated},
	{"get_created_by", PlayerGetCreatedBy},
	{"get_gender", PlayerGetGender},
	{"get_modified", PlayerGetModified},
	{"get_modified_by", PlayerGetModifiedBy},
	{"get_name", PlayerGetName},
	{"get_owner", PlayerGetOwner},
	{"get_preferences", PlayerGetPreferences},
	{"get_trust", PlayerGetTrust},
	{"has_preference", PlayerHasPreference},
	{"set_created", PlayerSetCreated},
	{"set_created_by", PlayerSetCreatedBy},
	{"set_gender", PlayerSetGender},
	{"set_modified", PlayerSetModified},
	{"set_modified_by", PlayerSetModifiedBy},
	{"set_name", PlayerSetName},
	{"set_owner", PlayerSetOwner},
	{"set_preferences", PlayerSetPreferences},
	{"set_trust", PlayerSetTrust},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

//! Resolves a PlayerRepository userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the repository owned by Game
PlayerRepository& PlayerBindings::CheckRepository(
	lua_State* L,
	const int index) {
    return *Lua::CheckWeakUserdata<PlayerRepository>(
	L, RepositoryMetaName, "invalid player repository", index);
}

//! Handles PlayerRepository:erase(name).
static int PlayerRepositoryErase(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "erase expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    const bool erased = PlayerBindings::CheckRepository(L).Erase(
	Lua::CheckString(L, 2));
    lua.PushBool(erased);
    return 1;
}

//! Handles PlayerRepository userdata garbage collection.
static int PlayerRepositoryGc(lua_State* L) {
    return Lua::DestroyWeakUserdata<PlayerRepository>(
	L, PlayerBindings::RepositoryMetaName);
}

//! Handles PlayerRepository:get(name).
static int PlayerRepositoryGet(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "get expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    PlayerBindings::Push(lua, PlayerBindings::CheckRepository(L).Get(Lua::CheckString(L, 2)));
    return 1;
}

//! Handles PlayerRepository:get_ids().
static int PlayerRepositoryGetIds(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "get_ids expects no arguments");
    auto& lua = Lua::CheckLua(L);
    lua.PushStringSet(PlayerBindings::CheckRepository(L).GetIds());
    return 1;
}

//! Handles PlayerRepository:load(name).
static int PlayerRepositoryLoad(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "load expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    lua.PushBool(PlayerBindings::CheckRepository(L).Load(Lua::CheckString(L, 2)));
    return 1;
}

//! Handles PlayerRepository:load_index().
static int PlayerRepositoryLoadIndex(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "load_index expects no arguments");
    auto& lua = Lua::CheckLua(L);
    lua.PushBool(PlayerBindings::CheckRepository(L).LoadIndex());
    return 1;
}

//! Handles PlayerRepository:save(name).
static int PlayerRepositorySave(lua_State* L) {
    if (lua_gettop(L) != 2)
	return luaL_error(L, "save expects 1 argument");
    auto& lua = Lua::CheckLua(L);
    const bool saved = PlayerBindings::CheckRepository(L).Save(
	Lua::CheckString(L, 2));
    lua.PushBool(saved);
    return 1;
}

//! Handles PlayerRepository:save_index().
static int PlayerRepositorySaveIndex(lua_State* L) {
    if (lua_gettop(L) != 1)
	return luaL_error(L, "save_index expects no arguments");
    auto& lua = Lua::CheckLua(L);
    lua.PushBool(PlayerBindings::CheckRepository(L).SaveIndex());
    return 1;
}

//! Handles PlayerRepository:store(name, player).
static int PlayerRepositoryStore(lua_State* L) {
    if (lua_gettop(L) != 3)
	return luaL_error(L, "store expects 2 arguments");
    luaL_checktype(L, 2, LUA_TSTRING);
    const auto name = Lua::CheckString(L, 2);
    auto& repo = PlayerBindings::CheckRepository(L);
    if (!repo.IsValidThingId(name))
	return luaL_error(L, "invalid player id");
    auto player = PlayerBindings::Check(L, 3);
    repo.Store(name, player);
    return 0;
}

//! Pushes the player repository userdata.
//! \param lua the Lua facade
void PlayerBindings::PushRepository(Lua& lua) {
    lua.PushUserdata(lua.GetGame().GetPlayers(), RepositoryMetaName);
}

//! Registers player repository userdata bindings.
//! \param L the \c lua_State
static void RegisterPlayerRepositoryMeta(lua_State* L) {
    Lua::RegisterMetatable(L, PlayerBindings::RepositoryMetaName);

    static const luaL_Reg methods[] = {
	{"__gc", PlayerRepositoryGc},
	{"erase", PlayerRepositoryErase},
	{"get", PlayerRepositoryGet},
	{"get_ids", PlayerRepositoryGetIds},
	{"load", PlayerRepositoryLoad},
	{"load_index", PlayerRepositoryLoadIndex},
	{"save", PlayerRepositorySave},
	{"save_index", PlayerRepositorySaveIndex},
	{"store", PlayerRepositoryStore},
	{nullptr, nullptr}
    };
    luaL_setfuncs(L, methods, 0);
    lua_pop(L, 1);
}

//! Registers Player and PlayerRepository metatables.
//! \param lua the Lua facade
void PlayerBindings::Register(Lua& lua) {
    auto* L = lua.GetState();
    RegisterPlayerMeta(L);
    RegisterPlayerRepositoryMeta(L);
}

}; // namespace Scripting
}; // namespace Scratch
