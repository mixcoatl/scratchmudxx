//! \file config.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_CONFIG_CPP_

#include <scratch/config.hpp>
#include <scratch/data.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Core {

// ScratchMUD types.
using Data = Scratch::Utility::Data;

//! Fixed path of the host configuration Data file.
static const char configFileName[] = "data/config.dat";

//! Default constructor.
Config::Config() noexcept {
    // Nothing.
}

//! Destructor.
Config::~Config() noexcept {
    // Nothing.
}

//! Loads configuration from the fixed Data file.
//! \return true if the file was loaded successfully
//! \sa #Save() const
bool Config::Load() noexcept {
    auto root = std::make_shared<Data>();
    if (!root->LoadFile(configFileName))
	return false;
    return root->GetEntries().empty();
}

//! Saves configuration to the fixed Data file.
//! \return true if the file was written successfully
//! \sa #Load()
bool Config::Save() const noexcept {
    auto root = std::make_shared<Data>();
    return root->SaveFile(configFileName);
}

}; // namespace Core
}; // namespace Scratch
