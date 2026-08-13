//! \file editor.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_EDITOR_CPP_

#include <scratch/color.hpp>
#include <scratch/descriptor.hpp>
#include <scratch/editor.hpp>
#include <scratch/editor_verbs.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Net {

namespace {

//! Canonical TELNET line separator for joined length and GetText.
const String& JoinSep() {
    static const String sep("\r\n");
    return sep;
}

//! Keeps whole rows that fit under \a maxLength.
std::vector<String> TruncateRows(
	const std::vector<String>& rows,
	const std::size_t maxLength) {
    std::vector<String> kept;
    for (const auto& row : rows) {
	auto candidate = kept;
	candidate.push_back(row);
	if (Scratch::Algorithm::StringJoin(JoinSep(), candidate).size() > maxLength)
	    break;
	kept.swap(candidate);
    }
    return kept;
}

} // namespace

//! Constructor.
//! \param descriptor the owning descriptor
Editor::Editor(Descriptor& descriptor) noexcept :
	aborted_(false),
	active_(false),
	descriptor_(descriptor),
	lines_(),
	maxLength_(0),
	tag_() {
}

//! Appends one buffer row.
//! \param line the row to append
//! \return \c true while the session stays active
bool Editor::Append(const String& line) {
    auto candidate = lines_;
    candidate.push_back(line);
    if (Scratch::Algorithm::StringJoin(JoinSep(), candidate).size() > maxLength_) {
	this->PrintError("Line would exceed maximum length.");
	this->PrintPrompt();
	return true;
    }
    lines_.swap(candidate);
    this->PrintPrompt();
    return true;
}

//! Removes all buffer rows.
void Editor::Clear() noexcept {
    lines_.clear();
}

//! Deletes a 1-based buffer row.
//! \param index the 1-based row index
//! \return \c false if \a index is out of range
bool Editor::Delete(const std::size_t index) {
    if (index < 1 || index > lines_.size())
	return false;
    lines_.erase(lines_.begin() + static_cast<std::ptrdiff_t>(index - 1));
    return true;
}

//! Ends the session.
//! \param accepted \c true for \c @save; \c false for \c @abort
//! \return \c false (session finished)
//! \sa #Receive(const String&)
bool Editor::Finish(const bool accepted) {
    active_ = false;
    aborted_ = !accepted;
    return false;
}

//! Returns the current joined length.
//! \sa #GetMaxLength() const
//! \sa #GetText() const
std::size_t Editor::GetLength() const noexcept {
    return Scratch::Algorithm::StringJoin(JoinSep(), lines_).size();
}

//! Returns the joined storage string.
//! \sa #GetLength() const
//! \sa #SetText(const String&)
String Editor::GetText() const {
    return Scratch::Algorithm::StringJoin(JoinSep(), lines_);
}

//! Lists the buffer to the descriptor.
//! \param numbers whether to prefix 1-based line numbers
//! \sa #PrintPrompt() const
void Editor::List(const bool numbers) const {
    const auto height = static_cast<std::size_t>(descriptor_.GetWindowHeight());
    std::size_t body = 3;
    if (height > 4)
	body = height - 2;
    if (body < 3)
	body = 3;

    const auto* cPrompt = descriptor_.GetColor(Color::C_PROMPT);
    const auto* cPunct = descriptor_.GetColor(Color::C_PUNCTUATION);
    const auto* cNumber = descriptor_.GetColor(Color::C_NUMBER);
    const auto* cText = descriptor_.GetColor(Color::C_TEXT);
    const auto* cNormal = descriptor_.GetColor(Color::C_NORMAL);

    if (lines_.empty()) {
	std::ostringstream empty;
	empty << cText << "(empty)" << cNormal << "\r\n";
	descriptor_.Print(empty.str());
	return;
    }

    const auto show = std::min(body, lines_.size());
    for (std::size_t i = 0; i < show; ++i) {
	std::ostringstream row;
	if (numbers) {
	    row << cNumber << std::setfill('0') << std::setw(3) << (i + 1)
		<< cPunct << ": " << cText;
	} else {
	    row << cText;
	}
	row << lines_[i] << cNormal << "\r\n";
	descriptor_.Print(row.str());
    }
    if (lines_.size() > show) {
	std::ostringstream more;
	more << cPrompt << "(" << (lines_.size() - show)
	     << " more lines)" << cNormal << "\r\n";
	descriptor_.Print(more.str());
    }
}

