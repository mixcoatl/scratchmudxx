//! \file user.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_USER_HPP_
#define _SCRATCH_USER_HPP_

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
using Data = Scratch::Utility::Data;
using DataPtr = std::shared_ptr<Data>;

//! The user account class. \{
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

    //! Gets the email address.
    //! \sa #SetEmail(const String&)
    String GetEmail() const noexcept {
	return email_;
    }

    //! Gets the gender.
    //! \sa #SetGender(const String&)
    String GetGender() const noexcept {
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

    //! Gets the password.
    //! \sa #SetPassword(const String&)
    String GetPassword() const noexcept {
	return password_;
    }

    //! Reads login tracking from a data node.
    //! \param data the Time data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WriteTimeData(const DataPtr&) const
    void ReadTimeData(const DataPtr& data) noexcept;

    //! Reads this user from a data node.
    //! \param data the data node to read
    //! \sa #WriteData(const DataPtr&) const
    void ReadData(const DataPtr& data) noexcept;

    //! Sets the email address.
    //! \sa #GetEmail() const
    void SetEmail(const String& email) {
	email_ = email;
    }

    //! Sets the gender.
    //! \sa #GetGender() const
    void SetGender(const String& gender) {
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

    //! Sets the password.
    //! \param plain the plaintext password to hash and store
    //! \sa #GetPassword() const
    bool SetPassword(const String& plain) noexcept;

    //! Writes login tracking to a data node.
    //! \param data the Time data node to write
    //! \sa #ReadTimeData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WriteTimeData(const DataPtr& data) const noexcept;

    //! Writes this user to a data node.
    //! \param data the data node to write
    //! \sa #ReadData(const DataPtr&)
    void WriteData(const DataPtr& data) const noexcept;

protected:
    //! The email address.
    //! \sa #GetEmail() const
    //! \sa #SetEmail(const String&)
    String email_;

    //! The gender.
    //! \sa #GetGender() const
    //! \sa #SetGender(const String&)
    String gender_;

    //! The last login time.
    //! \sa #GetLastLogin() const
    //! \sa #SetLastLogin(const std::time_t)
    std::time_t lastLogin_;

    //! The last logout time.
    //! \sa #GetLastLogout() const
    //! \sa #SetLastLogout(const std::time_t)
    std::time_t lastLogout_;

    //! The password.
    //! \sa #GetPassword() const
    //! \sa #SetPassword(const String&)
    String password_;
};
//! \}

//! The type of a shared user pointer.
using UserPtr = std::shared_ptr<User>;

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_USER_HPP_
