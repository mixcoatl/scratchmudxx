//! \file direction.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#include <scratch/direction.hpp>

namespace Scratch {
namespace Core {

//! Returns the direction for \p name.
//! \param name the direction name or abbreviation
//! \sa #ToString(DirectionEnum)
Direction::DirectionEnum Direction::ByName(const String& name) noexcept {
    static const String abbrevs[MAX_D_TYPES] = {
	"e", "se", "s", "sw", "w", "nw", "n", "ne", "u", "d"
    };
    for (int i = 0; i < MAX_D_TYPES; ++i) {
	if (!Scratch::Algorithm::Strings::CompareCi(name, abbrevs[i]))
	    return static_cast<DirectionEnum>(i);
    }
    static const String names[MAX_D_TYPES] = {
	"east", "southeast", "south", "southwest", "west",
	"northwest", "north", "northeast", "up", "down"
    };
    for (int i = 0; i < MAX_D_TYPES; ++i) {
	if (!Scratch::Algorithm::Strings::CompareCi(name, names[i]))
	    return static_cast<DirectionEnum>(i);
    }
    return MAX_D_TYPES;
}

//! Returns the opposite of \p value.
//! \param value the direction
//! \sa #ByName(const String&)
Direction::DirectionEnum Direction::Opposite(
	const DirectionEnum value) noexcept {
    static const DirectionEnum opposites[MAX_D_TYPES] = {
	D_WEST, D_NORTHWEST, D_NORTH, D_NORTHEAST, D_EAST,
	D_SOUTHEAST, D_SOUTH, D_SOUTHWEST, D_DOWN, D_UP
    };
    return IsDefined(value) ? opposites[value] : D_UNDEFINED;
}

//! Returns the name of \p value.
//! \param value the direction
//! \sa #ByName(const String&)
String Direction::ToString(const DirectionEnum value) noexcept {
    static const String names[MAX_D_TYPES] = {
	"East", "Southeast", "South", "Southwest", "West",
	"Northwest", "North", "Northeast", "Up", "Down"
    };
    return IsDefined(value) ? names[value] : String();
}

}; // namespace Core
}; // namespace Scratch
