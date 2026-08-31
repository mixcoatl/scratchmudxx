//! \file user_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_USER_BINDINGS_CPP_

#include <scratch/gender.hpp>
#include <scratch/lua.hpp>
#include <scratch/repository.hpp>
#include <scratch/scratch.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/string.hpp>
#include <scratch/user.hpp>
#include <scratch/user_bindings.hpp>

namespace Scratch {
namespace Scripting {

//! Metatable name for User userdata.
const char UserBindings::MetaName[] = "Scratch.User";

//! Metatable name for user repository userdata.
const char UserBindings::RepositoryMetaName[] = "Scratch.UserRepository";

//! ScratchMUD types.
using Color = Scratch::Net::Color;
using Gender = Scratch::Core::Gender;
using UserRepositoryPtr = Scratch::Core::UserRepositoryPtr;

//! Sets a User name.
static void UserSetName(
    UserPtr user,
    Game& game,
    String name) {
    auto repo = game.GetUsers();
    if (!repo->IsValidThingId(name))
	throw std::invalid_argument("invalid user id");
    if (repo->Contains(user)) {
	throw std::runtime_error("cannot set_name on a live user");
    }
    user->SetName(name);
}

//! Resolves a User userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the user
UserPtr UserBindings::Check(
	lua_State* L,
	const int index) {
    return Lua::CheckWeakUserdata<User>(
	L, MetaName, "invalid user", index);
}

//! Pushes a User userdata, or nil.
//! \param lua the Lua facade
//! \param user the user to push
void UserBindings::Push(
	Lua& lua,
	UserPtr user) {
    lua.PushUserdata(std::move(user), MetaName);
}

//! Registers User userdata bindings.
//! \param L the \c lua_State
static void RegisterUserMeta(Lua& lua) {
    lua.Class<User>(UserBindings::MetaName).
	Function("add_player", &User::AddPlayer).
	Function("add_preference", &User::AddPreference).
	Function("clear_metacolor", &User::ClearMetaColor).
	Function("erase_player", &User::ErasePlayer).
	Function("erase_preference", &User::ErasePreference).
	Function("get_created", &User::GetCreated).
	Function("get_created_by", &User::GetCreatedBy).
	Function("get_email", &User::GetEmail).
	Function("get_gender", &User::GetGender).
	Function("get_last_login", &User::GetLastLogin).
	Function("get_last_logout", &User::GetLastLogout).
	Function("get_metacolors", &User::GetMetaColors).
	Function("get_modified", &User::GetModified).
	Function("get_modified_by", &User::GetModifiedBy).
	Function("get_name", &User::GetName).
	Function("get_password", &User::GetPassword).
	Function("get_players", &User::GetPlayers).
	Function("get_preferences", &User::GetPreferences).
	Function("has_player", &User::HasPlayer).
	Function("has_preference", &User::HasPreference).
	Function("set_created", &User::SetCreated).
	Function("set_created_by", &User::SetCreatedBy).
	Function("set_email", &User::SetEmail).
	Function("set_gender", &User::SetGender).
	Function("set_metacolor", &User::SetMetaColor).
	Function("set_modified", &User::SetModified).
	Function("set_modified_by", &User::SetModifiedBy).
	Function("set_name", &UserSetName, Injected<Game>()).
	Function("set_password", &User::SetPassword).
	Function("set_preferences", &User::SetPreferences);
}

//! Resolves a UserRepository userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the repository owned by Game
UserRepository& UserBindings::CheckRepository(
	lua_State* L,
	const int index) {
    return *Lua::CheckWeakUserdata<UserRepository>(
	L, RepositoryMetaName, "invalid user repository", index);
}

//! Pushes the user repository userdata.
//! \param lua the Lua facade
void UserBindings::PushRepository(Lua& lua) {
    lua.PushUserdata(lua.GetGame().GetUsers(), RepositoryMetaName);
}

//! Registers User and UserRepository metatables.
//! \param lua the Lua facade
void UserBindings::Register(Lua& lua) {
    RegisterUserMeta(lua);
}

}; // namespace Scripting
}; // namespace Scratch
