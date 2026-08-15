//! \file command_bindings.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_COMMAND_BINDINGS_HPP_
#define _SCRATCH_COMMAND_BINDINGS_HPP_

#include <scratch/game.hpp>
#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Scripting {

using Command = Scratch::Core::Command;
using CommandPtr = std::shared_ptr<Command>;
using CommandRepository = Scratch::Core::CommandRepository;
using Social = Scratch::Core::Social;
using SocialPtr = std::shared_ptr<Social>;

//! The command bindings class. \{
class CommandBindings {
public:
    //! Metatable name for Command userdata.
    static const char MetaName[];

    //! Metatable name for CommandRepository userdata.
    static const char RepositoryMetaName[];

    //! Metatable name for Social userdata.
    static const char SocialMetaName[];

    //! Resolves a Command userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the command
    static CommandPtr Check(
	lua_State* L,
	const int index = 1);

    //! Resolves a CommandRepository userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the repository owned by Game
    static CommandRepository& CheckRepository(
	lua_State* L,
	const int index = 1);

    //! Resolves a Social userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the social
    static SocialPtr CheckSocial(
	lua_State* L,
	const int index = 1);

    //! Pushes a Command userdata, or nil.
    //! \param lua the Lua facade
    //! \param command the command to push
    static void Push(
	Lua& lua,
	CommandPtr command);

    //! Pushes the command repository userdata.
    //! \param lua the Lua facade
    static void PushRepository(Lua& lua);

    //! Pushes a Social userdata, or nil.
    //! \param lua the Lua facade
    //! \param social the social to push
    static void PushSocial(
	Lua& lua,
	SocialPtr social);

    //! Registers Command and Social bindings.
    //! \param lua the Lua facade
    static void Register(Lua& lua);
};
//! \}

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_COMMAND_BINDINGS_HPP_
