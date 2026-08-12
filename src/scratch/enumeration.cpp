//! \file enumeration.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_ENUMERATION_CPP_

#include <scratch/data.hpp>
#include <scratch/enumeration.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! Default constructor.
Enumeration::Enumeration() noexcept :
	Thing(),
	members_() {
    // Nothing.
}

//! Copy constructor.
//! \param other the \sa enumeration to copy
Enumeration::Enumeration(const Enumeration& other) noexcept :
	Thing(other),
	members_(other.members_) {
    // Nothing.
}

//! Destructor.
Enumeration::~Enumeration() noexcept {
    // Nothing.
}

//! Default assignment.
//! \param other the \sa enumeration to assign
Enumeration& Enumeration::operator=(const Enumeration& other) noexcept {
    Thing::operator=(other);
    members_ = other.members_;
    return *this;
}

//! Reads this enumeration from a data node.
//! \param data the data node to read
//! \sa #WriteData(const DataPtr&) const
void Enumeration::ReadData(const DataPtr& data) noexcept {
    if (!data)
	return;

    // Read members.
    auto membersData = data->Get("Members");
    if (!membersData)
	membersData = std::make_shared<Data>();
    this->ReadMembersData(membersData);

    // Read metadata.
    auto metadataData = data->Get("Metadata");
    if (!metadataData)
	metadataData = std::make_shared<Data>();
    this->ReadMetadataData(metadataData);
}

//! Reads members from a data node.
//! \param data the Members data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WriteMembersData(const DataPtr&) const
void Enumeration::ReadMembersData(const DataPtr& data) noexcept {
    members_.clear();
    for (const auto& entry: data->GetEntries()) {
	const auto member = data->GetString(entry.first);
	if (!member.empty())
	    members_.insert(member);
    }
}

//! Writes this enumeration to a data node.
//! \param data the data node to write
//! \sa #ReadData(const DataPtr&)
void Enumeration::WriteData(const DataPtr& data) const noexcept {
    if (!data)
	return;

    // Write members.
    auto membersData = std::make_shared<Data>();
    this->WriteMembersData(membersData);
    if (membersData->Size())
	data->Put("Members", membersData);

    // Write metadata.
    auto metadataData = std::make_shared<Data>();
    this->WriteMetadataData(metadataData);
    if (metadataData->Size())
	data->Put("Metadata", metadataData);
}

//! Writes members to a data node.
//! \param data the Members data node to write
//! \sa #ReadMembersData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void Enumeration::WriteMembersData(const DataPtr& data) const noexcept {
    for (const auto& member: members_)
	data->PutString("%", member);
}

}; // namespace Core
}; // namespace Scratch
