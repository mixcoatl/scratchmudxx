//! \file string.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
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

}; // namespace Algorithm
}; // namespace Scratch
