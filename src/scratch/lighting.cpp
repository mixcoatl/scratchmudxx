//! \file lighting.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_LIGHTING_CPP_

#include <scratch/lighting.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! Returns the lighting type indicated by the specified name.
//! \param name the lighting type name
//! \sa #ToString(LightingEnum)
Lighting::LightingEnum Lighting::ByName(const String& name) noexcept {
    static StringMapCi<LightingEnum> lightingTypes;
    if (lightingTypes.empty()) {
	lightingTypes["Ambient"] = LIGHTING_AMBIENT;
	lightingTypes["Natural"] = LIGHTING_NATURAL;
	lightingTypes["None"] = LIGHTING_NONE;
    }

    auto const found = lightingTypes.find(name);
    if (found != lightingTypes.end())
	return found->second;
    return LIGHTING_UNDEFINED;
}

//! Returns a string representation.
//! \param value the lighting type
//! \sa #ByName(const String&)
String Lighting::ToString(LightingEnum value) noexcept {
    switch (static_cast<int>(value)) {
    case LIGHTING_AMBIENT:	return String("Ambient");
    case LIGHTING_NATURAL:	return String("Natural");
    case LIGHTING_NONE:		return String("None");
    default:			return String();
    }
}

}; // namespace Core
}; // namespace Scratch
