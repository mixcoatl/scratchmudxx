//! \file config.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_CONFIG_HPP_
#define _SCRATCH_CONFIG_HPP_

#include <scratch/scratch.hpp>

namespace Scratch {
namespace Core {

//! The Config class. \{
//! \remark Host settings document. Not a Repository.
class Config {
public:
    //! Default constructor.
    Config() noexcept;

    //! Destructor.
    virtual ~Config() noexcept;

    //! Loads configuration from the fixed Data file.
    //! \return true if the file was loaded successfully
    //! \sa #Save() const
    bool Load() noexcept;

    //! Saves configuration to the fixed Data file.
    //! \return true if the file was written successfully
    //! \sa #Load()
    bool Save() const noexcept;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_CONFIG_HPP_
