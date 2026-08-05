//! \file color.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_COLOR_HPP_
#define _SCRATCH_COLOR_HPP_

#include <scratch/scratch.hpp>

// Forward declarations.
namespace Scratch {
namespace Net {
class Descriptor;
}; // namespace Net
}; // namespace Scratch


namespace Scratch {
namespace Net {

// ScratchMUD types.
using Descriptor = Scratch::Net::Descriptor;

//! The color class. \{
class Color {
public:
    //! The color enumeration. \{
    enum ColorEnum: char {
	C_UNDEFINED	= -1,	//!< No color information.
	C_CHARCOAL	= 0,	//!< Dim black.
	C_CRIMSON	= 1,	//!< Dim red.
	C_FOREST	= 2,	//!< Dim green.
	C_OCHRE		= 3,	//!< Dim yellow.
	C_INDIGO	= 4,	//!< Dim blue.
	C_PURPLE	= 5,	//!< Dim magenta.
	C_TEAL		= 6,	//!< Dim cyan.
	C_SILVER	= 7,	//!< Dim white.
	C_GRAY		= 8,	//!< Bright black.
	C_PINK		= 9,	//!< Bright red.
	C_LIME		= 10,	//!< Bright green.
	C_AMBER		= 11,	//!< Bright yellow.
	C_AZURE		= 12,	//!< Bright blue.
	C_VIOLET	= 13,	//!< Bright magenta.
	C_AQUA		= 14,	//!< Bright cyan.
	C_SNOW		= 15,	//!< Bright white.
	C_NORMAL	= 16,	//!< Color is normal.
	C_FIRST_REAL	= C_CHARCOAL, //!< First real color.
	MAX_C_TYPES	= 17	//!< How many color types.
    };
    //! \}

    //! Returns the color indicated by the specified name.
    //! \param name the color name
    //! \sa #ToString(ColorEnum)
    static ColorEnum ByName(const String& name) noexcept;

    //! Returns whether the value is a real color.
    //! \param value the color
    static constexpr bool IsReal(ColorEnum value) noexcept {
	return value >= C_FIRST_REAL && value <= C_NORMAL;
    }

    //! Returns a string representation.
    //! \param value the color
    //! \sa #ByName(const String&)
    static String ToString(ColorEnum value) noexcept;
};
//! \}

}; // namespace Net
}; // namespace Scratch

#endif // _SCRATCH_COLOR_HPP_
