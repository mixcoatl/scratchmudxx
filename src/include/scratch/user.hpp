//! \file user.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_USER_HPP_
#define _SCRATCH_USER_HPP_

#include <scratch/color.hpp>
#include <scratch/gender.hpp>
#include <scratch/preference.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>
#include <scratch/thing.hpp>

// Forward declarations.
namespace Scratch {
namespace Utility {
class Data;
}; // namespace Utility
}; // namespace Scratch

namespace Scratch {
namespace Core {

// ScratchMUD types.
using Color = Scratch::Net::Color;
using Data = Scratch::Utility::Data;
using DataPtr = std::shared_ptr<Data>;
using Preference = Scratch::Core::Preference;

//! The user class. \{
class User : public Thing {
public:
    //! Default constructor.
    User() noexcept;

    //! Copy constructor.
    //! \param other the \sa user to copy
    User(const User& other) noexcept;

    //! Destructor.
    virtual ~User() noexcept;

    //! Default assignment.
    //! \param other the \sa user to assign
    User& operator=(const User& other) noexcept;

    //! Adds a player name.
    //! \param player the player name to add
    //! \sa #ErasePlayer(const String&)
    //! \sa #HasPlayer(const String&) const
    //! \sa #SetPlayers(const StringSetCi&)
    void AddPlayer(const String& player) {
	players_.insert(player);
    }

    //! Adds a preference.
    //! \param preference the preference to add
    //! \sa #ErasePreference(const String&)
    //! \sa #HasPreference(const String&) const
    //! \sa #SetPreferences(const StringSetCi&)
    void AddPreference(const String& preference);

    //! Clears the metacolor.
    //! \param meta the metacolor
    //! \sa #SetMetaColor(Color::ColorEnum, Color::ColorEnum)
    void ClearMetaColor(Color::ColorEnum meta) noexcept;

    //! Erases a player name.
    //! \param player the player name to erase
    //! \sa #AddPlayer(const String&)
    //! \sa #HasPlayer(const String&) const
    //! \sa #SetPlayers(const StringSetCi&)
    void ErasePlayer(const String& player) {
	players_.erase(player);
    }

    //! Erases a preference.
    //! \param preference the preference to erase
    //! \sa #AddPreference(const String&)
    //! \sa #HasPreference(const String&) const
    //! \sa #SetPreferences(const StringSetCi&)
    void ErasePreference(const String& preference) {
	preferences_.erase(preference);
    }

    //! Gets the email address.
    //! \sa #SetEmail(const String&)
    String GetEmail() const noexcept {
	return email_;
    }

    //! Gets the gender.
    //! \sa #SetGender(Gender::GenderEnum)
    Gender::GenderEnum GetGender() const noexcept {
	return gender_;
    }

    //! Gets the last login time.
    //! \sa #SetLastLogin(const std::time_t)
    std::time_t GetLastLogin() const noexcept {
	return lastLogin_;
    }

    //! Gets the last logout time.
    //! \sa #SetLastLogout(const std::time_t)
    std::time_t GetLastLogout() const noexcept {
	return lastLogout_;
    }

    //! Gets the metacolor overrides.
    //! \sa #ClearMetaColor(Color::ColorEnum)
    //! \sa #SetMetaColor(Color::ColorEnum, Color::ColorEnum)
    std::map<Color::ColorEnum, Color::ColorEnum> GetMetaColors() const noexcept {
	return metaColors_;
    }

    //! Gets the password.
    //! \sa #SetPassword(const String&)
    String GetPassword() const noexcept {
	return password_;
    }

    //! Gets the player names.
    //! \sa #SetPlayers(const StringSetCi&)
    StringSetCi GetPlayers() const noexcept {
	return players_;
    }

    //! Gets the preferences.
    //! \sa #SetPreferences(const StringSetCi&)
    StringSetCi GetPreferences() const noexcept {
	return preferences_;
    }

    //! Returns whether \p player is present.
    //! \param player the player name to test
    //! \sa #AddPlayer(const String&)
    //! \sa #ErasePlayer(const String&)
    //! \sa #GetPlayers() const
    bool HasPlayer(const String& player) const noexcept {
	return players_.find(player) != players_.end();
    }

    //! Returns whether \p preference is present.
    //! \param preference the preference to test
    //! \sa #AddPreference(const String&)
    //! \sa #ErasePreference(const String&)
    //! \sa #GetPreferences() const
    bool HasPreference(const String& preference) const noexcept {
	return preferences_.find(preference) != preferences_.end();
    }

    //! Reads colors from a data node.
    //! \param data the Colors data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WriteColorsData(const DataPtr&) const
    void ReadColorsData(const DataPtr& data) noexcept;

