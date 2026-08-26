//! \file menu.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_MENU_CPP_

#include <scratch/color.hpp>
#include <scratch/descriptor.hpp>
#include <scratch/menu.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Net {

// ScratchMUD types.
using Strings = Scratch::Algorithm::Strings;

namespace {

//! Returns the blank/shown display for a title value.
String ResolveTitleValue(
	const String& value,
	const String& empty) {
    if (value.empty())
	return empty.empty() ? String("<Blank>") : empty;
    return value;
}

} // namespace

//! Default constructor.
Menu::Menu() noexcept :
	prompt_(),
	sections_(),
	title_() {
}

//! Default constructor.
Menu::Row::Row() :
	block(false),
	empty("<Blank>"),
	isField(false),
	key(),
	label(),
	shown(),
	value(),
	valueColor(Color::C_TEXT) {
}

//! Destructor.
Menu::Row::~Row() = default;

//! Default constructor.
Menu::Section::Section() :
	fold(false),
	kind(SectionKind::Choices),
	rows(),
	title() {
}

//! Destructor.
Menu::Section::~Section() = default;

//! Default constructor.
Menu::Title::Title() :
	empty("<Blank>"),
	field(),
	isField(false),
	present(false),
	text(),
	value(),
	valueColor(Color::C_TEXT) {
}

//! Destructor.
Menu::Title::~Title() = default;

//! Adds a block field row to the current section.
//! \param key the canonical key or listing index
//! \param field the field label
//! \param value the raw value
//! \param valueColor the value metacolor
//! \param empty the blank placeholder
//! \param shown the optional shown substitute when value is non-empty
//! \return \c false if there is no current section, a duplicate choice
//!     key, or the section is folded
bool Menu::AddBlock(
	const String& key,
	const String& field,
	const String& value,
	Color::ColorEnum valueColor,
	const String& empty,
	const String& shown) {
    return AddFieldRow(key, field, value, valueColor, empty, shown, true);
}

//! Adds a line field row to the current section.
//! \param key the canonical key or listing index
//! \param field the field label
//! \param value the raw value
//! \param valueColor the value metacolor
//! \param empty the blank placeholder
//! \param shown the optional shown substitute when value is non-empty
//! \return \c false if there is no current section or a duplicate
//!     choice key
bool Menu::AddField(
	const String& key,
	const String& field,
	const String& value,
	Color::ColorEnum valueColor,
	const String& empty,
	const String& shown) {
    return AddFieldRow(key, field, value, valueColor, empty, shown, false);
}

//! Adds a field row with the given block flag.
//! \param key the canonical key or listing index
//! \param field the field label
//! \param value the raw value
//! \param valueColor the value metacolor
//! \param empty the blank placeholder
//! \param shown the optional shown substitute when value is non-empty
//! \param block whether to use block layout
//! \return \c false if there is no current section, a duplicate choice
//!     key, or a block row in a folded section
bool Menu::AddFieldRow(
	const String& key,
	const String& field,
	const String& value,
	Color::ColorEnum valueColor,
	const String& empty,
	const String& shown,
	const bool block) {
    if (sections_.empty())
	return false;
    if (sections_.back().kind == SectionKind::Choices && HasChoiceKey(key))
	return false;
    if (block && sections_.back().fold)
	return false;
    Row row;
    row.key = key;
    row.isField = true;
    row.block = block;
    row.label = field;
    row.value = value;
    row.empty = empty.empty() ? String("<Blank>") : empty;
    row.shown = shown;
    row.valueColor = valueColor;
    sections_.back().rows.push_back(std::move(row));
    return true;
}

//! Adds a simple label row to the current section.
//! \param key the canonical key or listing index
//! \param label the row label
//! \return \c false if there is no current section or a duplicate
//!     choice key
bool Menu::AddItem(
	const String& key,
	const String& label) {
    if (sections_.empty())
	return false;
    if (sections_.back().kind == SectionKind::Choices && HasChoiceKey(key))
	return false;
    Row row;
    row.key = key;
    row.isField = false;
    row.label = label;
    sections_.back().rows.push_back(std::move(row));
    return true;
}

//! Begins a new section.
//! \param fold whether to column-fold the section
//! \param kind the section kind
//! \param title the optional section title
void Menu::AddSection(
	const bool fold,
	const SectionKind kind,
	const String& title) {
    Section section;
    section.fold = fold;
    section.kind = kind;
    section.title = title;
    sections_.push_back(std::move(section));
}

boost::optional<String> Menu::MatchKey(const String& line) const {
    String key;
    return this->Match(line, key) ? boost::optional<String>(key) :
	boost::none;
}

