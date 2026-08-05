//! \file string.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_STRING_CPP_

#include <scratch/random.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Algorithm {

//! Compares strings.
//! \param left the first string to compare
//! \param right the second string to compare
//! \return < 0 if \a left is less than \a right;
//!         > 0 if \a left is greater than \a right;
//!           0 if the specified strings are equal
int StringCompareCi(
	const String& left,
	const String& right) {
    // Comparison result.
    auto cmp = 0;

    // Search for first non-same character.
    auto leftIt = std::begin(left), leftEnd = std::end(left);
    auto rightIt = std::begin(right), rightEnd = std::end(right);
    for (; !cmp && leftIt != leftEnd && rightIt != rightEnd; ++leftIt, ++rightIt) {
	cmp = std::tolower(*leftIt) - std::tolower(*rightIt);
    }

    // Return if non-same character found.
    if (cmp)
	return cmp;

    // Left string shorter.
    if (leftIt == leftEnd && rightIt != rightEnd)
	return -1;

    // Right string shorter.
    if (leftIt != leftEnd && rightIt == rightEnd)
	return +1;

    // Equal strings.
    return 0;
}

//! Returns a random string.
//! \param random the RNG state
//! \param str the location to store the random string
//! \sa StringGenerate(String&)
String& StringGenerate(
	Random& random,
	String& str) {
    // Generate random value.
    auto value = random.Next();

    // Convert to base-36.
    std::basic_ostringstream<typename String::value_type> oss;
    do {
	const int digit = value % 36;
	oss << (String::value_type)(digit < 10 ? '0' + digit : 'a' + digit - 10);
    } while (value /= 36);

    // Reverse characters.
    str = oss.str();
    std::reverse(std::begin(str), std::end(str));

    return str;
}

//! Returns a random string.
//! \param str the location to store the random string
//! \sa StringGenerate(Random&, String&)
String& StringGenerate(String& str) {
    static Random random;
    return StringGenerate(random, str);
}

//! Returns a random string.
//! \param random the RNG state
//! \sa StringGenerateCopy(String&)
String StringGenerateCopy(Random& random) {
    String str;
    return StringGenerate(random, str);
}

//! Returns a random string.
//! \sa StringGenerateCopy(String&)
String StringGenerateCopy() {
    String str;
    return StringGenerate(str);
}

//! Normalizes whitespace.
//! \param str the string to normalize
//! \sa StringNormalizeCopy(const String&)
String& StringNormalize(String& str) {
    // Output stream.
    std::basic_ostringstream<typename String::value_type> oss;
    auto found = false;

    // Read words.
    String word;
    std::basic_istringstream<typename String::value_type> iss(str);
    while (iss >> word) {
	oss << (found ? " " : "") << word;
	found = true;
    }
    return str = oss.str();
}

//! Normalizes whitespace.
//! \param str the string to normalize
//! \sa StringNormalize(String&)
String StringNormalizeCopy(const String& str) {
    auto strCopy(str);
    return StringNormalize(strCopy);
}

//! Sanitizes a string.
//! \param str the string to sanitize
//! \sa StringSanitizeCopy(const String&)
String& StringSanitize(String& str) {
    // Lowercase and underscores.
    for (auto& ch : str) {
	ch = std::isalnum((int) ch) ? std::tolower((int) ch) : '_';
    }
    return str;
}

//! Sanitizes a string.
//! \param str the string to sanitize
//! \sa StringSanitize(String&)
String StringSanitizeCopy(const String& str) {
    auto strCopy(str);
    return StringSanitize(strCopy);
}

//! Removes color codes from a string.
//! \param str the string to strip
//! \sa StringStripCopy(const String&)
String& StringStrip(String& str) noexcept {
    return str = StringStripCopy(str);
}

//! Removes color codes from a string.
//! \param str the string to strip
//! \sa StringStrip(String&)
String StringStripCopy(const String& str) noexcept {
    std::ostringstream oss("");
    for (auto ptr = std::begin(str); ptr != std::end(str); ++ptr) {
	if (*ptr == '\x1b') {
	    while (ptr != std::end(str) && !std::isalpha(*ptr))
		++ptr;
	} else {
	    oss << *ptr;
	}
    }
    return oss.str();
}

//! Hashes a plaintext string.
//! \param plaintext the string to hash
//! \param salt the crypt salt; empty generates a new salt
//! \sa StringCryptCopy(const String&, const String&)
String& StringCrypt(
	String& plaintext,
	const String& salt) {
    plaintext = StringCryptCopy(plaintext, salt);
    return plaintext;
}

//! Hashes a plaintext string.
//! \param plaintext the string to hash
//! \param salt the crypt salt; empty generates a new salt
//! \sa StringCrypt(String&, const String&)
String StringCryptCopy(
	const String& plaintext,
	const String& salt) {
#ifdef HAVE_LIBCRYPT
    if (!salt.empty()) {
	const char* hashed = crypt(plaintext.c_str(), salt.c_str());
	if (hashed && *hashed)
	    return hashed;
	return plaintext;
    }

    // 1) Large salt → modern ($6$ SHA-512). 2) Truncate → DES. 3) Plaintext.
    String newSalt;
    while (newSalt.size() < 16)
	newSalt += StringGenerateCopy();
    newSalt.resize(16);

    const String shaSalt = "$6$" + newSalt + "$";
    const char* hashed = crypt(plaintext.c_str(), shaSalt.c_str());
    if (hashed && *hashed)
	return hashed;

    newSalt.resize(2);
    hashed = crypt(plaintext.c_str(), newSalt.c_str());
    if (hashed && *hashed)
	return hashed;
#endif // HAVE_LIBCRYPT

    return plaintext;
}

}; // namespace Algorithm
}; // namespace Scratch
