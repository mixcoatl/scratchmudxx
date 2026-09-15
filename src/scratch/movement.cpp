//! \file movement.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_MOVEMENT_CPP_

#include <scratch/movement.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! Returns the movement type indicated by the specified name.
//! \param name the movement type name
//! \sa #ToString(MovementEnum)
Movement::MovementEnum Movement::ByName(const String& name) noexcept {
    static StringMapCi<MovementEnum> movementTypes;
    if (movementTypes.empty()) {
	movementTypes["Boat"] = MOVEMENT_BOAT;
	movementTypes["Fly"] = MOVEMENT_FLY;
	movementTypes["None"] = MOVEMENT_NONE;
	movementTypes["Swim"] = MOVEMENT_SWIM;
    }

    auto const found = movementTypes.find(name);
    if (found != movementTypes.end())
	return found->second;
    return MOVEMENT_UNDEFINED;
}

//! Returns a string representation.
//! \param value the movement type
//! \sa #ByName(const String&)
String Movement::ToString(MovementEnum value) noexcept {
    switch (static_cast<int>(value)) {
    case MOVEMENT_BOAT:	return String("Boat");
    case MOVEMENT_FLY:	return String("Fly");
    case MOVEMENT_NONE:	return String("None");
    case MOVEMENT_SWIM:	return String("Swim");
    default:		return String();
    }
}

}; // namespace Core
}; // namespace Scratch