//! Canonicalizes a menu key.
//! \param key the raw key
//! \param out the canonical key
//! \return \c false if \a key is not a digit token or single letter
bool Menu::CanonicalizeKey(
	const String& key,
	String& out) {
    if (key.empty())
	return false;
    bool digits = true;
    for (const auto ch : key) {
	if (!std::isdigit(static_cast<unsigned char>(ch))) {
	    digits = false;
	    break;
	}
    }
    if (digits) {
	try {
	    out = std::to_string(std::stoll(key));
	} catch (...) {
	    return false;
	}
	return true;
    }
    if (key.size() == 1 && std::isalpha(static_cast<unsigned char>(key[0]))) {
	out.assign(1, static_cast<char>(
	    std::tolower(static_cast<unsigned char>(key[0]))));
	return true;
    }
    return false;
}

//! Clears title, sections, and prompt.
void Menu::Clear() noexcept {
    prompt_.clear();
    sections_.clear();
    title_ = Title();
}

//! Matches input against Choices-section keys.
//! \param line the input line
//! \param keyOut receives the canonical key on success
//! \return \c true if a choice key matched
bool Menu::Match(
	const String& line,
	String& keyOut) const {
    std::size_t begin = 0;
    while (begin < line.size() &&
	std::isspace(static_cast<unsigned char>(line[begin])))
	++begin;
    if (begin >= line.size())
	return false;
    std::size_t end = begin;
    while (end < line.size() &&
	!std::isspace(static_cast<unsigned char>(line[end])))
	++end;
    const String token = line.substr(begin, end - begin);

    String canonical;
    if (!CanonicalizeKey(token, canonical))
	return false;

    const bool digitKey = !canonical.empty() &&
	std::isdigit(static_cast<unsigned char>(canonical[0]));
    long long tokenN = 0;
    if (digitKey) {
	try {
	    tokenN = std::stoll(canonical);
	} catch (...) {
	    return false;
	}
    }

    for (const auto& section : sections_) {
	if (section.kind != SectionKind::Choices)
	    continue;
	for (const auto& row : section.rows) {
	    if (digitKey) {
		bool rowDigits = true;
		for (const auto ch : row.key) {
		    if (!std::isdigit(static_cast<unsigned char>(ch))) {
			rowDigits = false;
			break;
		    }
		}
		if (!rowDigits)
		    continue;
		try {
		    if (std::stoll(row.key) == tokenN) {
			keyOut = row.key;
			return true;
		    }
		} catch (...) {
		    continue;
		}
	    } else if (row.key == canonical) {
		keyOut = row.key;
		return true;
	    }
	}
    }
    return false;
}

