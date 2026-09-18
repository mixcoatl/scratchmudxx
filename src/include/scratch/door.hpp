//! \file door.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_DOOR_HPP_
#define _SCRATCH_DOOR_HPP_

#include <scratch/enum.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

template<typename Derived>
using EnumBase = Scratch::Algorithm::EnumBase<Derived>;

//! The door class. \{
class Door: public EnumBase<Door> {
public:
    //! The door enumeration. \{
    enum DoorEnum: char {
	DOOR_UNDEFINED	= -1,	//!< No door state.
	DOOR_NONE	= 0,	//!< Not a door.
	DOOR_OPEN,			//!< Door open.
	DOOR_CLOSED,			//!< Door closed.
	DOOR_LOCKED,			//!< Door closed and locked.
	DOOR_BROKEN,			//!< Broken open.
	MAX_DOOR_STATES			//!< How many door states.
    };
    //! \}

    //! The enum type.
    using Enum = DoorEnum;

    //! Returns the door state indicated by the specified name.
    //! \param name the door state name
    //! \sa #ToString(DoorEnum)
    static DoorEnum ByName(const String& name) noexcept;

    //! Returns whether the value blocks passage.
    //! \param value the door state
    static constexpr bool BlocksPassage(DoorEnum value) noexcept {
	return value == DOOR_CLOSED || value == DOOR_LOCKED;
    }

    //! Returns whether the value is a defined door state.
    //! \param value the door state
    static constexpr bool IsDefined(DoorEnum value) noexcept {
	return value >= DOOR_NONE && value < MAX_DOOR_STATES;
    }

    //! Returns the exclusive upper bound for listing.
    static constexpr int Max() noexcept {
	return MAX_DOOR_STATES;
    }

    //! Returns a string representation.
    //! \param value the door state
    //! \sa #ByName(const String&)
    static String ToString(DoorEnum value) noexcept;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_DOOR_HPP_
