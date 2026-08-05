//! \file string.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_STRING_HPP_
#define _SCRATCH_STRING_HPP_

#include <scratch/scratch.hpp>

// Forward declarations.
namespace Scratch {
namespace Math {
class Random;
}; // namespace Math
}; // namespace Scratch

namespace Scratch {
namespace Algorithm {

// ScratchMUD types.
using Random = Scratch::Math::Random;

//! Compares strings.
//! \param left the first string to compare
//! \param right the second string to compare
//! \return < 0 if \a left is less than \a right;
//!         > 0 if \a left is greater than \a right;
//!           0 if the specified strings are equal
int StringCompareCi(
	const String& left,
	const String& right);

//! Returns a random string.
//! \param random the RNG state
//! \param str the location to store the random string
//! \sa StringGenerate(String&)
String& StringGenerate(
	Random& random,
	String& str);

//! Returns a random string.
//! \param str the location to store the random string
//! \sa StringGenerate(Random&, String&)
String& StringGenerate(String& str);

//! Returns a random string.
//! \param random the RNG state
//! \sa StringGenerateCopy()
String StringGenerateCopy(Random& random);

//! Returns a random string.
//! \sa StringGenerateCopy(Random&)
String StringGenerateCopy();

//! Case-insensitive weak order. \{
struct StringLessCi {
    bool operator()(
	const String& left,
	const String& right) const noexcept {
	return StringCompareCi(left, right) < 0;
    }
};
//! \}

//! Normalizes whitespace.
//! \param str the string to normalize
//! \sa StringNormalizeCopy(const String&)
String& StringNormalize(String& str);

//! Normalizes whitespace.
//! \param str the string to normalize
//! \sa StringNormalize(String&)
String StringNormalizeCopy(const String& str);

//! Sanitizes a string.
//! \param str the string to sanitize
//! \sa StringSanitizeCopy(const String&)
String& StringSanitize(String& str);

//! Sanitizes a string.
//! \param str the string to sanitize
//! \sa StringSanitize(String&)
String StringSanitizeCopy(const String& str);

//! Removes color codes from a string.
//! \param str the string to strip
//! \sa StringStripCopy(const String&)
String& StringStrip(String& str) noexcept;

//! Removes color codes from a string.
//! \param str the string to strip
//! \sa StringStrip(String&)
String StringStripCopy(const String& str) noexcept;

//! Hashes a plaintext string.
//! \param plaintext the string to hash
//! \param salt the crypt salt; empty generates a new salt
//! \sa StringCryptCopy(const String&, const String&)
String& StringCrypt(
	String& plaintext,
	const String& salt = String());

//! Hashes a plaintext string.
//! \param plaintext the string to hash
//! \param salt the crypt salt; empty generates a new salt
//! \sa StringCrypt(String&, const String&)
String StringCryptCopy(
	const String& plaintext,
	const String& salt = String());

}; // namespace Algorithm
}; // namespace Scratch

// Specialized stdlib types.
namespace Scratch {

//! A \ref std::map specialized for string keys.
//! \tparam ValueT the C++ type of map values
template<class ValueT>
using StringMap = std::map<String, ValueT>;

//! A \ref std::map specialized for case-insensitive string keys.
//! \tparam ValueT the C++ type of map values
template<class ValueT>
using StringMapCi = std::map<String, ValueT, Scratch::Algorithm::StringLessCi>;

//! A \ref std::set specialized for strings.
using StringSet = std::set<String>;

//! A \ref std::set specialized for case-insensitive strings.
using StringSetCi = std::set<String, Scratch::Algorithm::StringLessCi>;

}; // namespace Scratch

#endif // _SCRATCH_STRING_HXX_
