//! \file descriptor_bindings.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_DESCRIPTOR_BINDINGS_HPP_
#define _SCRATCH_DESCRIPTOR_BINDINGS_HPP_

#include <scratch/descriptor.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Scripting {

using Descriptor = Scratch::Net::Descriptor;
using DescriptorPtr = std::shared_ptr<Descriptor>;

//! The descriptor bindings class. \{
class DescriptorBindings {
public:
    //! Metatable name for Descriptor userdata.
    static const char MetaName[];

    //! Registers the Descriptor metatable.
    //! \param lua the Lua facade
    static void Register(Lua& lua);

    //! Resolves a Descriptor userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return a temporary shared pointer; Game remains the owner
    //! \remark Do not hold across \c luaL_error (skips C++ destructors).
    static DescriptorPtr Check(
	lua_State* L,
	const int index = 1);

    //! Pushes a Descriptor userdata, or nil.
    //! \param lua the Lua facade
    //! \param d the descriptor to push
    static void Push(
	Lua& lua,
	DescriptorPtr d);
};
//! \}

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_DESCRIPTOR_BINDINGS_HPP_
