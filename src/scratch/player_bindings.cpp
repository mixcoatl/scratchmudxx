//! \file player_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_PLAYER_BINDINGS_CPP_

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
#include <scratch/trust.hpp>

namespace Scratch {
namespace Scripting {

//! Metatable name for Player userdata.
const char PlayerBindings::MetaName[] = "Scratch.Player";

//! Metatable name for player repository userdata.
const char PlayerBindings::RepositoryMetaName[] = "Scratch.PlayerRepository";

//! ScratchMUD types.
using Gender = Scratch::Core::Gender;
using PlayerRepositoryPtr = std::shared_ptr<PlayerRepository>;
using Trust = Scratch::Core::Trust;

//! Sets a Player name.
static void PlayerSetName(
    PlayerPtr player,
    Game& game,
    String name) {
    auto repo = game.GetPlayers();
    if (!repo->IsValidThingId(name))
	throw std::invalid_argument("invalid player id");
    if (repo->Contains(player)) {
	throw std::runtime_error("cannot set_name on a live player");
    }
    player->SetName(name);
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
static void RegisterPlayerMeta(Lua& lua) {
    lua.Class<Player>(PlayerBindings::MetaName).
	Function("add_preference", &Player::AddPreference).
	Function("erase_preference", &Player::ErasePreference).
	Function("get_created", &Player::GetCreated).
	Function("get_created_by", &Player::GetCreatedBy).
	Function("get_gender", &Player::GetGender).
	Function("get_modified", &Player::GetModified).
	Function("get_modified_by", &Player::GetModifiedBy).
	Function("get_name", &Player::GetName).
	Function("get_owner", &Player::GetOwner).
	Function("get_preferences", &Player::GetPreferences).
	Function("get_trust", &Player::GetTrust).
	Function("has_preference", &Player::HasPreference).
	Function("set_created", &Player::SetCreated).
	Function("set_created_by", &Player::SetCreatedBy).
	Function("set_gender", &Player::SetGender).
	Function("set_modified", &Player::SetModified).
	Function("set_modified_by", &Player::SetModifiedBy).
	Function("set_name", &PlayerSetName, Injected<Game>()).
	Function("set_owner", &Player::SetOwner).
	Function("set_preferences", &Player::SetPreferences).
	Function("set_trust", &Player::SetTrust);
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

//! Pushes the player repository userdata.
//! \param lua the Lua facade
void PlayerBindings::PushRepository(Lua& lua) {
    lua.PushUserdata(lua.GetGame().GetPlayers(), RepositoryMetaName);
}

//! Registers Player and PlayerRepository metatables.
//! \param lua the Lua facade
void PlayerBindings::Register(Lua& lua) {
    RegisterPlayerMeta(lua);
}

}; // namespace Scripting
}; // namespace Scratch
