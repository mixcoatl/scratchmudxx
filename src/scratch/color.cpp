//! \file color.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_COLOR_CPP_

#include <scratch/color.hpp>
#include <scratch/descriptor.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Net {

//! Constructor.
Color::Color(const ColorEnum value) noexcept:
	value_(value) {
    // Nothing.
}

//! Returns the color indicated by the specified color name.
Color Color::ByName(const String& name) noexcept {
    // Configure color mappings.
    static StringMapCi<Color> colors;
    if (colors.empty()) {
	colors["Black"] = C_BLACK;
	colors["Blue"] = C_BLUE;
	colors["Cyan"] = C_CYAN;
	colors["Green"] = C_GREEN;
	colors["Magenta"] = C_MAGENTA;
	colors["Normal"] = C_NORMAL;
	colors["Red"] = C_RED;
	colors["Yellow"] = C_YELLOW;
	colors["White"] = C_WHITE;
    }

    // Search using color name.
    auto const found = colors.find(name);
    if (found != colors.end())
	return found->second;

    // Color not defined.
    return C_UNDEFINED;
}

//! Returns the color code.
const char* Color::Get(const Descriptor& d) const noexcept {
    if (!d.GetColorBit())
	return "";

    switch (value_) {
    case C_BLACK:	return "\x1b[0;30m";
    case C_RED:		return "\x1b[0;31m";
    case C_GREEN:	return "\x1b[0;32m";
    case C_YELLOW:	return "\x1b[0;33m";
    case C_BLUE:	return "\x1b[0;34m";
    case C_MAGENTA:	return "\x1b[0;35m";
    case C_CYAN:	return "\x1b[0;36m";
    case C_WHITE:	return "\x1b[0;37m";
    case C_NORMAL:	return "\x1b[0m";
    case C_UNDEFINED:	return "";
    case MAX_C_TYPES:	return "";
    default:		return "";
    }
}

//! Removes color codes from a string.
String& Color::Strip(String& str) noexcept {
    return str = StripCopy(str);
}

//! Removes color codes from a string.
String Color::StripCopy(const String& str) noexcept {
    std::ostringstream oss("");
    for (auto ptr = std::begin(str); ptr != std::end(str); ++ptr) {
	if (*ptr == '\x1b') {
	    while (ptr != std::end(str) && !std::isalpha(*ptr))
		++ptr;
	} else {
	    oss << *ptr;
	}
    }
    return oss.str();
}

//! Returns the length of a string without color codes.
std::size_t Color::Strlen(const String& str) noexcept {
    std::size_t len = 0;
    for (auto ptr = std::begin(str); ptr != std::end(str); ++ptr) {
	if (*ptr == '\x1b') {
	    while (ptr != std::end(str) && !std::isalpha(*ptr))
		++ptr;
	} else {
	    len++;
	}
    }
    return len;
}

//! Returns a string representation.
String Color::ToString() const noexcept {
    switch (value_) {
    case C_BLACK:	return String("Black");
    case C_RED:		return String("Red");
    case C_GREEN:	return String("Green");
    case C_YELLOW:	return String("Yellow");
    case C_BLUE:	return String("Blue");
    case C_MAGENTA:	return String("Magenta");
    case C_CYAN:	return String("Cyan");
    case C_WHITE:	return String("White");
    case C_NORMAL:	return String("Normal");
    case C_UNDEFINED:	return String();
    case MAX_C_TYPES:	return String();
    default:		return String();
    }
}

}; // namespace Net
}; // namespace Scratch
