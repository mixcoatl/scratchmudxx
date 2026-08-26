//! \file color_bindings.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_COLOR_BINDINGS_HPP_
#define _SCRATCH_COLOR_BINDINGS_HPP_

#include <scratch/descriptor.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Scripting {

using Descriptor = Scratch::Net::Descriptor;

//! The color bindings class. \{
class ColorBindings {
public:
    //! Assigns \c Q from \p d.
    //! \param lua the Lua facade
    //! \param d the descriptor
    static void AssignQ(
	Lua& lua,
	const Descriptor& d);

    //! Builds the color-code map for \c Q.
    //! \param d the descriptor whose palette to export
    //! \remark ToString names; legacy ANSI dim aliases for Lua \c Q only.
    static StringMapCi<String> Codes(const Descriptor& d);

    //! Refills \c Q when \p desc is the current \c d.
    //! \param L the \c lua_State
    //! \param lua the Lua facade
    //! \param desc the descriptor
    static void RefillGlobalQ(
	lua_State* L,
	Lua& lua,
	const Descriptor& desc);

};
//! \}

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_COLOR_BINDINGS_HPP_
