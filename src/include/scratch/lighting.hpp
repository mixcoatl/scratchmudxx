//! \file lighting.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_LIGHTING_HPP_
#define _SCRATCH_LIGHTING_HPP_

#include <scratch/enum.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

template<typename Derived>
using EnumBase = Scratch::Algorithm::EnumBase<Derived>;

//! The lighting class. \{
class Lighting: public EnumBase<Lighting> {
public:
    //! The lighting enumeration. \{
    enum LightingEnum: char {
	LIGHTING_UNDEFINED	= -1,	//!< No lighting information.
	LIGHTING_NONE		= 0,	//!< Always dark.
	LIGHTING_NATURAL,		//!< Natural light.
	LIGHTING_AMBIENT,		//!< Lit at night.
	MAX_LIGHTING_TYPES		//!< How many lighting types.
    };
    //! \}

    //! The enum type.
    using Enum = LightingEnum;

    //! Returns the lighting type indicated by the specified name.
    //! \param name the lighting type name
    //! \sa #ToString(LightingEnum)
    static LightingEnum ByName(const String& name) noexcept;

    //! Returns whether the value is a defined lighting type.
    //! \param value the lighting type
    static constexpr bool IsDefined(LightingEnum value) noexcept {
	return value >= LIGHTING_NONE && value < MAX_LIGHTING_TYPES;
    }

    //! Returns the exclusive upper bound for listing.
    static constexpr int Max() noexcept {
	return MAX_LIGHTING_TYPES;
    }

    //! Returns a string representation.
    //! \param value the lighting type
    //! \sa #ByName(const String&)
    static String ToString(LightingEnum value) noexcept;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_LIGHTING_HPP_
