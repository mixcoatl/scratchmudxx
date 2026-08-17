//! \file action.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_ACTION_HPP_
#define _SCRATCH_ACTION_HPP_

#include <scratch/color.hpp>
#include <scratch/instance.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! The action target bits. \{
enum ActionTarget: unsigned {
    // Reserved holes for ACT_HIDE / ACT_SLEEP.
    ACT_NOREPEAT  = 1u << 1,	//!< Subject OK instead of echo.
    ACT_TOCHAR    = 1u << 3,	//!< Deliver to subject.
    ACT_TONOTVICT = 1u << 4,	//!< Deliver to others.
    ACT_TOVICT    = 1u << 5,	//!< Deliver to victim.
    ACT_TOROOM    = ACT_TONOTVICT | ACT_TOVICT,	//!< Deliver to room.
    ACT_TOALL     = ACT_TOROOM | ACT_TOCHAR	//!< Deliver to all.
};
//! \}

//! The action param class. \{
class ActionParam {
public:
    //! Default constructor.
    ActionParam() noexcept;

    //! Copy constructor.
    //! \param other the \sa actionparam to copy
    ActionParam(const ActionParam& other) noexcept;

    //! Destructor.
    ~ActionParam() noexcept;

    //! Default assignment.
    //! \param other the \sa actionparam to assign
    ActionParam& operator=(const ActionParam& other) noexcept;

    //! Constructs from text.
    //! \param text the text
    ActionParam(const char* text) noexcept;

    //! Constructs from number.
    //! \param value the value
    ActionParam(const double value);

    //! Constructs from text.
    //! \param text the text
    ActionParam(const String& text) noexcept;

    //! Constructs from instance.
    //! \param instance the instance
    ActionParam(const InstancePtr& instance) noexcept;

    //! Gets the text.
    //! \sa #GetInstance() const
    String GetText() const noexcept {
	return text_;
    }

    //! Gets the instance.
    //! \sa #GetText() const
    InstancePtr GetInstance() const noexcept {
	return instance_;
    }

private:
    //! The text.
    String text_;

    //! The instance.
    InstancePtr instance_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_ACTION_HPP_
