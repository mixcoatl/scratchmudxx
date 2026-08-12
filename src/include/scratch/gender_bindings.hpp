//! \file gender_bindings.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_GENDER_BINDINGS_HPP_
#define _SCRATCH_GENDER_BINDINGS_HPP_

#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Scripting {

//! The gender bindings class. \{
class GenderBindings {
public:
    //! Registers Gender free functions on \p lua.
    //! \param lua the Lua facade
    static void Register(Lua& lua);
};
//! \}

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_GENDER_BINDINGS_HPP_
