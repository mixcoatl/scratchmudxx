//! \file state_bindings.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_STATE_BINDINGS_HPP_
#define _SCRATCH_STATE_BINDINGS_HPP_

#include <scratch/game.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Scripting {

using State = Scratch::Core::State;
using StatePtr = std::shared_ptr<State>;
using StateRepository = Scratch::Core::StateRepository;

//! The state bindings class. \{
class StateBindings {
public:
    //! Metatable name for State userdata.
    static const char MetaName[];

    //! Metatable name for StateRepository userdata.
    static const char RepositoryMetaName[];

    //! Registers State and StateRepository metatables.
    //! \param lua the Lua facade
    static void Register(Lua& lua);

    //! Resolves a State userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the connection state
    static StatePtr Check(
	lua_State* L,
	const int index = 1);

    //! Pushes a State userdata, or nil.
    //! \param lua the Lua facade
    //! \param state the state to push
    static void Push(
	Lua& lua,
	StatePtr state);

    //! Resolves a StateRepository userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the repository owned by Game
    static StateRepository& CheckRepository(
	lua_State* L,
	const int index = 1);

    //! Pushes the connection-state repository userdata.
    //! \param lua the Lua facade
    static void PushRepository(Lua& lua);
};
//! \}

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_STATE_BINDINGS_HPP_
