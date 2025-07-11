//! \file main.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_MAIN_CPP_

#include <scratch/scratch.hpp>

//! Program entry point.
//! \param argc the number of command line arguments
//! \param argv an array containing the command line arguments
//! \return zero for normal program termination; non-zero otherwise
int main(int argc, const char** argv) {
    std::cout << "Starting " << PACKAGE_STRING << '!' << std::endl;
    return EXIT_SUCCESS;
}
