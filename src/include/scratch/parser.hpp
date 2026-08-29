//! \file parser.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_PARSER_HPP_
#define _SCRATCH_PARSER_HPP_

#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

// ScratchMUD types.
using StringSetCi = Scratch::StringSetCi;

//! The parser class. \{
class Parser {
public:
    //! The phrase class. \{
    class Phrase {
    public:
	//! Default constructor.
	Phrase() noexcept;

	//! Copy constructor.
	//! \param other the phrase to copy
	Phrase(const Phrase& other);

	//! Move constructor.
	//! \param other the phrase to move
	Phrase(Phrase&& other) noexcept = default;

	//! Copy assignment operator.
	//! \param other the phrase to assign
	Phrase& operator=(const Phrase& other) = default;

	//! Move assignment operator.
	//! \param other the phrase to move
	Phrase& operator=(Phrase&& other) noexcept = default;

	//! Gets the requested count.
	unsigned GetCount() const noexcept {
	    return count_;
	}

	//! Returns whether this is a single phrase.
	bool IsSingle() const noexcept {
	    return count_ == 1 && nth_ == 1;
	}

	//! Gets the delimiter that opened this phrase.
	String GetDelimiter() const noexcept {
	    return delimiter_;
	}

	//! Gets the 1-based ordinal.
	unsigned GetNth() const noexcept {
	    return nth_;
	}

	//! Gets the name words.
	std::vector<String> GetWords() const noexcept {
	    return words_;
	}

    protected:
	friend class Parser;

	//! Requested count.
	//! \sa #GetCount() const
	unsigned count_;

	//! Delimiter that opened this phrase.
	//! \sa #GetDelimiter() const
	String delimiter_;

	//! 1-based ordinal.
	//! \sa #GetNth() const
	unsigned nth_;

	//! Name words.
	//! \sa #GetWords() const
	std::vector<String> words_;
    };
    //! \}

    //! Default constructor.
    Parser() noexcept;

    //! Returns the phrase at \p index.
    //! \param index the phrase index
    //! \return the phrase
    //! \exception std::out_of_range if \p index is out of range
    //! \sa #GetSize() const
    Phrase GetPhrase(const std::size_t index) const;

    //! Returns the phrase opened by \p delimiter.
    //! \param delimiter the opening delimiter
    //! \param ordinal the zero-based matching phrase ordinal
    //! \return the phrase, or an empty phrase if no phrase matches
    //! \sa #GetPhrase(const std::size_t) const
    Phrase GetPhraseByDelimiter(
	const String& delimiter,
	const std::size_t ordinal = 0) const;

    //! Returns the number of phrases.
    //! \sa #GetPhrase(const std::size_t) const
    std::size_t GetSize() const noexcept {
	return phrases_.size();
    }

    //! Parses \p line into phrases split by \p delimiters.
    //! \param line the targeting line
    //! \param delimiters caller-supplied function words, or an empty set
    //! \return \c true if \p line is valid
    bool Parse(
	const String& line,
	const StringSetCi& delimiters = StringSetCi()) noexcept;

protected:
    //! Parses one phrase from \p tokens.
    //! \param tokens the remaining input tokens
    //! \param delimiters caller-supplied function words
    //! \param phrase the phrase to fill
    //! \return \c true if a phrase was successfully parsed
    static bool ParsePhrase(
	std::vector<String>& tokens,
	const StringSetCi& delimiters,
	Phrase& phrase) noexcept;

    //! Parsed phrases.
    //! \sa #GetPhrase(const std::size_t) const
    //! \sa #GetSize() const
    std::vector<Phrase> phrases_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_PARSER_HPP_
