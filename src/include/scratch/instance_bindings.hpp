//! \file instance_bindings.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_INSTANCE_BINDINGS_HPP_
#define _SCRATCH_INSTANCE_BINDINGS_HPP_

#include <scratch/game.hpp>
#include <scratch/instance.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Scripting {

using Instance = Scratch::Core::Instance;
using InstancePtr = std::shared_ptr<Instance>;

//! The instance bindings class. \{
class InstanceBindings {
public:
    //! Metatable name for Instance userdata.
    static const char MetaName[];

    //! Registers Instance metatable.
    //! \param lua the Lua facade
    static void Register(Lua& lua);

    //! Resolves an Instance userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the instance
    static InstancePtr Check(
	lua_State* L,
	const int index = 1);

    //! Pushes an Instance userdata, or nil.
    //! \param lua the Lua facade
    //! \param instance the instance to push
    static void Push(
	Lua& lua,
	InstancePtr instance);
};
//! \}

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_INSTANCE_BINDINGS_HPP_
