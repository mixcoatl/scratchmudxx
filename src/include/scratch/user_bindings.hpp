//! \file user_bindings.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_USER_BINDINGS_HPP_
#define _SCRATCH_USER_BINDINGS_HPP_

#include <scratch/game.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Scripting {

using User = Scratch::Core::User;
using UserPtr = std::shared_ptr<User>;
using UserRepository = Scratch::Core::UserRepository;

//! The user bindings class. \{
class UserBindings {
public:
    //! Metatable name for User userdata.
    static const char MetaName[];

    //! Metatable name for UserRepository userdata.
    static const char RepositoryMetaName[];

    //! Registers User and UserRepository metatables.
    //! \param lua the Lua facade
    static void Register(Lua& lua);

    //! Resolves a User userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the user
    static UserPtr Check(
	lua_State* L,
	const int index = 1);

    //! Pushes a User userdata, or nil.
    //! \param lua the Lua facade
    //! \param user the user to push
    static void Push(
	Lua& lua,
	UserPtr user);

    //! Resolves a UserRepository userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the repository owned by Game
    static UserRepository& CheckRepository(
	lua_State* L,
	const int index = 1);

    //! Pushes the user repository userdata.
    //! \param lua the Lua facade
    static void PushRepository(Lua& lua);
};
//! \}

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_USER_BINDINGS_HPP_
