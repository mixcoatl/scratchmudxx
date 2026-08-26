//! \file storage_null.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_STORAGE_NULL_HPP_
#define _SCRATCH_STORAGE_NULL_HPP_

#include <scratch/logger.hpp>
#include <scratch/scratch.hpp>
#include <scratch/storage.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Storage {

//! The null storage class. \{
//! \tparam ThingT the domain thing type
//! \sa Repository
//! \sa Storage
template<typename ThingT>
class NullStorage : public Storage<ThingT> {
public:
    using ThingPtr = typename Storage<ThingT>::ThingPtr;
    using Map = typename Storage<ThingT>::Map;

    //! Constructor.
    //! \param label the singular type label
    explicit NullStorage(String label) noexcept :
	    Storage<ThingT>(std::move(label)) {
	// Nothing.
    }

    //! Destructor.
    virtual ~NullStorage() noexcept {
	// Nothing.
    }

    //! Removes a thing.
    //! \param thingId the thing ID of the thing to remove
    //! \param remaining unused; accepted for uniformity with #FileStorage
    //! \remark No durable state to update.
    //! \sa #Write(const String&, const ThingPtr&, const Map&) const
    //! \sa #WriteIndex(const Map&) const
    void Erase(
	    const String& thingId,
	    const Map& /*remaining*/) const noexcept {
	if (thingId.empty()) {
	    LOGGER_ASSERT() << "Invalid `thingId` String.";
	}
    }

    //! Reads a thing.
    //! \param thingId the thing ID of the thing to load
    //! \param out the thing pointer to receive the loaded thing
    //! \return \c false; null storage has nothing to load
    //! \sa #ReadIndex(Map&)
    //! \sa #Write(const String&, const ThingPtr&, const Map&) const
    bool Read(const String& thingId, ThingPtr& out) noexcept {
	out.reset();
	if (thingId.empty()) {
	    LOGGER_ASSERT() << "Invalid `thingId` String.";
	    return false;
	}
	return false;
    }

    //! Reads the thing index.
    //! \param out the map to receive the loaded things; cleared on entry
    //! \return \c true with an empty collection
    //! \sa #WriteIndex(const Map&) const
    bool ReadIndex(Map& out) noexcept {
	out.clear();
	return true;
    }

    //! Writes a thing.
    //! \param thingId the thing ID of the thing to save
    //! \param thing the thing to save
    //! \param all unused; accepted for uniformity with #FileStorage
    //! \return \c true on success
    //! \remark No durable state to update.
    //! \sa #WriteIndex(const Map&) const
    //! \sa #Read(const String&, ThingPtr&)
    //! \sa #Erase(const String&, const Map&) const
    bool Write(
	    const String& thingId,
	    const ThingPtr& thing,
	    const Map& /*all*/) const noexcept {
	if (thingId.empty()) {
	    LOGGER_ASSERT() << "Invalid `thingId` String.";
	    return false;
	}
	if (!thing) {
	    LOGGER_ASSERT() << "Invalid `thing` ThingPtr.";
	    return false;
	}
	return true;
    }

    //! Writes the thing index.
    //! \param in unused; accepted for uniformity with #FileStorage
    //! \return \c true on success
    //! \remark No durable state to update.
    //! \sa #ReadIndex(Map&)
    bool WriteIndex(const Map& /*in*/) const noexcept {
	return true;
    }
};
//! \}

}; // namespace Storage
}; // namespace Scratch

#endif // _SCRATCH_STORAGE_NULL_HPP_
