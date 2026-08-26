//! \file editor.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_EDITOR_HPP_
#define _SCRATCH_EDITOR_HPP_

#include <scratch/scratch.hpp>

// Forward declarations.
namespace Scratch {
namespace Net {
class Descriptor;
}; // namespace Net
}; // namespace Scratch

namespace Scratch {
namespace Net {

//! The editor class. \{
class Editor {
public:
    //! Constructor.
    //! \param descriptor the owning descriptor
    explicit Editor(Descriptor& descriptor) noexcept;

    //! Appends one buffer row.
    //! \param line the row to append
    //! \return \c true while the session stays active
    bool Append(const String& line);

    //! Removes all buffer rows.
    void Clear() noexcept;

    //! Deletes a 1-based buffer row.
    //! \param index the 1-based row index
    //! \return \c false if \a index is out of range
    bool Delete(const std::size_t index);

    //! Ends the session.
    //! \param accepted \c true for \c @save; \c false for \c @abort
    //! \return \c false (session finished)
    //! \sa #Receive(const String&)
    bool Finish(const bool accepted);

    //! Returns the current joined length.
    //! \sa #GetMaxLength() const
    //! \sa #GetText() const
    std::size_t GetLength() const noexcept;

    //! Returns the maximum joined length.
    //! \sa #GetLength() const
    std::size_t GetMaxLength() const noexcept {
	return maxLength_;
    }

    //! Returns the opaque caller tag.
    //! \sa #Start(const String&, const String&, const std::size_t)
    String GetTag() const noexcept {
	return tag_;
    }

    //! Returns the joined storage string.
    //! \sa #GetLength() const
    //! \sa #SetText(const String&)
    String GetText() const;

    //! Returns whether the last finish was \c @abort.
    //! \sa #IsSaved() const
    bool IsAborted() const noexcept {
	return aborted_;
    }

    //! Returns whether this session is intercepting input.
    bool IsActive() const noexcept {
	return active_;
    }

    //! Returns whether the last finish was \c @save.
    //! \sa #IsAborted() const
    bool IsSaved() const noexcept {
	return !aborted_;
    }

    //! Lists the buffer to the descriptor.
    //! \param numbers whether to prefix 1-based line numbers
    //! \sa #PrintPrompt() const
    void List(const bool numbers) const;

    //! Prints an editor error.
    //! \param message the error text
    void PrintError(const String& message) const;

    //! Prints the full \c @help listing.
    //! \sa #PrintInstructions() const
    void PrintHelp() const;

    //! Prints the short entry instructions line.
    //! \sa #PrintHelp() const
    //! \sa #Start(const String&, const String&, const std::size_t)
    void PrintInstructions() const;

    //! Prints the standing editor prompt.
    //! \sa #List(const bool) const
    void PrintPrompt() const;

    //! Feeds one input line while active.
    //! \param line the raw input line (no trailing CR/LF)
    //! \return \c true if the session is still active; \c false if finished
    //! \sa #Finish(const bool)
    bool Receive(const String& line);

    //! Replaces the buffer from text.
    //! \param text the replacement text
    //! \return \c true after applying whole-row truncate to fit
    //! \sa #GetText() const
    //! \sa #Start(const String&, const String&, const std::size_t)
    bool SetText(const String& text);

    //! Loads text, sets limits, lists, and marks active.
    //! \param text the initial buffer
    //! \param tag the opaque caller tag
    //! \param maxLength the maximum joined byte length
    //! \sa #SetText(const String&)
    void Start(
	const String& text,
	const String& tag = String(),
	const std::size_t maxLength = 1024);

    //! Reflows the buffer to the descriptor window width.
    //! \param indent leading spaces on continuation lines
    //! \return \c false if the wrapped join would exceed maxLength_
    bool Wrap(const std::size_t indent = 0);

protected:
    //! Dispatches one @-command.
    //! \param command the verb name (without \c @)
    //! \param args the remainder of the line
    //! \return \c true if still active; \c false if finished
    //! \sa #Receive(const String&)
    //! \sa EditorVerbs
    bool Dispatch(
	const String& command,
	const String& args);

    //! The aborted bit.
    //! \sa #IsAborted() const
    //! \sa #Finish(const bool)
    bool aborted_;

    //! The active bit.
    //! \sa #IsActive() const
    bool active_;

    //! The owning descriptor.
    Descriptor& descriptor_;

    //! The buffer rows.
    std::vector<String> lines_;

    //! The maximum joined byte length.
    //! \sa #GetMaxLength() const
    std::size_t maxLength_;

    //! The opaque caller tag.
    //! \sa #GetTag() const
    String tag_;
};
//! \}

using EditorPtr = std::shared_ptr<Editor>;

}; // namespace Net
}; // namespace Scratch

#endif // _SCRATCH_EDITOR_HPP_
