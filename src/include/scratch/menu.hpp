//! \file menu.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_MENU_HPP_
#define _SCRATCH_MENU_HPP_

#include <scratch/color.hpp>
#include <scratch/scratch.hpp>

// Forward declarations.
namespace Scratch {
namespace Net {
class Descriptor;
}; // namespace Net
}; // namespace Scratch

namespace Scratch {
namespace Net {

//! The menu class. \{
class Menu {
public:
    //! The section-kind enumeration. \{
    enum class SectionKind {
	Choices, //!< Selectable action keys; Match applies.
	Listing //!< Reference list indices; not matchable.
    };
    //! \}

    //! Default constructor.
    Menu() noexcept;

    //! Adds a block field row to the current section.
    //! \param key the canonical key or listing index
    //! \param field the field label
    //! \param value the raw value
    //! \param valueColor the value metacolor
    //! \param empty the blank placeholder
    //! \param shown the optional shown substitute when value is non-empty
    //! \return \c false if there is no current section, a duplicate choice
    //!     key, or the section is folded
    bool AddBlock(
	const String& key,
	const String& field,
	const String& value,
	Color::ColorEnum valueColor,
	const String& empty,
	const String& shown);

    //! Adds a line field row to the current section.
    //! \param key the canonical key or listing index
    //! \param field the field label
    //! \param value the raw value
    //! \param valueColor the value metacolor
    //! \param empty the blank placeholder
    //! \param shown the optional shown substitute when value is non-empty
    //! \return \c false if there is no current section or a duplicate
    //!     choice key
    bool AddField(
	const String& key,
	const String& field,
	const String& value,
	Color::ColorEnum valueColor,
	const String& empty,
	const String& shown);

    //! Adds a simple label row to the current section.
    //! \param key the canonical key or listing index
    //! \param label the row label
    //! \return \c false if there is no current section or a duplicate
    //!     choice key
    bool AddItem(
	const String& key,
	const String& label);

    //! Begins a new section.
    //! \param fold whether to column-fold the section
    //! \param kind the section kind
    //! \param title the optional section title
    void AddSection(
	const bool fold,
	const SectionKind kind,
	const String& title = String());

    //! Canonicalizes a menu key.
    //! \param key the raw key
    //! \param out the canonical key
    //! \return \c false if \a key is not a digit token or single letter
    static bool CanonicalizeKey(
	const String& key,
	String& out);

    //! Clears title, sections, and prompt.
    void Clear() noexcept;

    //! Returns whether a prompt has been set.
    bool HasPrompt() const noexcept {
	return !prompt_.empty();
    }

    //! Returns whether any section exists.
    bool HasSection() const noexcept {
	return !sections_.empty();
    }

    //! Matches input against Choices-section keys.
    //! \param line the input line
    //! \param keyOut receives the canonical key on success
    //! \return \c true if a choice key matched
    bool Match(
	const String& line,
	String& keyOut) const;

    //! Renders the menu to the descriptor.
    //! \param d the descriptor
    //! \return \c false if there is no prompt
    bool Print(Descriptor& d) const;

    //! Sets the solicit prompt label (without \c :).
    //! \param prompt the prompt text
    void SetPrompt(const String& prompt) {
	prompt_ = prompt;
    }

    //! Sets a simple menu title.
    //! \param text the title text
    void SetTitle(const String& text);

    //! Sets a field-style menu title (line layout).
    //! \param field the title field label
    //! \param value the title value
    //! \param valueColor the value metacolor
    //! \param empty the blank placeholder
    void SetTitle(
	const String& field,
	const String& value,
	Color::ColorEnum valueColor,
	const String& empty);

protected:
    //! The row structure. \{
    struct Row {
	//! Default constructor.
	Row();
	Row(const Row&) = default;
	Row(Row&&) = default;
	Row& operator=(const Row&) = default;
	Row& operator=(Row&&) = default;
	//! Destructor.
	~Row();

	//! Field layout: \c false = line, \c true = block.
	bool block = false;
	//! The blank placeholder when \ref value is empty.
	String empty;
	//! Whether this row is a field (vs a simple label).
	bool isField = false;
	//! The canonical key or listing index.
	String key;
	//! The label or field name.
	String label;
	//! The optional shown substitute when \ref value is non-empty.
	String shown;
	//! The raw field value.
	String value;
	//! The value metacolor.
	Color::ColorEnum valueColor = Color::C_TEXT;
    };
    //! \}

    //! The section structure. \{
    struct Section {
	//! Default constructor.
	Section();
	Section(const Section&) = default;
	Section(Section&&) = default;
	Section& operator=(const Section&) = default;
	Section& operator=(Section&&) = default;
	//! Destructor.
	~Section();

	//! Whether to column-fold the section.
	bool fold = false;
	//! The section kind.
	SectionKind kind = SectionKind::Choices;
	//! The ordered rows.
	std::vector<Row> rows;
	//! The optional section title.
	String title;
    };
    //! \}

    //! The title structure. \{
    struct Title {
	//! Default constructor.
	Title();
	Title(const Title&) = default;
	Title(Title&&) = default;
	Title& operator=(const Title&) = default;
	Title& operator=(Title&&) = default;
	//! Destructor.
	~Title();

	//! The blank placeholder when \ref value is empty.
	String empty;
	//! The title field label when \ref isField.
	String field;
	//! Whether this is a field-style title.
	bool isField = false;
	//! Whether a title is set.
	bool present = false;
	//! The simple title text when not \ref isField.
	String text;
	//! The title value when \ref isField.
	String value;
	//! The value metacolor when \ref isField.
	Color::ColorEnum valueColor = Color::C_TEXT;
    };
    //! \}

    //! The solicit prompt (without \c :).
    String prompt_;

    //! The ordered sections.
    std::vector<Section> sections_;

    //! The optional title.
    Title title_;

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
    bool AddFieldRow(
	const String& key,
	const String& field,
	const String& value,
	Color::ColorEnum valueColor,
	const String& empty,
	const String& shown,
	const bool block);

    //! Returns whether \a key is already used by a Choices row.
    //! \param key the canonical key
    bool HasChoiceKey(const String& key) const noexcept;

    //! Resolves the displayed value for a field row.
    //! \param row the field row
    static String ResolveValue(const Row& row);
};
//! \}

using MenuPtr = std::shared_ptr<Menu>;

}; // namespace Net
}; // namespace Scratch

#endif // _SCRATCH_MENU_HPP_
