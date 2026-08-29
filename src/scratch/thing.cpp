//! \file thing.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_THING_CPP_

#include <scratch/data.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>
#include <scratch/thing.hpp>

namespace Scratch {
namespace Core {

//! Default constructor.
Thing::Thing() noexcept :
	std::enable_shared_from_this<Thing>(),
	created_(0),
	createdBy_(),
	modified_(0),
	modifiedBy_(),
	name_() {
    // Nothing.
}

//! Copy constructor.
//! \param other the \sa thing to copy
Thing::Thing(const Thing& other) noexcept :
	std::enable_shared_from_this<Thing>(),
	created_(other.created_),
	createdBy_(other.createdBy_),
	modified_(other.modified_),
	modifiedBy_(other.modifiedBy_),
	name_(other.name_) {
    // Nothing.
}

//! Destructor.
Thing::~Thing() noexcept {
    // Nothing.
}

//! Default assignment.
//! \param other the \sa thing to assign
Thing& Thing::operator=(const Thing& other) noexcept {
    created_ = other.created_;
    createdBy_ = other.createdBy_;
    modified_ = other.modified_;
    modifiedBy_ = other.modifiedBy_;
    name_ = other.name_;
    return *this;
}

//! Finds a thing by name.
//! \param game the game state
//! \param name the name token
//! \return the matched thing, or \c nullptr
ThingPtr Thing::Find(
	const Game& /*game*/,
	const String& name) const noexcept {
    if (Scratch::Algorithm::Strings::CompareCi(name, "me") != 0 &&
	    Scratch::Algorithm::Strings::CompareCi(name, "self") != 0)
	return nullptr;
    try {
	return std::const_pointer_cast<Thing>(this->shared_from_this());
    } catch (const std::bad_weak_ptr&) {
	return nullptr;
    }
}

//! Reads metadata from a data node.
//! \param data the Metadata data node to read
//! \sa #WriteMetadataData(const DataPtr&) const
void Thing::ReadMetadataData(const DataPtr& data) noexcept {
    created_ = data->GetTime("Created");
    createdBy_ = data->GetString("CreatedBy");
    modified_ = data->GetTime("Modified");
    modifiedBy_ = data->GetString("ModifiedBy");
}

//! Writes metadata to a data node.
//! \param data the Metadata data node to write
//! \sa #ReadMetadataData(const DataPtr&)
void Thing::WriteMetadataData(const DataPtr& data) const noexcept {
    if (created_)
	data->PutTime("Created", created_);
    if (!createdBy_.empty())
	data->PutString("CreatedBy", createdBy_);
    if (modified_)
	data->PutTime("Modified", modified_);
    if (!modifiedBy_.empty())
	data->PutString("ModifiedBy", modifiedBy_);
}

}; // namespace Core
}; // namespace Scratch
