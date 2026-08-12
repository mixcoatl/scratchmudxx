//! \file enumeration_bindings.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_ENUMERATION_BINDINGS_HPP_
#define _SCRATCH_ENUMERATION_BINDINGS_HPP_

#include <scratch/game.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Scripting {

using Enumeration = Scratch::Core::Enumeration;
using EnumerationPtr = std::shared_ptr<Enumeration>;
using EnumerationRepository = Scratch::Core::EnumerationRepository;

//! The enumeration bindings class. \{
class EnumerationBindings {
public:
    //! Metatable name for Enumeration userdata.
    static const char MetaName[];

    //! Metatable name for EnumerationRepository userdata.
    static const char RepositoryMetaName[];

    //! Resolves an Enumeration userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the enumeration
    static EnumerationPtr Check(
	lua_State* L,
	const int index = 1);

    //! Resolves an EnumerationRepository userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the enumeration repository
    static EnumerationRepository& CheckRepository(
	lua_State* L,
	const int index = 1);

    //! Pushes an Enumeration userdata, or nil.
    //! \param lua the Lua facade
    //! \param enumeration the enumeration to push
    static void Push(
	Lua& lua,
	EnumerationPtr enumeration);

    //! Pushes the enumeration repository userdata.
    //! \param lua the Lua facade
    static void PushRepository(Lua& lua);

    //! Registers Enumeration and EnumerationRepository metatables.
    //! \param lua the Lua facade
    static void Register(Lua& lua);
};
//! \}

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_ENUMERATION_BINDINGS_HPP_