//! Prints an editor error.
//! \param message the error text
void Editor::PrintError(const String& message) const {
    std::ostringstream oss;
    oss << descriptor_.GetColor(Color::C_FAILED) << message
	<< descriptor_.GetColor(Color::C_NORMAL) << "\r\n";
    descriptor_.Print(oss.str());
}

//! Prints the full \c @help listing.
//! \sa #PrintInstructions() const
void Editor::PrintHelp() const {
    const auto* cPrompt = descriptor_.GetColor(Color::C_PROMPT);
    const auto* cPunct = descriptor_.GetColor(Color::C_PUNCTUATION);
    const auto* cEmphasis = descriptor_.GetColor(Color::C_EMPHASIS);
    const auto* cNormal = descriptor_.GetColor(Color::C_NORMAL);

    static const struct {
	const char* verb;
	const char* detail;
    } rows[] = {
	{"@abort", "discard changes and leave the editor"},
	{"@clear", "delete every line in the buffer"},
	{"@delete <n>", "delete 1-based line n"},
	{"@help", "show this help"},
	{"@list", "show the buffer"},
	{"@listn", "show the buffer with line numbers"},
	{"@save", "keep changes and leave the editor"},
	{"@wrap [indent]", "reflow lines to the window width"},
	{"@@text", "insert a line that begins with a single @"},
    };

    std::size_t verbWidth = 0;
    for (const auto& row : rows)
	verbWidth = std::max(verbWidth, String(row.verb).size());

    std::ostringstream oss;
    oss << cPrompt << "Editor commands" << cPunct << ':'
	<< cNormal << "\r\n";
    descriptor_.Print(oss.str());
    for (const auto& row : rows) {
	oss.str(String());
	oss.clear();
	oss << cPunct << " * " << cEmphasis << std::left
	    << std::setw(static_cast<int>(verbWidth)) << row.verb
	    << cPunct << " - " << cPrompt << row.detail << cNormal << "\r\n";
	descriptor_.Print(oss.str());
    }
}

//! Prints the short entry instructions line.
//! \sa #PrintHelp() const
//! \sa #Start(const String&, const String&, const std::size_t)
void Editor::PrintInstructions() const {
    const auto* cPrompt = descriptor_.GetColor(Color::C_PROMPT);
    const auto* cPunct = descriptor_.GetColor(Color::C_PUNCTUATION);
    const auto* cEmphasis = descriptor_.GetColor(Color::C_EMPHASIS);
    const auto* cNormal = descriptor_.GetColor(Color::C_NORMAL);

    std::ostringstream oss;
    oss << cPrompt << "Instructions" << cPunct << ": "
	<< cEmphasis << "@save" << cPrompt << " to save, "
	<< cEmphasis << "@abort" << cPrompt << " to abort, or "
	<< cEmphasis << "@help" << cPrompt << " for more options."
	<< cNormal << "\r\n";
    descriptor_.Print(oss.str());
}

//! Prints the standing editor prompt.
//! \sa #List(const bool) const
void Editor::PrintPrompt() const {
    std::ostringstream oss;
    oss << descriptor_.GetColor(Color::C_PROMPT);
    if (!tag_.empty()) {
	oss << tag_;
	oss << descriptor_.GetColor(Color::C_PUNCTUATION) << ' ';
	oss << descriptor_.GetColor(Color::C_PROMPT);
    }
    oss << descriptor_.GetColor(Color::C_NUMBER)
	<< std::setfill('0')
	<< std::setw(static_cast<int>(std::to_string(maxLength_).size()))
	<< this->GetLength()
	<< descriptor_.GetColor(Color::C_PUNCTUATION) << '/'
	<< descriptor_.GetColor(Color::C_NUMBER) << maxLength_
	<< descriptor_.GetColor(Color::C_PUNCTUATION) << "> "
	<< descriptor_.GetColor(Color::C_NORMAL);
    descriptor_.Print(oss.str());
}

//! Feeds one input line while active.
//! \param line the raw input line (no trailing CR/LF)
//! \return \c true if the session is still active; \c false if finished
//! \sa #Finish(const bool)
bool Editor::Receive(const String& line) {
    if (!active_)
	return false;

    if (line.size() >= 2 && line[0] == '@' && line[1] == '@')
	return this->Append(line.substr(1));

    if (!line.empty() && line[0] == '@') {
	String command;
	String args;
	const auto sp = line.find_first_of(" \t", 1);
	if (sp == String::npos) {
	    command = line.substr(1);
	} else {
	    command = line.substr(1, sp - 1);
	    const auto rest = line.find_first_not_of(" \t", sp);
	    if (rest != String::npos)
		args = line.substr(rest);
	}
	return this->Dispatch(command, args);
    }

    return this->Append(line);
}

