//! \file thing.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_THING_HPP_
#define _SCRATCH_THING_HPP_

#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

// Forward declarations.
namespace Scratch {
namespace Utility {
class Data;
}; // namespace Utility
}; // namespace Scratch

namespace Scratch {
namespace Core {

// Forward declarations.
class Thing;

//! The type of a shared thing pointer.
using ThingPtr = std::shared_ptr<Thing>;

// ScratchMUD types.
using Data = Scratch::Utility::Data;
using DataPtr = std::shared_ptr<Data>;

//! The thing class. \{
class Thing : public std::enable_shared_from_this<Thing> {
public:
    //! Default constructor.
    Thing() noexcept;

    //! Copy constructor.
    //! \param other the \sa thing to copy
    Thing(const Thing& other) noexcept;

    //! Destructor.
    virtual ~Thing() noexcept;

    //! Default assignment.
    //! \param other the \sa thing to assign
    Thing& operator=(const Thing& other) noexcept;

    //! Gets the creation time.
    //! \sa #SetCreated(const std::time_t)
    std::time_t GetCreated() const noexcept {
	return created_;
    }

    //! Gets the creator identity.
    //! \sa #SetCreatedBy(const String&)
    String GetCreatedBy() const noexcept {
	return createdBy_;
    }

    //! Gets the modification time.
    //! \sa #SetModified(const std::time_t)
    std::time_t GetModified() const noexcept {
	return modified_;
    }

    //! Gets the last modifier identity.
    //! \sa #SetModifiedBy(const String&)
    String GetModifiedBy() const noexcept {
	return modifiedBy_;
    }

    //! Gets the thing name.
    //! \sa #SetName(const String&)
    virtual String GetName() const noexcept {
	return name_;
    }

    //! Reads metadata from a data node.
    //! \param data the Metadata data node to read
    //! \sa #WriteMetadataData(const DataPtr&) const
    void ReadMetadataData(const DataPtr& data) noexcept;

    //! Sets the creation time.
    //! \sa #GetCreated() const
    void SetCreated(const std::time_t created) noexcept {
	created_ = created;
    }

    //! Sets the creator identity.
    //! \sa #GetCreatedBy() const
    void SetCreatedBy(const String& createdBy) {
	createdBy_ = createdBy;
    }

    //! Sets the modification time.
    //! \sa #GetModified() const
    void SetModified(const std::time_t modified) noexcept {
	modified_ = modified;
    }

    //! Sets the last modifier identity.
    //! \sa #GetModifiedBy() const
    void SetModifiedBy(const String& modifiedBy) {
	modifiedBy_ = modifiedBy;
    }

    //! Sets the thing name.
    //! \sa #GetName() const
    void SetName(const String& name) {
	name_ = name;
    }

    //! Writes metadata to a data node.
    //! \param data the Metadata data node to write
    //! \sa #ReadMetadataData(const DataPtr&)
    void WriteMetadataData(const DataPtr& data) const noexcept;

protected:
    //! The creation time.
    //! \sa #GetCreated() const
    //! \sa #SetCreated(const std::time_t)
    std::time_t created_;

    //! The creator identity.
    //! \sa #GetCreatedBy() const
    //! \sa #SetCreatedBy(const String&)
    String createdBy_;

    //! The modification time.
    //! \sa #GetModified() const
    //! \sa #SetModified(const std::time_t)
    std::time_t modified_;

    //! The last modifier identity.
    //! \sa #GetModifiedBy() const
    //! \sa #SetModifiedBy(const String&)
    String modifiedBy_;

    //! The thing name.
    //! \sa #GetName() const
    //! \sa #SetName(const String&)
    String name_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_THING_HPP_
