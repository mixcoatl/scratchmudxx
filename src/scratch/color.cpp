//! \file color.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_COLOR_CPP_

#include <scratch/color.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Net {

//! Returns the color indicated by the specified name.
//! \param name the color name
//! \sa #ToString(ColorEnum)
Color::ColorEnum Color::ByName(const String& name) noexcept {
    static StringMapCi<ColorEnum> colors;
    if (colors.empty()) {
	colors["Amber"] = C_AMBER;
	colors["Aqua"] = C_AQUA;
	colors["Azure"] = C_AZURE;
	colors["Black"] = C_CHARCOAL;
	colors["Blue"] = C_INDIGO;
	colors["Charcoal"] = C_CHARCOAL;
	colors["Crimson"] = C_CRIMSON;
	colors["Cyan"] = C_TEAL;
	colors["Forest"] = C_FOREST;
	colors["Gray"] = C_GRAY;
	colors["Green"] = C_FOREST;
	colors["Indigo"] = C_INDIGO;
	colors["Lime"] = C_LIME;
	colors["Magenta"] = C_PURPLE;
	colors["Normal"] = C_NORMAL;
	colors["Ochre"] = C_OCHRE;
	colors["Pink"] = C_PINK;
	colors["Purple"] = C_PURPLE;
	colors["Red"] = C_CRIMSON;
	colors["Silver"] = C_SILVER;
	colors["Snow"] = C_SNOW;
	colors["Teal"] = C_TEAL;
	colors["Violet"] = C_VIOLET;
	colors["White"] = C_SILVER;
	colors["Yellow"] = C_OCHRE;
    }

    auto const found = colors.find(name);
    if (found != colors.end())
	return found->second;
    return C_UNDEFINED;
}

//! Returns a string representation.
//! \param value the color
//! \sa #ByName(const String&)
String Color::ToString(ColorEnum value) noexcept {
    switch (static_cast<int>(value)) {
    case C_CHARCOAL:	return String("Charcoal");
    case C_CRIMSON:	return String("Crimson");
    case C_FOREST:	return String("Forest");
    case C_OCHRE:	return String("Ochre");
    case C_INDIGO:	return String("Indigo");
    case C_PURPLE:	return String("Purple");
    case C_TEAL:	return String("Teal");
    case C_SILVER:	return String("Silver");
    case C_GRAY:	return String("Gray");
    case C_PINK:	return String("Pink");
    case C_LIME:	return String("Lime");
    case C_AMBER:	return String("Amber");
    case C_AZURE:	return String("Azure");
    case C_VIOLET:	return String("Violet");
    case C_AQUA:	return String("Aqua");
    case C_SNOW:	return String("Snow");
    case C_NORMAL:	return String("Normal");
    default:		return String();
    }
}

}; // namespace Net
}; // namespace Scratch
