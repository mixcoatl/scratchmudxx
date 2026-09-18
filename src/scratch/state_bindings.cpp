//! \file state_bindings.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_STATE_BINDINGS_CPP_

#include <scratch/config.hpp>
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

//! Sets a State name.
static void StateSetName(
    StatePtr state,
    Game& game,
    String name) {
    auto repo = game.GetStates();
    if (!repo->IsValidThingId(name))
	throw std::invalid_argument("invalid state id");
    if (repo->Contains(state)) {
	throw std::runtime_error("cannot set_name on a live connection state");
    }
    state->SetName(name);
}

//! Resolves a StateRepository userdata at \p index.
//! \param L the \c lua_State
//! \param index the stack index of the userdata
//! \return the repository owned by Game
StateRepository& StateBindings::CheckRepository(
	lua_State* L,
	const int index) {
    return *Lua::CheckWeakUserdata<StateRepository>(
	L, RepositoryMetaName, "invalid state repository", index);
}

//! Handles StateRepository:save(name).
//! Pushes the connection-state repository userdata.
//! \param lua the Lua facade
void StateBindings::PushRepository(Lua& lua) {
    lua.PushUserdata(lua.GetGame().GetStates(), RepositoryMetaName);
}

//! Registers connection-state repository userdata bindings.
//! \param L the \c lua_State
//! Registers State and StateRepository metatables.
//! \param lua the Lua facade
void StateBindings::Register(Lua& lua) {
    lua.Class<State>(MetaName).
	Function("get_focus_hook", &State::GetFocus).
	Function("get_focus_lost_hook", &State::GetFocusLost).
	Function("get_received_hook", &State::GetReceived).
	Function("is_prompt", &State::GetPromptBit).
	Function("is_quiet", &State::GetQuietBit).
	Function("set_focus_hook", &State::SetFocus).
	Function("set_focus_lost_hook", &State::SetFocusLost).
	Function("set_received_hook", &State::SetReceived).
	Function("get_created", &State::GetCreated).
	Function("get_created_by", &State::GetCreatedBy).
	Function("get_modified", &State::GetModified).
	Function("get_modified_by", &State::GetModifiedBy).
	Function("get_name", &State::GetName).
	Function("set_created", &State::SetCreated).
	Function("set_created_by", &State::SetCreatedBy).
	Function("set_modified", &State::SetModified).
	Function("set_modified_by", &State::SetModifiedBy).
	Function("set_name", &StateSetName, Injected<Game>()).
	Function("set_prompt", &State::SetPromptBit).
	Function("set_quiet", &State::SetQuietBit);
    lua.Class<StateRepository>(RepositoryMetaName).
	Function("get", &StateRepository::Get).
	Function("get_ids", &StateRepository::GetIds).
	Function("load", &StateRepository::Load).
	Function("load_index", &StateRepository::LoadIndex).
	Function("save", &StateRepository::Save).
	Function("save_index", &StateRepository::SaveIndex).
	Function("store", &StateRepository::Store);
}

}; // namespace Scripting
}; // namespace Scratch
