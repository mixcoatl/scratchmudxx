//! \file trust.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_TRUST_HPP_
#define _SCRATCH_TRUST_HPP_

#include <scratch/enum.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

template<typename Derived>
using EnumBase = Scratch::Algorithm::EnumBase<Derived>;

//! The trust class. \{
class Trust: public EnumBase<Trust> {
public:
    //! The trust enumeration. \{
    enum TrustEnum: char {
	TRUST_NONE		= -1,	//!< No elevated role.
	TRUST_PLAYER		= 0,	//!< Normal player.
	TRUST_BUILDER		= 1,	//!< Builder.
	TRUST_WIZARD		= 2,	//!< Wizard.
	TRUST_OWNER		= 3,	//!< Owner.
	MAX_TRUST_TYPES		= 4	//!< How many trust types.
    };
    //! \}

    //! The enum type.
    using Enum = TrustEnum;

    //! Returns the exclusive upper bound for listing.
    static constexpr int Max() noexcept {
	return MAX_TRUST_TYPES;
    }

    //! Returns whether \p granted meets \p required.
    //! \param granted the granted trust
    //! \param required the required trust
    static bool Allows(
	TrustEnum granted,
	TrustEnum required) noexcept;

    //! Returns the trust indicated by the specified name.
    //! \param name the trust name
    //! \sa #ToString(TrustEnum)
    static TrustEnum ByName(const String& name) noexcept;

    //! Returns whether the value is a defined trust.
    //! \param value the trust
    static constexpr bool IsDefined(TrustEnum value) noexcept {
	return value >= TRUST_NONE && value < MAX_TRUST_TYPES;
    }

    //! Returns rank.
    //! \param value the trust
    static constexpr int Rank(TrustEnum value) noexcept {
	return static_cast<int>(value);
    }

    //! Returns a string representation.
    //! \param value the trust
    //! \sa #ByName(const String&)
    static String ToString(TrustEnum value) noexcept;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_TRUST_HPP_
