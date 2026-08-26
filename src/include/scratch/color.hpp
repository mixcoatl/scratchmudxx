//! \file color.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_COLOR_HPP_
#define _SCRATCH_COLOR_HPP_

#include <scratch/enum.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Net {

template<typename Derived>
using EnumBase = Scratch::Algorithm::EnumBase<Derived>;

//! The color class. \{
class Color: public EnumBase<Color> {
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
	C_EMPHASIS	= 17,	//!< Color is emphasis.
	C_ENUM		= 18,	//!< Color is enum.
	C_FAILED	= 19,	//!< Color is failed.
	C_KEY		= 20,	//!< Color is key.
	C_NUMBER	= 21,	//!< Color is number.
	C_OKAY		= 22,	//!< Color is okay.
	C_NAME		= 23,	//!< Color is name.
	C_PERCENT	= 24,	//!< Color is percent.
	C_PROMPT	= 25,	//!< Color is prompt.
	C_PUNCTUATION	= 26,	//!< Color is punctuation.
	C_RESTRICTED	= 27,	//!< Color is restricted.
	C_SAY		= 28,	//!< Color is say.
	C_SOCIAL	= 29,	//!< Color is social.
	C_TEXT		= 30,	//!< Color is text.
	C_YESNO		= 31,	//!< Color is yesno.
	C_FIRST_REAL	= C_CHARCOAL, //!< First real color.
	C_FIRST_META	= C_EMPHASIS, //!< First metacolor.
	MAX_C_TYPES	= 32	//!< How many color types.
    };
    //! \}

    //! The enum type.
    using Enum = ColorEnum;

    //! Returns the exclusive upper bound for listing.
    static constexpr int Max() noexcept {
	return MAX_C_TYPES;
    }

    //! Returns the color indicated by the specified name.
    //! \param name the color name
    //! \sa #ToString(ColorEnum)
    static ColorEnum ByName(const String& name) noexcept;

    //! Returns whether the value is a metacolor.
    //! \param value the color
    static constexpr bool IsMetaColor(ColorEnum value) noexcept {
	return value >= C_FIRST_META && value < MAX_C_TYPES;
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
