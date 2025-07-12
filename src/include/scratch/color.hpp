//! \file color.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
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
	C_BLACK		= 0,	//!< Color is black.
	C_RED		= 1,	//!< Color is red.
	C_GREEN		= 2,	//!< Color is green.
	C_YELLOW	= 3,	//!< Color is yellow.
	C_BLUE		= 4,	//!< Color is blue.
	C_MAGENTA	= 5,	//!< Color is magenta.
	C_CYAN		= 6,	//!< Color is cyan.
	C_WHITE		= 7,	//!< Color is white.
	C_NORMAL	= 8,	//!< Color is normal.
	MAX_C_TYPES	= 9	//!< How many color types.
    };
    //! \}

    //! Constructor.
    Color(const ColorEnum value = C_UNDEFINED) noexcept;

    //! Conversion operator.
    operator ColorEnum() const {
	return value_;
    }

    //! Comparison operator.
    constexpr bool operator==(const Color& other) const noexcept {
	return value_ == other.value_;
    }

    //! Comparison operator.
    constexpr bool operator==(const ColorEnum value) const noexcept {
	return value_ == value;
    }

    //! Comparison operator.
    constexpr bool operator!=(const Color& other) const noexcept {
	return value_ != other.value_;
    }

    //! Comparison operator.
    constexpr bool operator!=(const ColorEnum value) const noexcept {
	return value_ != value;
    }

    //! Returns the color indicated by the specified color name.
    static Color ByName(const String& name) noexcept;

    //! Returns the color code.
    const char* Get(const Descriptor& d) const noexcept;

    //! Removes color codes from a string.
    static String& Strip(String& str) noexcept;

    //! Removes color codes from a string.
    static String StripCopy(const String& str) noexcept;

    //! Returns the length of a string without color codes.
    static std::size_t Strlen(const String& str) noexcept;

    //! Returns a string representation.
    String ToString() const noexcept;

private:
    //! The color value.
    //! \sa #operator ColorEnum() const
    //! \sa #operator=(const ColorEnum)
    //! \sa #operator=(const Color&)
    ColorEnum value_;
};
//! \}

}; // namespace Net
}; // namespace Scratch

//! Shortcut color macros. \{
#define Q_BLACK		Scratch::Net::Color(Scratch::Net::Color::C_BLACK).Get(*d)
#define Q_RED		Scratch::Net::Color(Scratch::Net::Color::C_RED).Get(*d)
#define Q_GREEN		Scratch::Net::Color(Scratch::Net::Color::C_GREEN).Get(*d)
#define Q_YELLOW	Scratch::Net::Color(Scratch::Net::Color::C_YELLOW).Get(*d)
#define Q_BLUE		Scratch::Net::Color(Scratch::Net::Color::C_BLUE).Get(*d)
#define Q_MAGENTA	Scratch::Net::Color(Scratch::Net::Color::C_MAGENTA).Get(*d)
#define Q_CYAN		Scratch::Net::Color(Scratch::Net::Color::C_CYAN).Get(*d)
#define Q_WHITE		Scratch::Net::Color(Scratch::Net::Color::C_WHITE).Get(*d)
#define Q_NORMAL	Scratch::Net::Color(Scratch::Net::Color::C_NORMAL).Get(*d)
//! \}

//! Shortcut color macros for `this`. \{
#define QT_BLACK	Scratch::Net::Color(Scratch::Net::Color::C_BLACK).Get(*this)
#define QT_RED		Scratch::Net::Color(Scratch::Net::Color::C_RED).Get(*this)
#define QT_GREEN	Scratch::Net::Color(Scratch::Net::Color::C_GREEN).Get(*this)
#define QT_YELLOW	Scratch::Net::Color(Scratch::Net::Color::C_YELLOW).Get(*this)
#define QT_BLUE		Scratch::Net::Color(Scratch::Net::Color::C_BLUE).Get(*this)
#define QT_MAGENTA	Scratch::Net::Color(Scratch::Net::Color::C_MAGENTA).Get(*this)
#define QT_CYAN		Scratch::Net::Color(Scratch::Net::Color::C_CYAN).Get(*this)
#define QT_WHITE	Scratch::Net::Color(Scratch::Net::Color::C_WHITE).Get(*this)
#define QT_NORMAL	Scratch::Net::Color(Scratch::Net::Color::C_NORMAL).Get(*this)
//! \}

#endif // _SCRATCH_COLOR_HXX_
