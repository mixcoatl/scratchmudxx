//! \file game.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_GAME_HPP_
#define _SCRATCH_GAME_HPP_

#include <scratch/scratch.hpp>

namespace Scratch {
namespace Core {

//! The game class. \{
class Game {
public:
    //! Default constructor.
    Game();

    //! Destructor.
    ~Game() noexcept;

    //! Gets the shutdown flag.
    //! \sa #SetShutdown(const bool)
    bool GetShutdown() const noexcept;

    //! Parses command line arguments.
    //! \param argc the number of command line arguments
    //! \param argv an array containing the command line arguments
    void ParseArguments(
	const int argc,
	const char **argv);

    //! Runs the game.
    virtual void Run();

    //! Sets the shutdown flag.
    //! \param shutdown the shutdown flag value
    //! \sa #GetShutdown() const
    void SetShutdown(const bool shutdown) noexcept;

protected:
    //! The shutdown flag.
    //! \sa #GetShutdown() const
    //! \sa #SetShutdown(const bool)
    bool shutdown_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_GAME_HXX_
