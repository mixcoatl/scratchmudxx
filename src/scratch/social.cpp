//! \file social.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_SOCIAL_CPP_

#include <scratch/data.hpp>
#include <scratch/scratch.hpp>
#include <scratch/social.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! Default constructor.
Social::Social() noexcept :
	found_(),
	foundAuto_(),
	noArgument_() {
    // Nothing.
}

//! Copy constructor.
//! \param other the \sa social to copy
Social::Social(const Social& other) noexcept :
	found_(other.found_),
	foundAuto_(other.foundAuto_),
	noArgument_(other.noArgument_) {
    // Nothing.
}

//! Destructor.
Social::~Social() noexcept {
    // Nothing.
}

//! Default assignment.
//! \param other the \sa social to assign
Social& Social::operator=(const Social& other) noexcept {
    found_ = other.found_;
    foundAuto_ = other.foundAuto_;
    noArgument_ = other.noArgument_;
    return *this;
}

//! Reads this social from a data node.
//! \param data the Social data node to read
//! \sa #WriteData(const DataPtr&) const
void Social::ReadData(const DataPtr& data) noexcept {
    if (!data)
	return;

    found_ = data->GetString("Found");
    foundAuto_ = data->GetString("FoundAuto");
    noArgument_ = data->GetString("NoArgument");
}

//! Writes this social to a data node.
//! \param data the Social data node to write
//! \sa #ReadData(const DataPtr&)
void Social::WriteData(const DataPtr& data) const noexcept {
    if (!data)
	return;

    if (!found_.empty())
	data->PutString("Found", found_);
    if (!foundAuto_.empty())
	data->PutString("FoundAuto", foundAuto_);
    if (!noArgument_.empty())
	data->PutString("NoArgument", noArgument_);
}

}; // namespace Core
}; // namespace Scratch
