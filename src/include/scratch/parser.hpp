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

	//! Copy assignment operator.
	//! \param other the phrase to assign
	Phrase& operator=(const Phrase& other);

	//! Gets the requested count.
	//! \sa #count_
	unsigned GetCount() const noexcept {
	    return count_;
	}

	//! Gets the delimiter that opened this phrase.
	//! \sa #delimiter_
	String GetDelimiter() const noexcept {
	    return delimiter_;
	}

	//! Gets the 1-based ordinal.
	//! \sa #nth_
	unsigned GetNth() const noexcept {
	    return nth_;
	}

	//! Gets the name words.
	//! \sa #words_
	const std::vector<String>& GetWords() const noexcept {
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
    //! \return the phrase, or an empty phrase if \p index is out of range
    //! \sa #GetSize() const
    const Phrase& GetPhrase(const std::size_t index) const noexcept;

    //! Returns the number of phrases.
    //! \sa #GetPhrase(const std::size_t) const
    std::size_t GetSize() const noexcept {
	return phrases_.size();
    }

    //! Parses \p line with no delimiters.
    //! \param line the targeting line
    //! \return \c true if \p line is valid
    //! \sa #Parse(const String&, const StringSetCi&)
    bool Parse(const String& line) noexcept;

    //! Parses \p line into phrases split by \p delimiters.
    //! \param line the targeting line
    //! \param delimiters caller-supplied function words
    //! \return \c true if \p line is valid
    //! \sa #Parse(const String&)
    bool Parse(
	const String& line,
	const StringSetCi& delimiters) noexcept;

protected:
    //! Fills \p phrase from \p tokens.
    //! \param tokens the phrase tokens
    //! \param delimiter the delimiter that opened this phrase
    //! \param phrase the phrase to fill
    //! \return \c false if the tokens are illegal
    static bool ParsePhrase(
	const std::vector<String>& tokens,
	const String& delimiter,
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
