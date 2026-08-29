//! \file parser_bindings.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_PARSER_BINDINGS_HPP_
#define _SCRATCH_PARSER_BINDINGS_HPP_

#include <scratch/lua.hpp>
#include <scratch/parser.hpp>

namespace Scratch {
namespace Scripting {

using Parser = Scratch::Core::Parser;

//! The parser bindings class. \{
class ParserBindings {
public:
    //! Metatable name for Parser::Phrase userdata.
    static const char PhraseMetaName[];

    //! Registers Parser metatables and functions.
    //! \param lua the Lua facade
    static void Register(Lua& lua);
};
//! \}

}; // namespace Scripting
}; // namespace Scratch

#endif // _SCRATCH_PARSER_BINDINGS_HPP_
