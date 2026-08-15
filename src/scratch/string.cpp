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

//! Capitalizes the first non-space letter.
//! \param str the string to capitalize
//! \sa StringCapitalizeCopy(const String&)
String& StringCapitalize(String& str) {
    auto first = std::find_if_not(
	    str.begin(), str.end(), boost::is_space());
    if (first != str.end())
	*first = static_cast<char>(std::toupper(
		static_cast<unsigned char>(*first)));
    return str;
}

//! Capitalizes the first non-space letter.
//! \param str the string to capitalize
//! \sa StringCapitalize(String&)
String StringCapitalizeCopy(const String& str) {
    auto strCopy(str);
    return StringCapitalize(strCopy);
}

//! Chops the first word from \p str.
//! \param str the string to chop
//! \param remainder the remainder after the first word
//! \return \p str
//! \sa StringChopCopy(const String&, String&)
String& StringChop(
	String& str,
	String& remainder) {
    boost::trim_left(str);
    const auto pos = static_cast<String::size_type>(
	    std::find_if(str.begin(), str.end(), boost::is_space()) -
	    str.begin());
    remainder = boost::trim_copy(str.substr(pos));
    str = boost::trim_copy(str.substr(0, pos));
    return str;
}

//! Chops the first word from \p str.
//! \param str the string to chop
//! \param remainder the remainder after the first word
//! \return the first word
//! \sa StringChop(String&, String&)
String StringChopCopy(
	const String& str,
	String& remainder) {
    auto strCopy(str);
    return StringChop(strCopy, remainder);
}

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

//! Returns whether \p str ends with \p suffix.
//! \param str the string to test
//! \param suffix the suffix
//! \sa StringEndsWithCi(const String&, const String&)
bool StringEndsWith(
	const String& str,
	const String& suffix) {
    if (suffix.size() > str.size())
	return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

//! Returns whether \p str ends with \p suffix.
//! \param str the string to test
//! \param suffix the suffix
//! \sa StringEndsWith(const String&, const String&)
bool StringEndsWithCi(
	const String& str,
	const String& suffix) {
    if (suffix.size() > str.size())
	return false;
    return StringCompareCi(
	    str.substr(str.size() - suffix.size()), suffix) == 0;
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
//! \sa StringGenerateCopy()
String StringGenerateCopy(Random& random) {
    String str;
    return StringGenerate(random, str);
}

//! Returns a random string.
//! \sa StringGenerateCopy(Random&)
String StringGenerateCopy() {
    String str;
    return StringGenerate(str);
}

//! Joins strings with a separator.
//! \param sep the separator inserted between elements
//! \param parts the strings to join
//! \return the joined string; empty if \a parts is empty
//! \sa StringSplitLines(const String&)
String StringJoin(
	const String& sep,
	const std::vector<String>& parts) {
    if (parts.empty())
	return String();
    std::ostringstream oss;
    oss << parts.front();
    for (std::size_t i = 1; i < parts.size(); ++i)
	oss << sep << parts[i];
    return oss.str();
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

//! Splits text into lines.
//! \param text the text to split
//! \sa StringJoin(const String&, const std::vector<String>&)
std::vector<String> StringSplitLines(const String& text) {
    std::vector<String> lines;
    String cur;
    for (std::size_t i = 0; i < text.size(); ++i) {
	if (text[i] == '\r')
	    continue;
	if (text[i] == '\n') {
	    lines.push_back(cur);
	    cur.clear();
	} else {
	    cur.push_back(text[i]);
	}
    }
    if (!cur.empty() || lines.empty())
	lines.push_back(cur);
    return lines;
}

//! Returns whether \p str starts with \p prefix.
//! \param str the string to test
//! \param prefix the prefix
//! \sa StringStartsWithCi(const String&, const String&)
bool StringStartsWith(
	const String& str,
	const String& prefix) {
    if (prefix.size() > str.size())
	return false;
    return str.compare(0, prefix.size(), prefix) == 0;
}

//! Returns whether \p str starts with \p prefix.
//! \param str the string to test
//! \param prefix the prefix
//! \sa StringStartsWith(const String&, const String&)
bool StringStartsWithCi(
	const String& str,
	const String& prefix) {
    if (prefix.size() > str.size())
	return false;
    return StringCompareCi(str.substr(0, prefix.size()), prefix) == 0;
}

}; // namespace Algorithm
}; // namespace Scratch
