//! \file door.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_DOOR_CPP_

#include <scratch/door.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! Returns the door state indicated by the specified name.
//! \param name the door state name
//! \sa #ToString(DoorEnum)
Door::DoorEnum Door::ByName(const String& name) noexcept {
    static StringMapCi<DoorEnum> doorStates;
    if (doorStates.empty()) {
	doorStates["Broken"] = DOOR_BROKEN;
	doorStates["Closed"] = DOOR_CLOSED;
	doorStates["Locked"] = DOOR_LOCKED;
	doorStates["None"] = DOOR_NONE;
	doorStates["Open"] = DOOR_OPEN;
    }

    auto const found = doorStates.find(name);
    if (found != doorStates.end())
	return found->second;
    return DOOR_UNDEFINED;
}

//! Returns a string representation.
//! \param value the door state
//! \sa #ByName(const String&)
String Door::ToString(DoorEnum value) noexcept {
    switch (static_cast<int>(value)) {
    case DOOR_BROKEN:	return String("Broken");
    case DOOR_CLOSED:	return String("Closed");
    case DOOR_LOCKED:	return String("Locked");
    case DOOR_NONE:	return String("None");
    case DOOR_OPEN:	return String("Open");
    default:		return String();
    }
}

}; // namespace Core
}; // namespace Scratch
