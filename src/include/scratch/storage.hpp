//! \file storage.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2024 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_STORAGE_HPP_
#define _SCRATCH_STORAGE_HPP_

#include <scratch/scratch.hpp>

namespace Scratch {
namespace Core {

//! The storage class. \{
template<class TValue>
class Storage {
public:
    //! Counts values.
    //! \param valueId the value ID of the value to count
    virtual std::size_t Count(const String& valueId) const = 0;

    //! Counts all values.
    virtual std::size_t CountAll() const = 0;

    //! Retrieves values.
    //! \param valueId the value ID of the value to retrieve
    virtual TValue Retrieve(
	const String& valueId,
	const TValue& defaultValue) const = 0;

    //! Retrieves all values.
    virtual std::set<TValue> RetrieveAll() const = 0;

    //! Removes values.
    //! \param valueId the value ID of the value to remove
    virtual std::size_t Remove(const String& valueId) = 0;

    //! Removes all values.
    virtual std::size_t RemoveAll() = 0;

    //! Stores value.
    //! \param valueId the value ID of the value to store
    //! \param value the value to store
    virtual void Store(
	const String& valueId,
	const TValue& value) = 0;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_STORAGE_HPP_
