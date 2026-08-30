//! \file player.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_PLAYER_CPP_

#include <scratch/data.hpp>
#include <scratch/logger.hpp>
#include <scratch/player.hpp>
#include <scratch/scratch.hpp>

namespace Scratch {
namespace Core {

// ScratchMUD types.
using Strings = Scratch::Algorithm::Strings;

//! Default constructor.
Player::Player() noexcept :
	Thing(),
	gender_(Gender::GENDER_UNDEFINED),
	owner_(),
	preferences_(),
	trust_(Trust::TRUST_PLAYER) {
    // Nothing.
}

//! Copy constructor.
//! \param other the \sa player to copy
Player::Player(const Player& other) noexcept :
	Thing(other),
	gender_(other.gender_),
	owner_(other.owner_),
	preferences_(other.preferences_),
	trust_(other.trust_) {
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
    preferences_ = other.preferences_;
    trust_ = other.trust_;
    return *this;
}

//! Adds a preference.
//! \param preference the preference to add
void Player::AddPreference(const String& preference) {
    if (preference.empty() ||
	Strings::CompareCi(Strings::SanitizeCopy(preference), preference))
	throw std::invalid_argument("invalid preference id");
    preferences_.insert(preference);
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
    trust_ = Trust::ByName(data->GetString("Trust"));
    if (!Trust::IsDefined(trust_))
	trust_ = Trust::TRUST_PLAYER;

    // Read preferences.
    auto preferencesData = data->Get("Preferences", std::make_shared<Data>());
    this->ReadPreferencesData(preferencesData);

    // Read metadata.
    auto metadataData = data->Get("Metadata", std::make_shared<Data>());
    this->ReadMetadataData(metadataData);
}

//! Reads preferences from a data node.
//! \param data the Preferences data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WritePreferencesData(const DataPtr&) const
void Player::ReadPreferencesData(const DataPtr& data) noexcept {
    preferences_.clear();
    auto loadedPreferences = data->GetStringSet("");
    for (const auto& p: loadedPreferences) {
	if (!Strings::CompareCi(Strings::SanitizeCopy(p), p))
	    preferences_.insert(p);
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

    if (trust_ != Trust::TRUST_PLAYER)
	data->PutString("Trust", Trust::ToString(trust_));

    // Write preferences.
    auto preferencesData = std::make_shared<Data>();
    this->WritePreferencesData(preferencesData);
    if (preferencesData->Size())
	data->Put("Preferences", preferencesData);

    // Write metadata.
    auto metadataData = std::make_shared<Data>();
    this->WriteMetadataData(metadataData);
    if (metadataData->Size())
	data->Put("Metadata", metadataData);
}

//! Writes preferences to a data node.
//! \param data the Preferences data node to write
//! \sa #ReadPreferencesData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void Player::WritePreferencesData(const DataPtr& data) const noexcept {
    data->PutStringSet("", preferences_);
}

}; // namespace Core
}; // namespace Scratch
