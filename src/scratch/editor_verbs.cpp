//! \file editor_verbs.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_EDITOR_VERBS_CPP_

#include <scratch/editor.hpp>
#include <scratch/editor_verbs.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Net {

//! Handles \c @abort.
//! \param editor the editor session
//! \return \c false (finished)
//! \sa #Save(Editor&)
bool EditorVerbs::Abort(Editor& editor) {
    return editor.Finish(false);
}

//! Handles \c @clear.
//! \param editor the editor session
//! \return \c true (still active)
bool EditorVerbs::Clear(Editor& editor) {
    editor.Clear();
    editor.PrintPrompt();
    return true;
}

//! Handles \c @delete \a args.
//! \param editor the editor session
//! \param args the argument text after the verb
//! \return \c true (still active)
bool EditorVerbs::Delete(
	Editor& editor,
	const String& args) {
    if (args.empty()) {
	editor.PrintError("Usage: @delete <line>");
	editor.PrintPrompt();
	return true;
    }
    char* end = nullptr;
    const auto index = std::strtoul(args.c_str(), &end, 10);
    if (end == args.c_str() || (end && *end)) {
	editor.PrintError("Usage: @delete <line>");
	editor.PrintPrompt();
	return true;
    }
    if (!editor.Delete(static_cast<std::size_t>(index))) {
	editor.PrintError("No such line.");
	editor.PrintPrompt();
	return true;
    }
    editor.PrintPrompt();
    return true;
}

//! Handles \c @help.
//! \param editor the editor session
//! \return \c true (still active)
bool EditorVerbs::Help(Editor& editor) {
    editor.PrintHelp();
    editor.PrintPrompt();
    return true;
}

//! Handles \c @list.
//! \param editor the editor session
//! \return \c true (still active)
//! \sa #Listn(Editor&)
bool EditorVerbs::List(Editor& editor) {
    editor.List(false);
    editor.PrintPrompt();
    return true;
}

//! Handles \c @listn.
//! \param editor the editor session
//! \return \c true (still active)
//! \sa #List(Editor&)
bool EditorVerbs::Listn(Editor& editor) {
    editor.List(true);
    editor.PrintPrompt();
    return true;
}

//! Handles \c @save.
//! \param editor the editor session
//! \return \c false (finished)
//! \sa #Abort(Editor&)
bool EditorVerbs::Save(Editor& editor) {
    return editor.Finish(true);
}

//! Handles \c @wrap [\a args].
//! \param editor the editor session
//! \param args the optional continuation indent
//! \return \c true (still active)
bool EditorVerbs::Wrap(
	Editor& editor,
	const String& args) {
    std::size_t indent = 0;
    if (!args.empty()) {
	char* end = nullptr;
	const auto value = std::strtoul(args.c_str(), &end, 10);
	if (end == args.c_str() || (end && *end)) {
	    editor.PrintError("Usage: @wrap [indent]");
	    editor.PrintPrompt();
	    return true;
	}
	indent = static_cast<std::size_t>(value);
    }
    editor.Wrap(indent);
    editor.PrintPrompt();
    return true;
}

}; // namespace Net
}; // namespace Scratch
