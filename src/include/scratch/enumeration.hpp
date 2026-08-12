//! \file enumeration.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_ENUMERATION_HPP_
#define _SCRATCH_ENUMERATION_HPP_

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

//! The enumeration class. \{
class Enumeration : public Thing {
public:
    //! Default constructor.
    Enumeration() noexcept;

    //! Copy constructor.
    //! \param other the \sa enumeration to copy
    Enumeration(const Enumeration& other) noexcept;

    //! Destructor.
    virtual ~Enumeration() noexcept;

    //! Default assignment.
    //! \param other the \sa enumeration to assign
    Enumeration& operator=(const Enumeration& other) noexcept;

    //! Adds a member.
    //! \param member the member to add
    //! \sa #Contains(const String&) const
    //! \sa #EraseMember(const String&)
    //! \sa #SetMembers(const StringSetCi&)
    void AddMember(const String& member) {
	members_.insert(member);
    }

    //! Returns whether \p member is present.
    //! \param member the member to test
    //! \sa #AddMember(const String&)
    //! \sa #EraseMember(const String&)
    //! \sa #GetMembers() const
    bool Contains(const String& member) const noexcept {
	return members_.find(member) != members_.end();
    }

    //! Erases a member.
    //! \param member the member to erase
    //! \sa #AddMember(const String&)
    //! \sa #Contains(const String&) const
    //! \sa #SetMembers(const StringSetCi&)
    void EraseMember(const String& member) {
	members_.erase(member);
    }

    //! Gets the members.
    //! \sa #SetMembers(const StringSetCi&)
    StringSetCi GetMembers() const noexcept {
	return members_;
    }

    //! Reads this enumeration from a data node.
    //! \param data the data node to read
    //! \sa #WriteData(const DataPtr&) const
    void ReadData(const DataPtr& data) noexcept;

    //! Reads members from a data node.
    //! \param data the Members data node to read
    //! \sa #ReadData(const DataPtr&)
    //! \sa #WriteMembersData(const DataPtr&) const
    void ReadMembersData(const DataPtr& data) noexcept;

    //! Sets the members.
    //! \param members the members to set
    //! \sa #GetMembers() const
    void SetMembers(const StringSetCi& members) {
	members_ = members;
    }

    //! Writes this enumeration to a data node.
    //! \param data the data node to write
    //! \sa #ReadData(const DataPtr&)
    void WriteData(const DataPtr& data) const noexcept;

    //! Writes members to a data node.
    //! \param data the Members data node to write
    //! \sa #ReadMembersData(const DataPtr&)
    //! \sa #WriteData(const DataPtr&) const
    void WriteMembersData(const DataPtr& data) const noexcept;

protected:
    //! The members.
    //! \sa #GetMembers() const
    //! \sa #SetMembers(const StringSetCi&)
    StringSetCi members_;
};
//! \}

//! The type of a shared enumeration pointer.
using EnumerationPtr = std::shared_ptr<Enumeration>;

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_ENUMERATION_HPP_
