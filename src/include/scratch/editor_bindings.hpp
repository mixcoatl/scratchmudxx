//! \file editor_bindings.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_EDITOR_BINDINGS_HPP_
#define _SCRATCH_EDITOR_BINDINGS_HPP_

#include <scratch/editor.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Scripting {

using Editor = Scratch::Net::Editor;
using EditorPtr = Scratch::Net::EditorPtr;

//! The editor bindings class. \{
class EditorBindings {
public:
    //! Metatable name for Editor userdata.
    static const char MetaName[];

    //! Registers the Editor metatable.
    //! \param lua the Lua facade
    static void Register(Lua& lua);

    //! Resolves an Editor userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the editor
    static EditorPtr Check(
	lua_State* L,
	const int index = 1);

    //! Pushes an Editor userdata, or nil.
    //! \param lua the Lua facade
    //! \param editor the editor to push
    static void Push(
	Lua& lua,
	EditorPtr editor);
};
//! \}

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_EDITOR_BINDINGS_HPP_
