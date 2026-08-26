//! \file config_bindings.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_CONFIG_BINDINGS_HPP_
#define _SCRATCH_CONFIG_BINDINGS_HPP_

#include <scratch/lua.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Core {
class Config;
}; // namespace Core
}; // namespace Scratch

namespace Scratch {
namespace Scripting {

using Config = Scratch::Core::Config;
using ConfigPtr = std::shared_ptr<Config>;

//! The config bindings class. \{
class ConfigBindings {
public:
    //! Metatable name for Config userdata.
    static const char MetaName[];

    //! Resolves a Config userdata at \p index.
    //! \param L the \c lua_State
    //! \param index the stack index of the userdata
    //! \return the configuration
    static ConfigPtr Check(
	lua_State* L,
	const int index = 1);

    //! Pushes a Config userdata, or nil.
    //! \param lua the Lua facade
    //! \param config the configuration to push
    static void Push(
	Lua& lua,
	ConfigPtr config);

    //! Registers Config userdata bindings.
    //! \param lua the Lua facade
    static void Register(Lua& lua);
};
//! \}

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_CONFIG_BINDINGS_HPP_
