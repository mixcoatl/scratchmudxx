//! \file main.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_MAIN_CPP_

#include <scratch/game.hpp>
#include <scratch/logger.hpp>
#include <scratch/scratch.hpp>

//! Program entry point.
//! \param argc the number of command line arguments
//! \param argv an array containing the command line arguments
//! \return zero for normal program termination; non-zero otherwise
int main(int argc, const char** argv) {
    // Startup message.
    LOGGER_MAIN() << "Starting " << PACKAGE_STRING << '!';

    // Configure game.
    auto game = std::make_shared<Scratch::Core::Game>();
    game->ParseArguments(argc, argv);

    // Run game.
    game->Run();

    // Exit.
    LOGGER_MAIN() << "Exiting.";
    return EXIT_SUCCESS;
}