//! Dispatches one @-command.
//! \param command the verb name (without \c @)
//! \param args the remainder of the line
//! \return \c true if still active; \c false if finished
//! \sa #Receive(const String&)
//! \sa EditorVerbs
bool Editor::Dispatch(
	const String& command,
	const String& args) {
    if (Scratch::Algorithm::StringCompareCi(command, "abort") == 0)
	return EditorVerbs::Abort(*this);
    if (Scratch::Algorithm::StringCompareCi(command, "clear") == 0)
	return EditorVerbs::Clear(*this);
    if (Scratch::Algorithm::StringCompareCi(command, "delete") == 0)
	return EditorVerbs::Delete(*this, args);
    if (Scratch::Algorithm::StringCompareCi(command, "help") == 0)
	return EditorVerbs::Help(*this);
    if (Scratch::Algorithm::StringCompareCi(command, "list") == 0)
	return EditorVerbs::List(*this);
    if (Scratch::Algorithm::StringCompareCi(command, "listn") == 0)
	return EditorVerbs::Listn(*this);
    if (Scratch::Algorithm::StringCompareCi(command, "save") == 0)
	return EditorVerbs::Save(*this);
    if (Scratch::Algorithm::StringCompareCi(command, "wrap") == 0)
	return EditorVerbs::Wrap(*this, args);

    this->PrintError("Unknown editor command.");
    this->PrintPrompt();
    return true;
}

//! Replaces the buffer from text.
//! \param text the replacement text
//! \return \c true after applying whole-row truncate to fit
//! \sa #GetText() const
//! \sa #Start(const String&, const String&, const std::size_t)
bool Editor::SetText(const String& text) {
    const auto rows = Scratch::Algorithm::StringSplitLines(text);
    const auto kept = TruncateRows(rows, maxLength_);
    const bool truncated = kept.size() < rows.size() ||
	(kept.empty() && !rows.empty() &&
	 Scratch::Algorithm::StringJoin(JoinSep(), rows).size() > maxLength_);
    lines_ = kept;
    if (truncated)
	this->PrintError("Text truncated to fit maximum length.");
    return true;
}

//! Loads text, sets limits, lists, and marks active.
//! \param text the initial buffer
//! \param tag the opaque caller tag
//! \param maxLength the maximum joined byte length
//! \sa #SetText(const String&)
void Editor::Start(
	const String& text,
	const String& tag,
	const std::size_t maxLength) {
    tag_ = tag;
    maxLength_ = maxLength ? maxLength : 1024;
    aborted_ = false;
    active_ = true;
    this->SetText(text);
    this->List(false);
    this->PrintInstructions();
    this->PrintPrompt();
}

//! Reflows the buffer to the descriptor window width.
//! \param indent leading spaces on continuation lines
//! \return \c false if the wrapped join would exceed maxLength_
bool Editor::Wrap(const std::size_t indent) {
    auto width = static_cast<std::size_t>(descriptor_.GetWindowWidth());
    if (width < 8)
	width = 8;
    if (indent >= width)
	width = indent + 1;

    const String pad(indent, ' ');
    std::vector<String> wrapped;
    for (const auto& src : lines_) {
	if (src.empty()) {
	    wrapped.push_back(String());
	    continue;
	}
	std::size_t pos = 0;
	bool first = true;
	while (pos < src.size()) {
	    const auto limit = first ? width : (width > indent ? width - indent : 1);
	    if (src.size() - pos <= limit) {
		wrapped.push_back((first ? String() : pad) + src.substr(pos));
		break;
	    }
	    auto breakAt = pos + limit;
	    auto space = src.rfind(' ', breakAt);
	    if (space == String::npos || space < pos)
		space = breakAt;
	    wrapped.push_back((first ? String() : pad) + src.substr(pos, space - pos));
	    pos = space;
	    while (pos < src.size() && src[pos] == ' ')
		++pos;
	    first = false;
	}
    }

    if (Scratch::Algorithm::StringJoin(JoinSep(), wrapped).size() > maxLength_) {
	this->PrintError("Wrap would exceed maximum length.");
	return false;
    }
    lines_.swap(wrapped);
    return true;
}

}; // namespace Net
}; // namespace Scratch
