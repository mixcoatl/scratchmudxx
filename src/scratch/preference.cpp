//! \file preference.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_PREFERENCE_CPP_

#include <scratch/preference.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! Returns the preference indicated by the specified name.
//! \param name the preference name
//! \sa #ToString(PreferenceEnum)
Preference::PreferenceEnum Preference::ByName(const String& name) noexcept {
    static StringMapCi<PreferenceEnum> preferences;
    if (preferences.empty()) {
	preferences["AutoSay"] = PREF_AUTOSAY;
	preferences["NoRepeat"] = PREF_NOREPEAT;
    }

    auto const found = preferences.find(name);
    if (found != preferences.end())
	return found->second;
    return PREF_UNDEFINED;
}

//! Returns a string representation.
//! \param value the preference
//! \sa #ByName(const String&)
String Preference::ToString(PreferenceEnum value) noexcept {
    switch (static_cast<int>(value)) {
    case PREF_AUTOSAY:	return String("AutoSay");
    case PREF_NOREPEAT:	return String("NoRepeat");
    default:			return String();
    }
}

}; // namespace Core
}; // namespace Scratch
