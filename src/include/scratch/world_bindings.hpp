//! \file world_bindings.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_WORLD_BINDINGS_HPP_
    #define _SCRATCH_WORLD_BINDINGS_HPP_

#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>
#include <scratch/world.hpp>

namespace Scratch {
namespace Scripting {

using World = Scratch::Core::World;
using WorldPtr = std::shared_ptr<World>;

//! The World bindings class. \{
class WorldBindings {
public:
    //! Metatable name for World userdata.
    static const char MetaName[];

    //! Registers World metatable.
    //! \param lua the Lua facade
    static void Register(Lua& lua);

    //! Resolves a World userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the world
    static WorldPtr Check(
	lua_State* L,
	const int index = 1);

    //! Pushes a World userdata, or nil.
    //! \param lua the Lua facade
    //! \param world the world to push
    static void Push(
	Lua& lua,
	WorldPtr world);
};
//! \}

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_WORLD_BINDINGS_HPP_
