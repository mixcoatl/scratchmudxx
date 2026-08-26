//! \file editor_verbs.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_EDITOR_VERBS_HPP_
#define _SCRATCH_EDITOR_VERBS_HPP_

#include <scratch/editor.hpp>

namespace Scratch {
namespace Net {

//! The editor verbs class. \{
class EditorVerbs {
public:
    //! Handles \c @abort.
    //! \param editor the editor session
    //! \return \c false (finished)
    //! \sa #Save(Editor&)
    static bool Abort(Editor& editor);

    //! Handles \c @clear.
    //! \param editor the editor session
    //! \return \c true (still active)
    static bool Clear(Editor& editor);

    //! Handles \c @delete \a args.
    //! \param editor the editor session
    //! \param args the argument text after the verb
    //! \return \c true (still active)
    static bool Delete(
	Editor& editor,
	const String& args);

    //! Handles \c @help.
    //! \param editor the editor session
    //! \return \c true (still active)
    static bool Help(Editor& editor);

    //! Handles \c @list.
    //! \param editor the editor session
    //! \return \c true (still active)
    //! \sa #Listn(Editor&)
    static bool List(Editor& editor);

    //! Handles \c @listn.
    //! \param editor the editor session
    //! \return \c true (still active)
    //! \sa #List(Editor&)
    static bool Listn(Editor& editor);

    //! Handles \c @save.
    //! \param editor the editor session
    //! \return \c false (finished)
    //! \sa #Abort(Editor&)
    static bool Save(Editor& editor);

    //! Handles \c @wrap [\a args].
    //! \param editor the editor session
    //! \param args the optional continuation indent
    //! \return \c true (still active)
    static bool Wrap(
	Editor& editor,
	const String& args);

private:
    EditorVerbs() = delete;
};
//! \}

}; // namespace Net
}; // namespace Scratch

#endif // _SCRATCH_EDITOR_VERBS_HPP_
