//! \file player_bindings.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_PLAYER_BINDINGS_HPP_
#define _SCRATCH_PLAYER_BINDINGS_HPP_

#include <scratch/game.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Scripting {

using Player = Scratch::Core::Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerRepository = Scratch::Core::PlayerRepository;

//! The player bindings class. \{
class PlayerBindings {
public:
    //! Metatable name for Player userdata.
    static const char MetaName[];

    //! Metatable name for PlayerRepository userdata.
    static const char RepositoryMetaName[];

    //! Registers Player and PlayerRepository metatables.
    //! \param lua the Lua facade
    static void Register(Lua& lua);

    //! Resolves a Player userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the player
    static PlayerPtr Check(
	lua_State* L,
	const int index = 1);

    //! Pushes a Player userdata, or nil.
    //! \param lua the Lua facade
    //! \param player the player to push
    static void Push(
	Lua& lua,
	PlayerPtr player);

    //! Resolves a PlayerRepository userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the repository owned by Game
    static PlayerRepository& CheckRepository(
	lua_State* L,
	const int index = 1);

    //! Pushes the player repository userdata.
    //! \param lua the Lua facade
    static void PushRepository(Lua& lua);
};
//! \}

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_PLAYER_BINDINGS_HPP_
