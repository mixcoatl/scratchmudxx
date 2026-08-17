//! \file command.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_COMMAND_HPP_
#define _SCRATCH_COMMAND_HPP_

#include <scratch/scratch.hpp>
#include <scratch/social.hpp>
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

// Forward declarations.
class Instance;

// ScratchMUD types.
using Data = Scratch::Utility::Data;
using DataPtr = std::shared_ptr<Data>;
using InstancePtr = std::shared_ptr<Instance>;

//! The command class. \{
class Command : public Thing {
public:
    //! Default constructor.
    Command() noexcept;

    //! Copy constructor.
    //! \param other the \sa command to copy
    Command(const Command& other) noexcept;

    //! Destructor.
    virtual ~Command() noexcept;

    //! Default assignment.
    //! \param other the \sa command to assign
    Command& operator=(const Command& other) noexcept;

    //! Adds a permission.
    //! \param permission the permission to add
    //! \sa #ErasePermission(const String&)
    //! \sa #HasPermission(const String&) const
    //! \sa #SetPermissions(const StringSetCi&)
    void AddPermission(const String& permission) {
	permissions_.insert(permission);
    }

    //! Returns whether \p performer may run this command.
    //! \param performer the performing instance, or null for open commands only
    //! \sa #GetPermissions() const
    //! \sa #HasPermission(const String&) const
    bool Allows(const InstancePtr& performer) const noexcept;

    //! Erases a permission.
    //! \param permission the permission to erase
    //! \sa #AddPermission(const String&)
    //! \sa #HasPermission(const String&) const
    //! \sa #SetPermissions(const StringSetCi&)
    void ErasePermission(const String& permission) {
	permissions_.erase(permission);
    }

    //! Gets the Action hook.
    //! \sa #SetAction(const String&)
    String GetAction() const noexcept {
	return action_;
    }

    //! Gets the keywords.
    //! \sa #SetKeywords(const StringSetCi&)
    StringSetCi GetKeywords() const noexcept {
	return keywords_;
    }

    //! Gets the permissions.
    //! \sa #SetPermissions(const StringSetCi&)
    StringSetCi GetPermissions() const noexcept {
	return permissions_;
    }

    //! Gets the social data.
    //! \sa #SetSocial(const SocialPtr&)
    SocialPtr GetSocial() const noexcept {
	return social_;
    }

    //! Returns whether \p permission is present.
    //! \param permission the permission to test
    //! \sa #AddPermission(const String&)
    //! \sa #ErasePermission(const String&)
    //! \sa #GetPermissions() const
    bool HasPermission(const String& permission) const noexcept {
	return permissions_.find(permission) != permissions_.end();
    }

    //! Reads this command from a data node.
    //! \param data the data node to read
    //! \sa #WriteData(const DataPtr&) const
    void ReadData(const DataPtr& data) noexcept;

    //! Reads permissions from a data node.
    //! \param data the Permissions data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WritePermissionsData(const DataPtr&) const
    void ReadPermissionsData(const DataPtr& data) noexcept;

    //! Sets the Action hook.
    //! \sa #GetAction() const
    void SetAction(const String& action) {
	action_ = action;
    }

    //! Sets the keywords.
    //! \sa #GetKeywords() const
    void SetKeywords(const StringSetCi& keywords) {
	keywords_ = keywords;
    }

    //! Sets the permissions.
    //! \param permissions the permissions to set
    //! \sa #GetPermissions() const
    void SetPermissions(const StringSetCi& permissions) {
	permissions_ = permissions;
    }

    //! Sets the social data.
    //! \sa #GetSocial() const
    void SetSocial(const SocialPtr& social) {
	social_ = social;
    }

    //! Writes this command to a data node.
    //! \param data the data node to write
    //! \sa #ReadData(const DataPtr&)
    void WriteData(const DataPtr& data) const noexcept;

    //! Writes permissions to a data node.
    //! \param data the Permissions data node to write
    //! \sa #ReadPermissionsData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WritePermissionsData(const DataPtr& data) const noexcept;

protected:
    //! The Action hook.
    //! \sa #GetAction() const
    //! \sa #SetAction(const String&)
    String action_;

    //! The keywords.
    //! \sa #GetKeywords() const
    //! \sa #SetKeywords(const StringSetCi&)
    StringSetCi keywords_;

    //! The permissions.
    //! \sa #GetPermissions() const
    //! \sa #SetPermissions(const StringSetCi&)
    StringSetCi permissions_;

    //! The social data.
    //! \sa #GetSocial() const
    //! \sa #SetSocial(const SocialPtr&)
    SocialPtr social_;
};
//! \}

//! The type of a shared command pointer.
using CommandPtr = std::shared_ptr<Command>;

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_COMMAND_HPP_
