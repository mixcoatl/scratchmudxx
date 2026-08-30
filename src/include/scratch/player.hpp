//! \file player.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_PLAYER_HPP_
#define _SCRATCH_PLAYER_HPP_

#include <scratch/gender.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>
#include <scratch/thing.hpp>
#include <scratch/trust.hpp>

// Forward declarations.
namespace Scratch {
namespace Utility {
class Data;
}; // namespace Utility
}; // namespace Scratch

namespace Scratch {
namespace Core {

// ScratchMUD types.
using Data = Scratch::Utility::Data;
using DataPtr = std::shared_ptr<Data>;
using Trust = Scratch::Core::Trust;

//! The player class. \{
class Player : public Thing {
public:
    //! Default constructor.
    Player() noexcept;

    //! Copy constructor.
    //! \param other the \sa player to copy
    Player(const Player& other) noexcept;

    //! Destructor.
    virtual ~Player() noexcept;

    //! Default assignment.
    //! \param other the \sa player to assign
    Player& operator=(const Player& other) noexcept;

    //! Adds a preference.
    //! \param preference the preference to add
    //! \sa #ErasePreference(const String&)
    //! \sa #HasPreference(const String&) const
    //! \sa #SetPreferences(const StringSetCi&)
    void AddPreference(const String& preference);

    //! Erases a preference.
    //! \param preference the preference to erase
    //! \sa #AddPreference(const String&)
    //! \sa #HasPreference(const String&) const
    //! \sa #SetPreferences(const StringSetCi&)
    void ErasePreference(const String& preference) {
	preferences_.erase(preference);
    }

    //! Gets the gender.
    //! \sa #SetGender(Gender::GenderEnum)
    Gender::GenderEnum GetGender() const noexcept {
	return gender_;
    }

    //! Gets the owner name.
    //! \sa #SetOwner(const String&)
    String GetOwner() const noexcept {
	return owner_;
    }

    //! Gets the preferences.
    //! \sa #SetPreferences(const StringSetCi&)
    StringSetCi GetPreferences() const noexcept {
	return preferences_;
    }

    //! Gets the trust.
    //! \sa #SetTrust(Trust::TrustEnum)
    Trust::TrustEnum GetTrust() const noexcept {
	return trust_;
    }

    //! Returns whether \p preference is present.
    //! \param preference the preference to test
    //! \sa #AddPreference(const String&)
    //! \sa #ErasePreference(const String&)
    //! \sa #GetPreferences() const
    bool HasPreference(const String& preference) const noexcept {
	return preferences_.find(preference) != preferences_.end();
    }

    //! Reads this player from a data node.
    //! \param data the data node to read
    //! \sa #WriteData(const DataPtr&) const
    void ReadData(const DataPtr& data) noexcept;

    //! Reads preferences from a data node.
    //! \param data the Preferences data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WritePreferencesData(const DataPtr&) const
    void ReadPreferencesData(const DataPtr& data) noexcept;

    //! Sets the gender.
    //! \param gender the gender
    //! \sa #GetGender() const
    void SetGender(Gender::GenderEnum gender) {
	gender_ = gender;
    }

    //! Sets the owner name.
    //! \param owner the owner name
    //! \sa #GetOwner() const
    void SetOwner(const String& owner) {
	owner_ = owner;
    }

    //! Sets the preferences.
    //! \param preferences the preferences to set
    //! \sa #GetPreferences() const
    void SetPreferences(const StringSetCi& preferences) {
	preferences_ = preferences;
    }

    //! Sets the trust.
    //! \param trust the trust
    //! \sa #GetTrust() const
    void SetTrust(Trust::TrustEnum trust) {
	trust_ = trust;
    }

    //! Writes this player to a data node.
    //! \param data the data node to write
    //! \sa #ReadData(const DataPtr&)
    void WriteData(const DataPtr& data) const noexcept;

    //! Writes preferences to a data node.
    //! \param data the Preferences data node to write
    //! \sa #ReadPreferencesData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WritePreferencesData(const DataPtr& data) const noexcept;

protected:
    //! The gender.
    //! \sa #GetGender() const
    //! \sa #SetGender(Gender::GenderEnum)
    Gender::GenderEnum gender_;

    //! The owner name.
    //! \sa #GetOwner() const
    //! \sa #SetOwner(const String&)
    String owner_;

    //! The preferences.
    //! \sa #GetPreferences() const
    //! \sa #SetPreferences(const StringSetCi&)
    StringSetCi preferences_;

    //! The trust.
    //! \sa #GetTrust() const
    //! \sa #SetTrust(Trust::TrustEnum)
    Trust::TrustEnum trust_;
};
//! \}

//! The type of a shared player pointer.
using PlayerPtr = std::shared_ptr<Player>;

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_PLAYER_HPP_
