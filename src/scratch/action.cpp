//! \file action.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_ACTION_CPP_

#include <scratch/action.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

#include <sstream>

namespace Scratch {
namespace Core {

//! Default constructor.
ActionParam::ActionParam() noexcept :
	text_(),
	instance_() {
    // Nothing.
}

//! Copy constructor.
//! \param other the \sa actionparam to copy
ActionParam::ActionParam(const ActionParam& other) noexcept :
	text_(other.text_),
	instance_(other.instance_) {
    // Nothing.
}

//! Destructor.
ActionParam::~ActionParam() noexcept {
    // Nothing.
}

//! Default assignment.
//! \param other the \sa actionparam to assign
ActionParam& ActionParam::operator=(const ActionParam& other) noexcept {
    text_ = other.text_;
    instance_ = other.instance_;
    return *this;
}

//! Constructs from text.
//! \param text the text
ActionParam::ActionParam(const char* text) noexcept :
	text_(text ? text : ""),
	instance_() {
    // Nothing.
}

//! Constructs from number.
//! \param value the value
ActionParam::ActionParam(const double value) :
	text_(),
	instance_() {
    std::ostringstream out;
    out << value;
    text_ = out.str();
}

//! Constructs from text.
//! \param text the text
ActionParam::ActionParam(const String& text) noexcept :
	text_(text),
	instance_() {
    // Nothing.
}

//! Constructs from instance.
//! \param instance the instance
ActionParam::ActionParam(const InstancePtr& instance) noexcept :
	text_(),
	instance_(instance) {
    // Nothing.
}

}; // namespace Core
}; // namespace Scratch
