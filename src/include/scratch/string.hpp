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

//! The string utility class. \{
class Strings {
public:
    //! Capitalizes the first non-space letter.
    //! \param str the string to capitalize
    //! \sa #CapitalizeCopy(const String&)
    static String& Capitalize(String& str);

    //! Capitalizes the first non-space letter.
    //! \param str the string to capitalize
    //! \sa #Capitalize(String&)
    static String CapitalizeCopy(const String& str);

    //! Chops the first word from \p str.
    //! \param str the string to chop
    //! \param remainder the remainder after the first word
    //! \return \p str
    //! \sa #ChopCopy(const String&, String&)
    static String& Chop(String& str, String& remainder);

    //! Chops the first word from \p str.
    //! \param str the string to chop
    //! \param remainder the remainder after the first word
    //! \return the first word
    //! \sa #Chop(String&, String&)
    static String ChopCopy(const String& str, String& remainder);

    //! Compares strings.
    //! \param left the first string to compare
    //! \param right the second string to compare
    //! \return < 0 if \a left is less than \a right;
    //!         > 0 if \a left is greater than \a right;
    //!           0 if the specified strings are equal
    static int CompareCi(const String& left, const String& right);

    //! Returns whether \p str ends with \p suffix.
    //! \param str the string to test
    //! \param suffix the suffix
    //! \sa #EndsWithCi(const String&, const String&)
    static bool EndsWith(const String& str, const String& suffix);

    //! Returns whether \p str ends with \p suffix.
    //! \param str the string to test
    //! \param suffix the suffix
    //! \sa #EndsWith(const String&, const String&)
    static bool EndsWithCi(const String& str, const String& suffix);

    //! Hashes a plaintext string.
    //! \param plaintext the string to hash
    //! \param salt the crypt salt; empty generates a new salt
    //! \sa #CryptCopy(const String&, const String&)
    static String& Crypt(
	String& plaintext,
	const String& salt = String());

    //! Hashes a plaintext string.
    //! \param plaintext the string to hash
    //! \param salt the crypt salt; empty generates a new salt
    //! \sa #Crypt(String&, const String&)
    static String CryptCopy(
	const String& plaintext,
	const String& salt = String());

    //! Returns a random string.
    //! \param random the RNG state
    //! \param str the location to store the random string
    //! \sa #Generate(String&)
    static String& Generate(Random& random, String& str);

    //! Returns a random string.
    //! \param str the location to store the random string
    //! \sa #Generate(Random&, String&)
    static String& Generate(String& str);

    //! Returns a random string.
    //! \param random the RNG state
    //! \sa #GenerateCopy()
    static String GenerateCopy(Random& random);

    //! Returns a random string.
    //! \sa #GenerateCopy(Random&)
    static String GenerateCopy();

    //! Normalizes whitespace.
    //! \param str the string to normalize
    //! \sa #NormalizeCopy(const String&)
    static String& Normalize(String& str);

    //! Normalizes whitespace.
    //! \param str the string to normalize
    //! \sa #Normalize(String&)
    static String NormalizeCopy(const String& str);

    //! Sanitizes a string.
    //! \param str the string to sanitize
    //! \sa #SanitizeCopy(const String&)
    static String& Sanitize(String& str);

    //! Sanitizes a string.
    //! \param str the string to sanitize
    //! \sa #Sanitize(String&)
    static String SanitizeCopy(const String& str);

    //! Splits text on a delimiter.
    //! \param text the text to split
    //! \param delimiter the delimiter; \c " " tokenizes on whitespace
    //! \return the parts; empty if \a text is empty
    //! \sa #SplitLines(const String&)
    static std::vector<String> Split(
	const String& text,
	const String& delimiter);

    //! Splits text into lines.
    //! \param text the text to split
    //! \return the lines; one empty element if \a text is empty
    //! \sa #Split(const String&, const String&)
    static std::vector<String> SplitLines(const String& text);

    //! Formats a string.
    //! \param format the printf-style format specifier
    //! \return the formatted string, or empty on error or overflow
    //! \sa #FormatVa(const char*, va_list)
    static String Format(const char* format, ...) noexcept;

    //! Formats a string.
    //! \param format the printf-style format specifier
    //! \param args the variadic argument list
    //! \return the formatted string, or empty on error or overflow
    //! \sa #Format(const char*, ...)
    static String FormatVa(
	const char* format,
	std::va_list args) noexcept;

    //! Returns whether \p str starts with \p prefix.
    //! \param str the string to test
    //! \param prefix the prefix
    //! \sa #StartsWithCi(const String&, const String&)
    static bool StartsWith(const String& str, const String& prefix);

    //! Returns whether \p str starts with \p prefix.
    //! \param str the string to test
    //! \param prefix the prefix
    //! \sa #StartsWith(const String&, const String&)
    static bool StartsWithCi(const String& str, const String& prefix);

    //! Removes color codes from a string.
    //! \param str the string to strip
    //! \sa #StripCopy(const String&)
    static String& Strip(String& str) noexcept;

    //! Removes color codes from a string.
    //! \param str the string to strip
    //! \sa #Strip(String&)
    static String StripCopy(const String& str) noexcept;

private:
    Strings() = delete;
};
//! \}

//! Case-insensitive weak order. \{
struct StringLessCi {
    bool operator()(
	const String& left,
	const String& right) const noexcept {
	return Strings::CompareCi(left, right) < 0;
    }
};
//! \}

}; // namespace Algorithm

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
