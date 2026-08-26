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
//! \sa Strings::CapitalizeCopy(const String&)
String& Strings::Capitalize(String& str) {
    auto first = std::find_if_not(
	    str.begin(), str.end(), boost::is_space());
    if (first != str.end())
	*first = static_cast<char>(std::toupper(
		static_cast<unsigned char>(*first)));
    return str;
}

//! Capitalizes the first non-space letter.
//! \param str the string to capitalize
//! \sa Strings::Capitalize(String&)
String Strings::CapitalizeCopy(const String& str) {
    auto strCopy(str);
    return Strings::Capitalize(strCopy);
}

//! Chops the first word from \p str.
//! \param str the string to chop
//! \param remainder the remainder after the first word
//! \return \p str
//! \sa Strings::ChopCopy(const String&, String&)
String& Strings::Chop(
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
//! \sa Strings::Chop(String&, String&)
String Strings::ChopCopy(
	const String& str,
	String& remainder) {
    auto strCopy(str);
    return Strings::Chop(strCopy, remainder);
}

//! Compares strings.
//! \param left the first string to compare
//! \param right the second string to compare
//! \return < 0 if \a left is less than \a right;
//!         > 0 if \a left is greater than \a right;
//!           0 if the specified strings are equal
int Strings::CompareCi(
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
//! \sa Strings::EndsWithCi(const String&, const String&)
bool Strings::EndsWith(
	const String& str,
	const String& suffix) {
    if (suffix.size() > str.size())
	return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

//! Returns whether \p str ends with \p suffix.
//! \param str the string to test
//! \param suffix the suffix
//! \sa Strings::EndsWith(const String&, const String&)
bool Strings::EndsWithCi(
	const String& str,
	const String& suffix) {
    if (suffix.size() > str.size())
	return false;
    return Strings::CompareCi(
	    str.substr(str.size() - suffix.size()), suffix) == 0;
}

//! Hashes a plaintext string.
//! \param plaintext the string to hash
//! \param salt the crypt salt; empty generates a new salt
//! \sa Strings::CryptCopy(const String&, const String&)
String& Strings::Crypt(
	String& plaintext,
	const String& salt) {
    plaintext = Strings::CryptCopy(plaintext, salt);
    return plaintext;
}

//! Hashes a plaintext string.
//! \param plaintext the string to hash
//! \param salt the crypt salt; empty generates a new salt
//! \sa Strings::Crypt(String&, const String&)
String Strings::CryptCopy(
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
	newSalt += Strings::GenerateCopy();
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
//! \sa Strings::Generate(String&)
String& Strings::Generate(
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
//! \sa Strings::Generate(Random&, String&)
String& Strings::Generate(String& str) {
    static Random random;
    return Strings::Generate(random, str);
}

//! Returns a random string.
//! \param random the RNG state
//! \sa Strings::GenerateCopy()
String Strings::GenerateCopy(Random& random) {
    String str;
    return Strings::Generate(random, str);
}

//! Returns a random string.
//! \sa Strings::GenerateCopy(Random&)
String Strings::GenerateCopy() {
    String str;
    return Strings::Generate(str);
}

//! Normalizes whitespace.
//! \param str the string to normalize
//! \sa Strings::NormalizeCopy(const String&)
String& Strings::Normalize(String& str) {
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
//! \sa Strings::Normalize(String&)
String Strings::NormalizeCopy(const String& str) {
    auto strCopy(str);
    return Strings::Normalize(strCopy);
}

//! Sanitizes a string.
//! \param str the string to sanitize
//! \sa Strings::SanitizeCopy(const String&)
String& Strings::Sanitize(String& str) {
    // Lowercase and underscores.
    for (auto& ch : str) {
	ch = std::isalnum((int) ch) ? std::tolower((int) ch) : '_';
    }
    return str;
}

//! Sanitizes a string.
//! \param str the string to sanitize
//! \sa Strings::Sanitize(String&)
String Strings::SanitizeCopy(const String& str) {
    auto strCopy(str);
    return Strings::Sanitize(strCopy);
}

//! Splits text on a delimiter.
//! \param text the text to split
//! \param delimiter the delimiter; \c " " tokenizes on whitespace
//! \return the parts; empty if \a text is empty
//! \sa Strings::SplitLines(const String&)
std::vector<String> Strings::Split(
	const String& text,
	const String& delimiter) {
    std::vector<String> parts;
    if (delimiter.empty()) {
	if (!text.empty())
	    parts.push_back(text);
    } else {
	const auto trimmed = boost::trim_copy(text);
	if (!trimmed.empty()) {
	    if (delimiter == " ")
		boost::split(parts, trimmed, boost::is_space(), boost::token_compress_on);
	    else
		boost::iter_split(parts, trimmed, boost::first_finder(delimiter));
	}
    }
    return parts;
}

//! Splits text into lines.
//! \param text the text to split
//! \return the lines; one empty element if \a text is empty
//! \sa Strings::Split(const String&, const String&)
std::vector<String> Strings::SplitLines(const String& text) {
    std::vector<String> lines;
    String cur;
    for (const char ch : text) {
	if (ch == '\n') {
	    if (!cur.empty() && cur.back() == '\r')
		cur.pop_back();
	    lines.push_back(cur);
	    cur.clear();
	    continue;
	}
	cur.push_back(ch);
    }
    if (!cur.empty() && cur.back() == '\r')
	cur.pop_back();
    if (!cur.empty() || lines.empty())
	lines.push_back(cur);
    return lines;
}

//! Formats a string.
//! \param format the printf-style format specifier
//! \return the formatted string, or empty on error or overflow
//! \sa Strings::FormatVa(const char*, va_list)
String Strings::Format(const char* format, ...) noexcept {
    std::va_list args;
    va_start(args, format);
    const auto result = Strings::FormatVa(format, args);
    va_end(args);
    return result;
}

//! Formats a string.
//! \param format the printf-style format specifier
//! \param args the variadic argument list
//! \return the formatted string, or empty on error or overflow
//! \sa Strings::Format(const char*, ...)
String Strings::FormatVa(
	const char* format,
	std::va_list args) noexcept {
    char buffer[MaxString] = {'\0'};
    std::va_list argsCopy;
    va_copy(argsCopy, args);
    const int n = std::vsnprintf(buffer, sizeof(buffer), format, argsCopy);
    va_end(argsCopy);
    if (n < 0 || static_cast<std::size_t>(n) >= MaxString)
	return String();
    return String(buffer, static_cast<std::size_t>(n));
}

//! Returns whether \p str starts with \p prefix.
//! \param str the string to test
//! \param prefix the prefix
//! \sa Strings::StartsWithCi(const String&, const String&)
bool Strings::StartsWith(
	const String& str,
	const String& prefix) {
    if (prefix.size() > str.size())
	return false;
    return str.compare(0, prefix.size(), prefix) == 0;
}

//! Returns whether \p str starts with \p prefix.
//! \param str the string to test
//! \param prefix the prefix
//! \sa Strings::StartsWith(const String&, const String&)
bool Strings::StartsWithCi(
	const String& str,
	const String& prefix) {
    if (prefix.size() > str.size())
	return false;
    return Strings::CompareCi(str.substr(0, prefix.size()), prefix) == 0;
}

//! Removes color codes from a string.
//! \param str the string to strip
//! \sa Strings::StripCopy(const String&)
String& Strings::Strip(String& str) noexcept {
    return str = Strings::StripCopy(str);
}

//! Removes color codes from a string.
//! \param str the string to strip
//! \sa Strings::Strip(String&)
String Strings::StripCopy(const String& str) noexcept {
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

}; // namespace Algorithm
}; // namespace Scratch
