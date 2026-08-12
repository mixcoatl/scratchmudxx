//! \file user.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_USER_CPP_

#include <scratch/data.hpp>
#include <scratch/gender.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>
#include <scratch/user.hpp>

namespace Scratch {
namespace Core {

//! Default constructor.
User::User() noexcept :
	Thing(),
	email_(),
	gender_(Gender::GENDER_UNDEFINED),
	lastLogin_(0),
	lastLogout_(0),
	metaColors_(),
	password_(),
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
    preferences_ = other.preferences_;
    return *this;
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
    for (const auto& entry: data->GetEntries()) {
	auto meta = Color::ByName(entry.first);
	auto assigned = Color::ByName(data->GetString(entry.first));
	this->SetMetaColor(meta, assigned);
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
    auto preferencesData = data->Get("Preferences");
    if (!preferencesData)
	preferencesData = std::make_shared<Data>();
    this->ReadPreferencesData(preferencesData);

    // Read colors.
    auto colorsData = data->Get("Colors");
    if (!colorsData)
	colorsData = std::make_shared<Data>();
    this->ReadColorsData(colorsData);

    // Read time.
    auto timeData = data->Get("Time");
    if (!timeData)
	timeData = std::make_shared<Data>();
    this->ReadTimeData(timeData);

    // Read metadata.
    auto metadataData = data->Get("Metadata");
    if (!metadataData)
	metadataData = std::make_shared<Data>();
    this->ReadMetadataData(metadataData);
}

//! Reads preferences from a data node.
//! \param data the Preferences data node to read
//! \sa #ReadData(const DataPtr&)
//! \sa #WritePreferencesData(const DataPtr&) const
void User::ReadPreferencesData(const DataPtr& data) noexcept {
    preferences_.clear();
    for (const auto& entry: data->GetEntries()) {
	const auto preference = data->GetString(entry.first);
	if (!preference.empty())
	    preferences_.insert(preference);
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
    if (!Color::IsMeta(meta) || !Color::IsReal(color))
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
    password_ = Scratch::Algorithm::StringCryptCopy(plain);
    return true;
}

//! Writes colors to a data node.
//! \param data the Colors data node to write
//! \sa #ReadColorsData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void User::WriteColorsData(const DataPtr& data) const noexcept {
    for (const auto& entry: metaColors_) {
	data->PutString(
	    Color::ToString(entry.first),
	    Color::ToString(entry.second));
    }
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

//! Writes preferences to a data node.
//! \param data the Preferences data node to write
//! \sa #ReadPreferencesData(const DataPtr&)
//! \sa #WriteData(const DataPtr&) const
void User::WritePreferencesData(const DataPtr& data) const noexcept {
    for (const auto& preference: preferences_)
	data->PutString("%", preference);
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
