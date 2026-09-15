//! \file movement.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_MOVEMENT_HPP_
#define _SCRATCH_MOVEMENT_HPP_

#include <scratch/enum.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

template<typename Derived>
using EnumBase = Scratch::Algorithm::EnumBase<Derived>;

//! The movement class. \{
class Movement: public EnumBase<Movement> {
public:
    //! The movement enumeration. \{
    enum MovementEnum: char {
	MOVEMENT_UNDEFINED	= -1,	//!< No movement information.
	MOVEMENT_NONE		= 0,	//!< Normal movement.
	MOVEMENT_BOAT,			//!< Requires a boat.
	MOVEMENT_FLY,			//!< Requires flight.
	MOVEMENT_SWIM,			//!< Requires swimming.
	MAX_MOVEMENT_TYPES		= 4	//!< How many movement types.
    };
    //! \}

    //! The enum type.
    using Enum = MovementEnum;

    //! Returns the movement type indicated by the specified name.
    //! \param name the movement type name
    //! \sa #ToString(MovementEnum)
    static MovementEnum ByName(const String& name) noexcept;

    //! Returns whether the value is a defined movement type.
    //! \param value the movement type
    static constexpr bool IsDefined(MovementEnum value) noexcept {
	return value >= MOVEMENT_NONE && value < MAX_MOVEMENT_TYPES;
    }

    //! Returns the exclusive upper bound for listing.
    static constexpr int Max() noexcept {
	return MAX_MOVEMENT_TYPES;
    }

    //! Returns a string representation.
    //! \param value the movement type
    //! \sa #ByName(const String&)
    static String ToString(MovementEnum value) noexcept;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_MOVEMENT_HPP_
