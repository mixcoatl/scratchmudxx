//! \file preference.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_PREFERENCE_HPP_
#define _SCRATCH_PREFERENCE_HPP_

#include <scratch/enum.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

template<typename Derived>
using EnumBase = Scratch::Algorithm::EnumBase<Derived>;

//! The preference class. \{
class Preference: public EnumBase<Preference> {
public:
    //! The preference enumeration. \{
    enum PreferenceEnum: char {
	PREF_UNDEFINED		= -1,	//!< No preference information.
	PREF_AUTOSAY		= 0,	//!< AutoSay.
	PREF_NOREPEAT		= 1,	//!< NoRepeat.
	MAX_PREF_TYPES		= 2	//!< How many preference types.
    };
    //! \}

    //! The enum type.
    using Enum = PreferenceEnum;

    //! Returns the exclusive upper bound for listing.
    static constexpr int Max() noexcept {
	return MAX_PREF_TYPES;
    }

    //! Returns the preference indicated by the specified name.
    //! \param name the preference name
    //! \sa #ToString(PreferenceEnum)
    static PreferenceEnum ByName(const String& name) noexcept;

    //! Returns whether the value is a defined preference.
    //! \param value the preference
    static constexpr bool IsDefined(PreferenceEnum value) noexcept {
	return value > PREF_UNDEFINED && value < MAX_PREF_TYPES;
    }

    //! Returns a string representation.
    //! \param value the preference
    //! \sa #ByName(const String&)
    static String ToString(PreferenceEnum value) noexcept;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_PREFERENCE_HPP_
