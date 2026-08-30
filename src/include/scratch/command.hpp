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
#include <scratch/trust.hpp>

// Forward declarations.
namespace Scratch {
namespace Scripting {
class CommandBindings;
}; // namespace Scripting
namespace Core {
class Game;
class Instance;
using InstancePtr = std::shared_ptr<Instance>;
}; // namespace Core
namespace Utility {
class Data;
}; // namespace Utility
}; // namespace Scratch

namespace Scratch {
namespace Core {

// Forward declarations.
class User;

// ScratchMUD types.
using Data = Scratch::Utility::Data;
using DataPtr = std::shared_ptr<Data>;
using Trust = Scratch::Core::Trust;
using UserPtr = std::shared_ptr<User>;

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

    //! Returns whether \p performer may run this command.
    //! \param performer the performing user, or null for open commands only
    //! \sa #GetTrust() const
    bool Allows(const UserPtr& performer) const noexcept;

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

    //! Gets the social data.
    //! \sa #SetSocial(const SocialPtr&)
    SocialPtr GetSocial() const noexcept {
	return social_;
    }

    //! Gets the required trust.
    //! \sa #SetTrust(Trust::TrustEnum)
    Trust::TrustEnum GetTrust() const noexcept {
	return trust_;
    }

    //! Reads this command from a data node.
    //! \param data the data node to read
    //! \sa #WriteData(const DataPtr&) const
    void ReadData(const DataPtr& data) noexcept;

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

    //! Sets the required trust.
    //! \param trust the required trust
    //! \sa #GetTrust() const
    void SetTrust(Trust::TrustEnum trust) {
	trust_ = trust;
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

protected:
    friend class Scratch::Scripting::CommandBindings;

    //! Creates social data for this command.
    //! \param command the command
    static void CreateSocialProxy(std::shared_ptr<Command> command);

    //! The Action hook.
    //! \sa #GetAction() const
    //! \sa #SetAction(const String&)
    String action_;

    //! The keywords.
    //! \sa #GetKeywords() const
    //! \sa #SetKeywords(const StringSetCi&)
    StringSetCi keywords_;

    //! The social data.
    //! \sa #GetSocial() const
    //! \sa #SetSocial(const SocialPtr&)
    SocialPtr social_;

    //! The required trust.
    //! \sa #GetTrust() const
    //! \sa #SetTrust(Trust::TrustEnum)
    Trust::TrustEnum trust_;
};
//! \}

//! The type of a shared command pointer.
using CommandPtr = std::shared_ptr<Command>;

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_COMMAND_HPP_
