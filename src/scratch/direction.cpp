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

Direction::DirectionEnum Direction::ByName(const String& name) noexcept {
    static const String names[MAX_DIRECTIONS] = {
	"east", "southeast", "south", "southwest", "west",
	"northwest", "north", "northeast", "up", "down"
    };
    for (int i = 0; i < MAX_DIRECTIONS; ++i) {
	if (!Scratch::Algorithm::Strings::CompareCi(name, names[i]))
	    return static_cast<DirectionEnum>(i);
    }
    return MAX_DIRECTIONS;
}

String Direction::ToString(const DirectionEnum value) noexcept {
    static const String names[MAX_DIRECTIONS] = {
	"East", "Southeast", "South", "Southwest", "West",
	"Northwest", "North", "Northeast", "Up", "Down"
    };
    return IsDefined(value) ? names[value] : String();
}

}; // namespace Core
}; // namespace Scratch
