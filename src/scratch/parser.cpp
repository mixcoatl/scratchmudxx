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

// ScratchMUD types.
using Strings = Scratch::Algorithm::Strings;

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
Parser::Phrase::Phrase(const Phrase& other) = default;

//! Default constructor.
Parser::Parser() noexcept :
	phrases_() {
    // Nothing.
}

//! Returns the phrase at \p index.
//! \param index the phrase index
//! \return the phrase, or an empty phrase if \p index is out of range
//! \sa #GetSize() const
Parser::Phrase Parser::GetPhrase(const std::size_t index) const {
    return phrases_.at(index);
}

//! Returns the phrase opened by \p delimiter.
//! \param delimiter the opening delimiter
//! \param ordinal the zero-based matching phrase ordinal
//! \return the phrase, or an empty phrase if no phrase matches
//! \sa #GetPhrase(const std::size_t) const
Parser::Phrase Parser::GetPhraseByDelimiter(
	const String& delimiter,
	const std::size_t ordinal) const {
    std::size_t found = 0;
    for (const auto& phrase: phrases_) {
	if (!Strings::CompareCi(phrase.GetDelimiter(), delimiter)) {
	    if (found++ == ordinal)
		return phrase;
	}
    }
    return Phrase();
}

//! Parses \p line into phrases split by \p delimiters.
//! \param line the targeting line
//! \param delimiters caller-supplied function words
//! \return \c true if \p line is valid
bool Parser::Parse(
	const String& line,
	const StringSetCi& delimiters) noexcept {
    auto valid = true;

    // Whitespace tokens.
    auto tokens = Strings::Split(line, " ");

    // Phrase accumulation.
    phrases_.clear();
    while (!tokens.empty() && valid) {
	Phrase phrase;
	if ((valid = this->ParsePhrase(tokens, delimiters, phrase)))
	    phrases_.push_back(std::move(phrase));
    }
    if (!valid)
	phrases_.clear();

    return valid;
}

//! Parses one phrase from \p tokens.
//! \param tokens the remaining input tokens
//! \param delimiters caller-supplied function words
//! \param phrase the phrase to fill
//! \return \c true if a phrase was successfully parsed
bool Parser::ParsePhrase(
	std::vector<String>& tokens,
	const StringSetCi& delimiters,
	Phrase& phrase) noexcept {
    phrase = Phrase();
    if (delimiters.find(tokens.front()) != delimiters.end()) {
	phrase.delimiter_ = tokens.front();
	tokens.erase(tokens.begin());
    }
    std::vector<String> phraseTokens;
    while (!tokens.empty() &&
	    delimiters.find(tokens.front()) == delimiters.end()) {
	phraseTokens.push_back(tokens.front());
	tokens.erase(tokens.begin());
    }
    if (phraseTokens.empty())
	return phrase.delimiter_.size() || phrase.words_.size();

    // Lone token is name word, including numeric token.
    if (phraseTokens.size() == 1) {
	phrase.words_ = std::move(phraseTokens);
	return true;
    }

    auto words = std::move(phraseTokens);

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
