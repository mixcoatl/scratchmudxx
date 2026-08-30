//! \file instance_bindings.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_INSTANCE_BINDINGS_HPP_
    #define _SCRATCH_INSTANCE_BINDINGS_HPP_

#include <scratch/instance.hpp>

namespace Scratch {
namespace Scripting {

class Lua;
using Instance = Scratch::Core::Instance;
using InstancePtr = std::shared_ptr<Instance>;

//! The Instance bindings class. \{
class InstanceBindings {
public:
    //! Metatable name for Instance userdata.
    static const char MetaName[];

    //! Resolves an Instance userdata.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the instance
    static InstancePtr Check(lua_State* L, const int index);

    //! Pushes an Instance userdata.
    //! \param lua the Lua facade
    //! \param instance the instance to push
    static void Push(Lua& lua, InstancePtr instance);

    //! Registers Instance userdata bindings.
    //! \param lua the Lua facade
    static void Register(Lua& lua);
};
//! \}

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_INSTANCE_BINDINGS_HPP_
