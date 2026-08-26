//! \file gender.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_GENDER_CPP_

#include <scratch/gender.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! Returns the gender indicated by the specified name.
//! \param name the gender name
//! \sa #ToString(GenderEnum)
Gender::GenderEnum Gender::ByName(const String& name) noexcept {
    static StringMapCi<GenderEnum> genders;
    if (genders.empty()) {
	genders["Common"] = GENDER_COMMON;
	genders["Female"] = GENDER_FEMALE;
	genders["Male"] = GENDER_MALE;
	genders["Neuter"] = GENDER_NEUTER;
    }

    auto const found = genders.find(name);
    if (found != genders.end())
	return found->second;
    return GENDER_UNDEFINED;
}

//! Returns the copula.
//! \param value the gender
String Gender::GetCopula(GenderEnum value) noexcept {
    switch (static_cast<int>(value)) {
    case GENDER_COMMON:	return String("are");
    case GENDER_FEMALE:	return String("is");
    case GENDER_MALE:	return String("is");
    case GENDER_NEUTER:	return String("is");
    default:		return String("are");
    }
}

//! Returns the possessive determiner.
//! \param value the gender
String Gender::GetDeterminer(GenderEnum value) noexcept {
    switch (static_cast<int>(value)) {
    case GENDER_COMMON:	return String("their");
    case GENDER_FEMALE:	return String("her");
    case GENDER_MALE:	return String("his");
    case GENDER_NEUTER:	return String("its");
    default:		return String("their");
    }
}

//! Returns the object pronoun.
//! \param value the gender
String Gender::GetObject(GenderEnum value) noexcept {
    switch (static_cast<int>(value)) {
    case GENDER_COMMON:	return String("them");
    case GENDER_FEMALE:	return String("her");
    case GENDER_MALE:	return String("him");
    case GENDER_NEUTER:	return String("it");
    default:		return String("them");
    }
}

//! Returns the possessive pronoun.
//! \param value the gender
String Gender::GetPossessive(GenderEnum value) noexcept {
    switch (static_cast<int>(value)) {
    case GENDER_COMMON:	return String("theirs");
    case GENDER_FEMALE:	return String("hers");
    case GENDER_MALE:	return String("his");
    case GENDER_NEUTER:	return String("its");
    default:		return String("theirs");
    }
}

//! Returns the reflexive pronoun.
//! \param value the gender
String Gender::GetReflexive(GenderEnum value) noexcept {
    switch (static_cast<int>(value)) {
    case GENDER_COMMON:	return String("themselves");
    case GENDER_FEMALE:	return String("herself");
    case GENDER_MALE:	return String("himself");
    case GENDER_NEUTER:	return String("itself");
    default:		return String("themselves");
    }
}

//! Returns the subject pronoun.
//! \param value the gender
String Gender::GetSubject(GenderEnum value) noexcept {
    switch (static_cast<int>(value)) {
    case GENDER_COMMON:	return String("they");
    case GENDER_FEMALE:	return String("she");
    case GENDER_MALE:	return String("he");
    case GENDER_NEUTER:	return String("it");
    default:		return String("they");
    }
}

//! Returns a string representation.
//! \param value the gender
//! \sa #ByName(const String&)
String Gender::ToString(GenderEnum value) noexcept {
    switch (static_cast<int>(value)) {
    case GENDER_COMMON:	return String("Common");
    case GENDER_FEMALE:	return String("Female");
    case GENDER_MALE:	return String("Male");
    case GENDER_NEUTER:	return String("Neuter");
    default:		return String();
    }
}

}; // namespace Core
}; // namespace Scratch
