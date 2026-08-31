//! \file direction.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_DIRECTION_HPP_
#define _SCRATCH_DIRECTION_HPP_

#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! The direction class. \{
class Direction {
public:
    //! The direction enumeration. \{
    enum DirectionEnum: char {
	D_UNDEFINED = -1,
	D_EAST = 0,
	D_SOUTHEAST,
	D_SOUTH,
	D_SOUTHWEST,
	D_WEST,
	D_NORTHWEST,
	D_NORTH,
	D_NORTHEAST,
	D_UP,
	D_DOWN,
	MAX_DIRECTIONS
    };
    //! \}

    //! Returns the direction indicated by the specified name.
    //! \param name the direction name
    static DirectionEnum ByName(const String& name) noexcept;

    //! Returns whether the value is a defined direction.
    //! \param value the direction
    static constexpr bool IsDefined(DirectionEnum value) noexcept {
	return value >= D_EAST && value < MAX_DIRECTIONS;
    }

    //! Returns a string representation.
    //! \param value the direction
    static String ToString(DirectionEnum value) noexcept;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_DIRECTION_HPP_
