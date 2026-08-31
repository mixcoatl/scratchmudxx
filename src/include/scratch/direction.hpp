//! \file direction.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_DIRECTION_HPP_
#define _SCRATCH_DIRECTION_HPP_

#include <scratch/enum.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

template<typename Derived>
using EnumBase = Scratch::Algorithm::EnumBase<Derived>;

//! The direction class. \{
class Direction: public EnumBase<Direction> {
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
	MAX_D_TYPES	//!< How many direction types.
    };
    //! \}

    //! The enum type.
    using Enum = DirectionEnum;

    //! Returns the direction for \p name.
    //! \param name the direction name or abbreviation
    static DirectionEnum ByName(const String& name) noexcept;

    //! Returns whether \p value is defined.
    //! \param value the direction
    static constexpr bool IsDefined(DirectionEnum value) noexcept {
	return value >= D_EAST && value < MAX_D_TYPES;
    }

    //! Returns the exclusive upper bound for listing.
    static constexpr int Max() noexcept {
	return MAX_D_TYPES;
    }

    //! Returns the opposite of \p value.
    //! \param value the direction
    static DirectionEnum Opposite(DirectionEnum value) noexcept;

    //! Returns the name of \p value.
    //! \param value the direction
    static String ToString(DirectionEnum value) noexcept;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_DIRECTION_HPP_
