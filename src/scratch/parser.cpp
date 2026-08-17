//! \file parser.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_PARSER_CPP_

#include <scratch/parser.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! Default constructor.
Parser::Phrase::Phrase() noexcept :
	count_(1),
	delimiter_(),
	nth_(1),
	words_() {
    // Nothing.
}

//! Copy constructor.
//! \param other the phrase to copy
Parser::Phrase::Phrase(const Phrase& other) :
	count_(other.count_),
	delimiter_(other.delimiter_),
	nth_(other.nth_),
	words_(other.words_) {
    // Nothing.
}

//! Copy assignment operator.
//! \param other the phrase to assign
Parser::Phrase& Parser::Phrase::operator=(const Phrase& other) {
    count_ = other.count_;
    delimiter_ = other.delimiter_;
    nth_ = other.nth_;
    words_ = other.words_;
    return *this;
}

//! Default constructor.
Parser::Parser() noexcept :
	phrases_() {
    // Nothing.
}

//! Returns the phrase at \p index.
//! \param index the phrase index
//! \return the phrase, or an empty phrase if \p index is out of range
//! \sa #GetSize() const
const Parser::Phrase& Parser::GetPhrase(const std::size_t index) const noexcept {
    if (index >= phrases_.size()) {
	static const Phrase empty;
	return empty;
    }
    return phrases_[index];
}

//! Parses \p line with no delimiters.
//! \param line the targeting line
//! \return \c true if \p line is valid
//! \sa #Parse(const String&, const StringSetCi&)
bool Parser::Parse(const String& line) noexcept {
    return this->Parse(line, StringSetCi());
}

//! Parses \p line into phrases split by \p delimiters.
//! \param line the targeting line
//! \param delimiters caller-supplied function words
//! \return \c true if \p line is valid
//! \sa #Parse(const String&)
bool Parser::Parse(
	const String& line,
	const StringSetCi& delimiters) noexcept {
    auto valid = true;

    // Split on whitespace.
    std::vector<String> tokens;
    const auto trimmed = boost::trim_copy(line);
    if (!trimmed.empty())
	boost::split(tokens, trimmed, boost::is_space(), boost::token_compress_on);

    // Accumulate phrases.
    phrases_.clear();
    if (!tokens.empty()) {
	String delimiter;
	std::vector<String> current;
	for (const auto& token: tokens) {
	    if (delimiters.find(token) != delimiters.end()) {
		Phrase phrase;
		if (current.empty() || !this->ParsePhrase(current, delimiter, phrase)) {
		    valid = false;
		    break;
		}
		phrases_.push_back(phrase);
		current.clear();
		delimiter = token;
	    } else {
		current.push_back(token);
	    }
	}
	if (valid) {
	    Phrase phrase;
	    if (current.empty() || !this->ParsePhrase(current, delimiter, phrase)) {
		valid = false;
	    } else {
		phrases_.push_back(phrase);
	    }
	}
    }
    if (!valid)
	phrases_.clear();

    return valid;
}

//! Fills \p phrase from \p tokens.
//! \param tokens the phrase tokens
//! \param delimiter the delimiter that opened this phrase
//! \param phrase the phrase to fill
//! \return \c false if the tokens are illegal
bool Parser::ParsePhrase(
	const std::vector<String>& tokens,
	const String& delimiter,
	Phrase& phrase) noexcept {
    phrase = Phrase();
    phrase.delimiter_ = delimiter;
    if (tokens.empty())
	return false;

    // Lone token is name word, including numeric token.
    if (tokens.size() == 1) {
	phrase.words_ = tokens;
	return true;
    }

    auto words = tokens;

    // Leading count.
    unsigned value = 0;
    if (boost::conversion::try_lexical_convert(words.front(), value)) {
	if (!value)
	    return false;
	phrase.count_ = value;
	words.erase(words.begin());
    }

    // Trailing nth. Need name word before ordinal.
    if (words.size() >= 2 &&
	    boost::conversion::try_lexical_convert(words.back(), value)) {
	if (!value)
	    return false;
	phrase.nth_ = value;
	words.pop_back();
    }

    if (phrase.count_ != 1 && phrase.nth_ != 1)
	return false;
    if (words.empty())
	return false;

    // Interior all-digit tokens.
    for (const auto& word: words) {
	value = 0;
	if (boost::conversion::try_lexical_convert(word, value))
	    return false;
    }

    phrase.words_ = std::move(words);
    return true;
}

}; // namespace Core
}; // namespace Scratch
