//! \file user.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_USER_CPP_

#include <scratch/data.hpp>
#include <scratch/logger.hpp>
#include <scratch/preference.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>
#include <scratch/user.hpp>

namespace Scratch {
namespace Core {

// ScratchMUD types.
using Strings = Scratch::Algorithm::Strings;

//! Default constructor.
User::User() noexcept :
	Thing(),
	email_(),
	gender_(Gender::GENDER_UNDEFINED),
	lastLogin_(0),
	lastLogout_(0),
	metaColors_(),
	password_(),
	players_(),
	preferences_() {
    // Nothing.
}

//! Copy constructor.
//! \param other the \sa user to copy
User::User(const User& other) noexcept :
	Thing(other),
	email_(other.email_),
	gender_(other.gender_),
	lastLogin_(other.lastLogin_),
	lastLogout_(other.lastLogout_),
	metaColors_(other.metaColors_),
	password_(other.password_),
	players_(other.players_),
	preferences_(other.preferences_) {
    // Nothing.
}

//! Destructor.
User::~User() noexcept {
    // Nothing.
}

//! Default assignment.
//! \param other the \sa user to assign
User& User::operator=(const User& other) noexcept {
    Thing::operator=(other);
    email_ = other.email_;
    gender_ = other.gender_;
    lastLogin_ = other.lastLogin_;
    lastLogout_ = other.lastLogout_;
    metaColors_ = other.metaColors_;
    password_ = other.password_;
    players_ = other.players_;
    preferences_ = other.preferences_;
    return *this;
}

//! Adds a preference.
//! \param preference the preference to add
void User::AddPreference(const String& preference) {
    if (preference.empty() ||
	Strings::CompareCi(Strings::SanitizeCopy(preference), preference))
	throw std::invalid_argument("invalid preference id");
    preferences_.insert(preference);
}

//! Clears the metacolor.
//! \param meta the metacolor
//! \sa #SetMetaColor(Color::ColorEnum, Color::ColorEnum)
void User::ClearMetaColor(Color::ColorEnum meta) noexcept {
    metaColors_.erase(meta);
}

//! Reads colors from a data node.
//! \param data the Colors data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WriteColorsData(const DataPtr&) const
void User::ReadColorsData(const DataPtr& data) noexcept {
    metaColors_.clear();
    for (const auto& entry: data->GetStringMap("")) {
	const auto meta = Color::ByName(entry.first);
	const auto assigned = Color::ByName(entry.second);
	if (!Color::IsMetaColor(meta) || Color::IsMetaColor(assigned)) {
	    LOGGER_STORAGE() << "Unknown color " << entry.first << ": " << entry.second << ".";
	    continue;
	}
	metaColors_[meta] = assigned;
    }
}

//! Reads this user from a data node.
//! \param data the data node to read
//! \sa #WriteData(const DataPtr&) const
void User::ReadData(const DataPtr& data) noexcept {
    if (!data)
	return;

    name_ = data->GetString("Name");
    email_ = data->GetString("Email");
    gender_ = Gender::ByName(data->GetString("Gender"));
    password_ = data->GetString("Password");

    // Read preferences.
    auto preferencesData = data->Get("Preferences", std::make_shared<Data>());
    this->ReadPreferencesData(preferencesData);

    // Read players.
    auto playersData = data->Get("Players", std::make_shared<Data>());
    this->ReadPlayersData(playersData);

    // Read colors.
    auto colorsData = data->Get("Colors", std::make_shared<Data>());
    this->ReadColorsData(colorsData);

    // Read time.
    auto timeData = data->Get("Time", std::make_shared<Data>());
    this->ReadTimeData(timeData);

    // Read metadata.
    auto metadataData = data->Get("Metadata", std::make_shared<Data>());
    this->ReadMetadataData(metadataData);
}

//! Reads players from a data node.
//! \param data the Players data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WritePlayersData(const DataPtr&) const
void User::ReadPlayersData(const DataPtr& data) noexcept {
    players_ = data->GetStringSet("");
}

//! Reads preferences from a data node.
//! \param data the Preferences data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WritePreferencesData(const DataPtr&) const
void User::ReadPreferencesData(const DataPtr& data) noexcept {
    preferences_.clear();
    auto loadedPreferences = data->GetStringSet("");
    for (const auto& p: loadedPreferences) {
	if (!Strings::CompareCi(Strings::SanitizeCopy(p), p))
	    preferences_.insert(p);
    }
}

//! Reads time from a data node.
//! \param data the Time data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WriteTimeData(const DataPtr&) const
void User::ReadTimeData(const DataPtr& data) noexcept {
    lastLogin_ = data->GetTime("LastLogin");
    lastLogout_ = data->GetTime("LastLogout");
}

//! Sets the metacolor.
//! \param meta the metacolor
//! \param color the real color
//! \return \c true if the metacolor was set
//! \sa #ClearMetaColor(Color::ColorEnum)
//! \sa #GetMetaColors() const
bool User::SetMetaColor(
	Color::ColorEnum meta,
	Color::ColorEnum color) noexcept {
    if (!Color::IsMetaColor(meta) || Color::IsMetaColor(color))
	return false;
    metaColors_[meta] = color;
    return true;
}

//! Sets the password.
//! \param plain the plaintext password to hash and store
//! \sa #GetPassword() const
bool User::SetPassword(const String& plain) noexcept {
    if (plain.empty()) {
	password_.clear();
	return false;
    }
    password_ = Scratch::Algorithm::Strings::CryptCopy(plain);
    return true;
}

//! Writes colors to a data node.
//! \param data the Colors data node to write
//! \sa #ReadColorsData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void User::WriteColorsData(const DataPtr& data) const noexcept {
    StringMapCi<String> map;
    for (const auto& entry: metaColors_) {
	map[Color::ToString(entry.first)] =
	    Color::ToString(entry.second);
    }
    data->PutStringMap("", map);
}

//! Writes this user to a data node.
//! \param data the data node to write
//! \sa #ReadData(const DataPtr&)
void User::WriteData(const DataPtr& data) const noexcept {
    if (!data)
	return;

    if (!name_.empty())
	data->PutString("Name", name_);
    if (!email_.empty())
	data->PutString("Email", email_);
    if (Gender::IsDefined(gender_))
	data->PutString("Gender", Gender::ToString(gender_));
    if (!password_.empty())
	data->PutString("Password", password_);

    // Write preferences.
    auto preferencesData = std::make_shared<Data>();
    this->WritePreferencesData(preferencesData);
    if (preferencesData->Size())
	data->Put("Preferences", preferencesData);

    // Write players.
    auto playersData = std::make_shared<Data>();
    this->WritePlayersData(playersData);
    if (playersData->Size())
	data->Put("Players", playersData);

    // Write colors.
    auto colorsData = std::make_shared<Data>();
    this->WriteColorsData(colorsData);
    if (colorsData->Size())
	data->Put("Colors", colorsData);

    // Write time.
    auto timeData = std::make_shared<Data>();
    this->WriteTimeData(timeData);
    if (timeData->Size())
	data->Put("Time", timeData);

    // Write metadata.
    auto metadataData = std::make_shared<Data>();
    this->WriteMetadataData(metadataData);
    if (metadataData->Size())
	data->Put("Metadata", metadataData);
}

//! Writes players to a data node.
//! \param data the Players data node to write
//! \sa #ReadPlayersData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void User::WritePlayersData(const DataPtr& data) const noexcept {
    data->PutStringSet("", players_);
}

//! Writes preferences to a data node.
//! \param data the Preferences data node to write
//! \sa #ReadPreferencesData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void User::WritePreferencesData(const DataPtr& data) const noexcept {
    data->PutStringSet("", preferences_);
}

//! Writes time to a data node.
//! \param data the Time data node to write
//! \sa #ReadTimeData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void User::WriteTimeData(const DataPtr& data) const noexcept {
    if (lastLogin_)
	data->PutTime("LastLogin", lastLogin_);
    if (lastLogout_)
	data->PutTime("LastLogout", lastLogout_);
}

}; // namespace Core
}; // namespace Scratch
