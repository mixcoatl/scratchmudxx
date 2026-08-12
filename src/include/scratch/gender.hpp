//! \file gender.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_GENDER_HPP_
#define _SCRATCH_GENDER_HPP_

#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! The gender class. \{
class Gender {
public:
    //! The gender enumeration. \{
    enum GenderEnum: char {
	GENDER_UNDEFINED	= -1,	//!< No gender information.
	GENDER_COMMON		= 0,	//!< Common.
	GENDER_FEMALE		= 1,	//!< Female.
	GENDER_MALE		= 2,	//!< Male.
	GENDER_NEUTER		= 3,	//!< Neuter.
	MAX_GENDER_TYPES	= 4	//!< How many gender types.
    };
    //! \}

    //! Returns the gender indicated by the specified name.
    //! \param name the gender name
    //! \sa #ToString(GenderEnum)
    static GenderEnum ByName(const String& name) noexcept;

    //! Returns the copula.
    //! \param value the gender
    static String GetCopula(GenderEnum value) noexcept;

    //! Returns the possessive determiner.
    //! \param value the gender
    static String GetDeterminer(GenderEnum value) noexcept;

    //! Returns the object pronoun.
    //! \param value the gender
    static String GetObject(GenderEnum value) noexcept;

    //! Returns the possessive pronoun.
    //! \param value the gender
    static String GetPossessive(GenderEnum value) noexcept;

    //! Returns the reflexive pronoun.
    //! \param value the gender
    static String GetReflexive(GenderEnum value) noexcept;

    //! Returns the subject pronoun.
    //! \param value the gender
    static String GetSubject(GenderEnum value) noexcept;

    //! Returns whether the value is a defined gender.
    //! \param value the gender
    static constexpr bool IsDefined(GenderEnum value) noexcept {
	return value > GENDER_UNDEFINED && value < MAX_GENDER_TYPES;
    }

    //! Returns a string representation.
    //! \param value the gender
    //! \sa #ByName(const String&)
    static String ToString(GenderEnum value) noexcept;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_GENDER_HPP_