//! Renders the menu to the descriptor.
//! \param d the descriptor
//! \return \c false if there is no prompt
bool Menu::Print(Descriptor& d) const {
    if (prompt_.empty())
	return false;

    const auto* cKey = d.GetColor(Color::C_KEY);
    const auto* cEmphasis = d.GetColor(Color::C_EMPHASIS);
    const auto* cPrompt = d.GetColor(Color::C_PROMPT);
    const auto* cPunct = d.GetColor(Color::C_PUNCTUATION);
    const auto* cNormal = d.GetColor(Color::C_NORMAL);

    auto formatKeyChrome = [&](
	    const SectionKind kind,
	    const String& key) -> String {
	const bool digit = !key.empty() &&
	    std::isdigit(static_cast<unsigned char>(key[0]));
	std::ostringstream shown;
	if (digit)
	    shown << std::setfill('0') << std::setw(2) << std::stoll(key);
	else
	    shown << static_cast<char>(
		std::toupper(static_cast<unsigned char>(key[0])));

	std::ostringstream chrome;
	if (kind == SectionKind::Listing)
	    chrome << " - " << cKey << '[' << shown.str() << ']'
		   << cPunct << ' ';
	else if (digit)
	    chrome << cKey << shown.str() << cPunct << ") ";
	else
	    chrome << ' ' << cKey << shown.str() << cPunct << ") ";
	return chrome.str();
    };

    if (title_.present) {
	std::ostringstream line;
	if (!title_.isField) {
	    line << cPrompt << title_.text
		 << cNormal << "\r\n";
	} else {
	    line << cPrompt << title_.field << cPunct << ": "
		 << d.GetColor(title_.valueColor)
		 << ResolveTitleValue(title_.value, title_.empty)
		 << cNormal << "\r\n";
	}
	d.Print(line.str());
    }

    auto appendLineRow = [&](
	    const Section& section,
	    const Row& row,
	    const std::size_t fieldWidth,
	    const std::size_t valueWidth,
	    String& cell) {
	std::ostringstream line;
	line << formatKeyChrome(section.kind, row.key);
	if (!row.isField) {
	    line << cPrompt << std::left << std::setfill(' ')
		 << std::setw(static_cast<int>(valueWidth)) << row.label
		 << cNormal;
	    cell += line.str();
	    return;
	}
	if (row.block) {
	    line << cPrompt << row.label << cPunct << ":-" << cNormal;
	    cell += line.str();
	    return;
	}
	const auto dots = std::max<std::size_t>(3,
	    fieldWidth + 3 - row.label.size());
	const auto shown = ResolveValue(row);
	line << cPrompt << row.label << ' ' << cPunct
	     << std::setfill('.') << std::setw(static_cast<int>(dots)) << '.'
	     << std::setfill(' ') << " : "
	     << d.GetColor(row.valueColor) << std::left
	     << std::setw(static_cast<int>(valueWidth)) << shown
	     << cNormal;
	cell += line.str();
    };

    auto appendBlockBody = [&](const Row& row, String& dest) {
	// start_editor handoff; menus only preview.
	auto lines = Strings::SplitLines(ResolveValue(row));
	// Drop trailing blank preview rows.
	while (lines.size() > 1 && lines.back().empty())
	    lines.pop_back();
	static const std::size_t kPreviewMax = 3;
	const bool more = lines.size() > kPreviewMax;
	const auto show = std::min(kPreviewMax, lines.size());
	for (std::size_t i = 0; i < show; ++i) {
	    dest += d.GetColor(row.valueColor);
	    dest += lines[i];
	    dest += cNormal;
	    dest += "\r\n";
	}
	if (more) {
	    dest += cPrompt;
	    dest += "...";
	    dest += cNormal;
	    dest += "\r\n";
	}
    };

    for (const auto& section : sections_) {
	if (!section.title.empty()) {
	    std::ostringstream line;
	    line << cPunct << "-- " << cEmphasis << section.title
		 << cPunct << " --" << cNormal << "\r\n";
	    d.Print(line.str());
	}

	std::size_t fieldWidth = 0;
	std::size_t valueWidth = 0;
	std::size_t labelWidth = 0;
	for (const auto& row : section.rows) {
	    if (row.isField) {
		fieldWidth = std::max(fieldWidth, row.label.size());
		if (!row.block)
		    valueWidth = std::max(valueWidth, ResolveValue(row).size());
	    } else {
		labelWidth = std::max(labelWidth, row.label.size());
	    }
	}

	const bool fold = section.fold && !section.rows.empty();
	if (!fold) {
	    for (const auto& row : section.rows) {
		String cell;
		appendLineRow(section, row, fieldWidth, 0, cell);
		cell += "\r\n";
		if (row.isField && row.block)
		    appendBlockBody(row, cell);
		d.Print(cell);
	    }
	    continue;
	}

	std::vector<String> cells;
	cells.reserve(section.rows.size());
	for (const auto& row : section.rows) {
	    String cell;
	    if (row.isField) {
		appendLineRow(section, row, fieldWidth, valueWidth, cell);
	    } else {
		appendLineRow(section, row, 0, labelWidth, cell);
	    }
	    cells.push_back(std::move(cell));
	}
	d.PrintColumns(cells);
    }

    std::ostringstream prompt;
    prompt << cPrompt << prompt_ << cPunct << ':' << cNormal << ' ';
    d.Print(prompt.str());
    return true;
}

//! Sets a simple menu title.
//! \param text the title text
void Menu::SetTitle(const String& text) {
    title_ = Title();
    title_.present = true;
    title_.isField = false;
    title_.text = text;
}

//! Sets a field-style menu title (line layout).
//! \param field the title field label
//! \param value the title value
//! \param valueColor the value metacolor
//! \param empty the blank placeholder
void Menu::SetTitle(
	const String& field,
	const String& value,
	Color::ColorEnum valueColor,
	const String& empty) {
    title_ = Title();
    title_.present = true;
    title_.isField = true;
    title_.field = field;
    title_.value = value;
    title_.valueColor = valueColor;
    title_.empty = empty.empty() ? String("<Blank>") : empty;
}

//! Returns whether \a key is already used by a Choices row.
//! \param key the canonical key
bool Menu::HasChoiceKey(const String& key) const noexcept {
    for (const auto& section : sections_) {
	if (section.kind != SectionKind::Choices)
	    continue;
	for (const auto& row : section.rows) {
	    if (row.key == key)
		return true;
	}
    }
    return false;
}

//! Resolves the displayed value for a field row.
//! \param row the field row
String Menu::ResolveValue(const Row& row) {
    if (row.value.empty())
	return row.empty;
    if (!row.shown.empty())
	return row.shown;
    return row.value;
}

}; // namespace Net
}; // namespace Scratch
