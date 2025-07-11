//! \file game.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_GAME_CPP_

#include <scratch/game.hpp>
#include <scratch/logger.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Core {

//! Default constructor.
Game::Game() :
	shutdown_(false) {
    // Nothing.
}

//! Destructor.
Game::~Game() noexcept {
    // Nothing.
}

//! Gets the shutdown flag.
//! \sa #SetShutdown(const bool)
bool Game::GetShutdown() const noexcept {
    return shutdown_;
}

//! Parses command line arguments.
//! \param argc the number of command line arguments
//! \param argv an array containing the command line arguments
void Game::ParseArguments(
	const int argc,
	const char **argv) {
    // Nothing.
}

//! Runs the game.
void Game::Run() {
    // Nothing.
}

//! Sets the shutdown flag.
//! \param shutdown the shutdown flag value
//! \sa #GetShutdown() const
void Game::SetShutdown(const bool shutdown) noexcept {
    shutdown_ = shutdown;
}

}; // namespace Core
}; // namespace Scratch
