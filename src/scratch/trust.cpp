//! \file trust.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_TRUST_CPP_

#include <scratch/trust.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! Returns whether \p granted meets \p required.
//! \param granted the granted trust
//! \param required the required trust
bool Trust::Allows(
	TrustEnum granted,
	TrustEnum required) noexcept {
    if (!IsDefined(required))
	return true;
    if (!IsDefined(granted))
	return false;
    return Rank(granted) >= Rank(required);
}

//! Returns the trust indicated by the specified name.
//! \param name the trust name
//! \sa #ToString(TrustEnum)
Trust::TrustEnum Trust::ByName(const String& name) noexcept {
    static StringMapCi<TrustEnum> trusts;
    if (trusts.empty()) {
	trusts["Builder"] = TRUST_BUILDER;
	trusts["None"] = TRUST_NONE;
	trusts["Owner"] = TRUST_OWNER;
	trusts["Player"] = TRUST_PLAYER;
	trusts["Wizard"] = TRUST_WIZARD;
    }

    auto const found = trusts.find(name);
    if (found != trusts.end())
	return found->second;
    return static_cast<TrustEnum>(MAX_TRUST_TYPES);
}

//! Returns a string representation.
//! \param value the trust
//! \sa #ByName(const String&)
String Trust::ToString(TrustEnum value) noexcept {
    switch (static_cast<int>(value)) {
    case TRUST_NONE:	return String("None");
    case TRUST_PLAYER:	return String("Player");
    case TRUST_BUILDER:	return String("Builder");
    case TRUST_WIZARD:	return String("Wizard");
    case TRUST_OWNER:	return String("Owner");
    default:		return String();
    }
}

}; // namespace Core
}; // namespace Scratch