    //! Reads this user from a data node.
    //! \param data the data node to read
    //! \sa #WriteData(const DataPtr&) const
    void ReadData(const DataPtr& data) noexcept;

    //! Reads players from a data node.
    //! \param data the Players data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WritePlayersData(const DataPtr&) const
    void ReadPlayersData(const DataPtr& data) noexcept;

    //! Reads preferences from a data node.
    //! \param data the Preferences data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WritePreferencesData(const DataPtr&) const
    void ReadPreferencesData(const DataPtr& data) noexcept;

    //! Reads time from a data node.
    //! \param data the Time data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WriteTimeData(const DataPtr&) const
    void ReadTimeData(const DataPtr& data) noexcept;

    //! Sets the email address.
    //! \sa #GetEmail() const
    void SetEmail(const String& email) {
	email_ = email;
    }

    //! Sets the gender.
    //! \param gender the gender
    //! \sa #GetGender() const
    void SetGender(Gender::GenderEnum gender) {
	gender_ = gender;
    }

    //! Sets the last login time.
    //! \sa #GetLastLogin() const
    void SetLastLogin(const std::time_t lastLogin) noexcept {
	lastLogin_ = lastLogin;
    }

    //! Sets the last logout time.
    //! \sa #GetLastLogout() const
    void SetLastLogout(const std::time_t lastLogout) noexcept {
	lastLogout_ = lastLogout;
    }

    //! Sets the metacolor.
    //! \param meta the metacolor
    //! \param color the real color
    //! \return \c true if the metacolor was set
    //! \sa #ClearMetaColor(Color::ColorEnum)
    //! \sa #GetMetaColors() const
    bool SetMetaColor(
	Color::ColorEnum meta,
	Color::ColorEnum color) noexcept;

    //! Sets the password.
    //! \param plain the plaintext password to hash and store
    //! \sa #GetPassword() const
    bool SetPassword(const String& plain) noexcept;

    //! Sets the player names.
    //! \param players the player names to set
    //! \sa #GetPlayers() const
    void SetPlayers(const StringSetCi& players) {
	players_ = players;
    }

    //! Sets the preferences.
    //! \param preferences the preferences to set
    //! \sa #GetPreferences() const
    void SetPreferences(const StringSetCi& preferences) {
	preferences_ = preferences;
    }

    //! Writes colors to a data node.
    //! \param data the Colors data node to write
    //! \sa #ReadColorsData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WriteColorsData(const DataPtr& data) const noexcept;

    //! Writes this user to a data node.
    //! \param data the data node to write
    //! \sa #ReadData(const DataPtr&)
    void WriteData(const DataPtr& data) const noexcept;

    //! Writes players to a data node.
    //! \param data the Players data node to write
    //! \sa #ReadPlayersData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WritePlayersData(const DataPtr& data) const noexcept;

    //! Writes preferences to a data node.
    //! \param data the Preferences data node to write
    //! \sa #ReadPreferencesData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WritePreferencesData(const DataPtr& data) const noexcept;

    //! Writes time to a data node.
    //! \param data the Time data node to write
    //! \sa #ReadTimeData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WriteTimeData(const DataPtr& data) const noexcept;

protected:
    //! The email address.
    //! \sa #GetEmail() const
    //! \sa #SetEmail(const String&)
    String email_;

    //! The gender.
    //! \sa #GetGender() const
    //! \sa #SetGender(Gender::GenderEnum)
    Gender::GenderEnum gender_;

    //! The last login time.
    //! \sa #GetLastLogin() const
    //! \sa #SetLastLogin(const std::time_t)
    std::time_t lastLogin_;

    //! The last logout time.
    //! \sa #GetLastLogout() const
    //! \sa #SetLastLogout(const std::time_t)
    std::time_t lastLogout_;

    //! The metacolor overrides.
    //! \sa #GetMetaColors() const
    //! \sa #SetMetaColor(Color::ColorEnum, Color::ColorEnum)
    std::map<Color::ColorEnum, Color::ColorEnum> metaColors_;

    //! The password.
    //! \sa #GetPassword() const
    //! \sa #SetPassword(const String&)
    String password_;

    //! The player names owned by this user.
    //! \sa #GetPlayers() const
    //! \sa #SetPlayers(const StringSetCi&)
    StringSetCi players_;

    //! The preferences.
    //! \sa #GetPreferences() const
    //! \sa #SetPreferences(const StringSetCi&)
    StringSetCi preferences_;

};
//! \}

//! The type of a shared user pointer.
using UserPtr = std::shared_ptr<User>;

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_USER_HPP_
