//! \file player.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_PLAYER_CPP_

#include <scratch/data.hpp>
#include <scratch/player.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Core {

//! Default constructor.
Player::Player() noexcept :
	Thing(),
	gender_(Gender::GENDER_UNDEFINED),
	owner_(),
	permissions_(),
	preferences_() {
    // Nothing.
}

//! Copy constructor.
//! \param other the \sa player to copy
Player::Player(const Player& other) noexcept :
	Thing(other),
	gender_(other.gender_),
	owner_(other.owner_),
	permissions_(other.permissions_),
	preferences_(other.preferences_) {
    // Nothing.
}

//! Destructor.
Player::~Player() noexcept {
    // Nothing.
}

//! Default assignment.
//! \param other the \sa player to assign
Player& Player::operator=(const Player& other) noexcept {
    Thing::operator=(other);
    gender_ = other.gender_;
    owner_ = other.owner_;
    permissions_ = other.permissions_;
    preferences_ = other.preferences_;
    return *this;
}

//! Reads this player from a data node.
//! \param data the data node to read
//! \sa #WriteData(const DataPtr&) const
void Player::ReadData(const DataPtr& data) noexcept {
    if (!data)
	return;

    name_ = data->GetString("Name");
    owner_ = data->GetString("Owner");
    gender_ = Gender::ByName(data->GetString("Gender"));

    auto permissionsData = data->Get("Permissions");
    if (!permissionsData)
	permissionsData = std::make_shared<Data>();
    this->ReadPermissionsData(permissionsData);

    auto preferencesData = data->Get("Preferences");
    if (!preferencesData)
	preferencesData = std::make_shared<Data>();
    this->ReadPreferencesData(preferencesData);

    auto metadataData = data->Get("Metadata");
    if (!metadataData)
	metadataData = std::make_shared<Data>();
    this->ReadMetadataData(metadataData);
}

//! Reads permissions from a data node.
//! \param data the Permissions data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WritePermissionsData(const DataPtr&) const
void Player::ReadPermissionsData(const DataPtr& data) noexcept {
    permissions_.clear();
    for (const auto& entry: data->GetEntries()) {
	const auto permission = data->GetString(entry.first);
	if (!permission.empty())
	    permissions_.insert(permission);
    }
}

//! Reads preferences from a data node.
//! \param data the Preferences data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WritePreferencesData(const DataPtr&) const
void Player::ReadPreferencesData(const DataPtr& data) noexcept {
    preferences_.clear();
    for (const auto& entry: data->GetEntries()) {
	const auto preference = data->GetString(entry.first);
	if (!preference.empty())
	    preferences_.insert(preference);
    }
}

//! Writes this player to a data node.
//! \param data the data node to write
//! \sa #ReadData(const DataPtr&)
void Player::WriteData(const DataPtr& data) const noexcept {
    if (!data)
	return;

    if (!name_.empty())
	data->PutString("Name", name_);
    if (!owner_.empty())
	data->PutString("Owner", owner_);
    if (Gender::IsDefined(gender_))
	data->PutString("Gender", Gender::ToString(gender_));

    auto permissionsData = std::make_shared<Data>();
    this->WritePermissionsData(permissionsData);
    if (permissionsData->Size())
	data->Put("Permissions", permissionsData);

    auto preferencesData = std::make_shared<Data>();
    this->WritePreferencesData(preferencesData);
    if (preferencesData->Size())
	data->Put("Preferences", preferencesData);

    auto metadataData = std::make_shared<Data>();
    this->WriteMetadataData(metadataData);
    if (metadataData->Size())
	data->Put("Metadata", metadataData);
}

//! Writes permissions to a data node.
//! \param data the Permissions data node to write
//! \sa #ReadPermissionsData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void Player::WritePermissionsData(const DataPtr& data) const noexcept {
    for (const auto& permission: permissions_)
	data->PutString("%", permission);
}

//! Writes preferences to a data node.
//! \param data the Preferences data node to write
//! \sa #ReadPreferencesData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void Player::WritePreferencesData(const DataPtr& data) const noexcept {
    for (const auto& preference: preferences_)
	data->PutString("%", preference);
}

}; // namespace Core
}; // namespace Scratch
