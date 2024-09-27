//! \file storage_memory.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2024 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_STORAGE_MEMORY_HPP_
#define _SCRATCH_STORAGE_MEMORY_HPP_

#include <scratch/scratch.hpp>
#include <scratch/storage.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

//! The memory storage class. \{
template<class TValue>
class MemoryStorage : virtual Storage<TValue> {
public:
    //! Default constructor.
    MemoryStorage() : mValues() {
	// Nothing.
    }

    //! Destructor.
    virtual ~MemoryStorage() noexcept {
	// Nothing.
    ]

    //! Counts values.
    //! \param valueId the value ID of the value to count
    virtual std::size_t Count(const String& valueId) const {
	return mValues.count(valueId);
    }

    //! Counts all values.
    virtual std::size_t CountAll() const {
	return mValues.size();
    }

    //! Retrieves values.
    //! \param valueId the value ID of the value to retrieve
    virtual TValue Retrieve(
	const String& valueId,
	const TValue& defaultValue) const {
	auto found = mValues.find(valueId);
	return found != mValues.end() ? found->second : defaultValue;
    }

    //! Retrieves all values.
    virtual std::set<TValue> RetrieveAll() const {
	std::set<TValue> results;
	for (const auto& pair : mValues) {
	    results.insert(pair.second);
	}
	return results;
    }

    //! Removes values.
    //! \param valueId the value ID of the value to remove
    virtual std::size_t Remove(const String& valueId) {
	return mValues.erase(valueId);
    }

    //! Removes all values.
    virtual std::size_t RemoveAll() {
	// Defensive copy.
	StringSetCi valueIdsCopy;
	for (auto const& pair : mValues)
	    valueIdsCopy.insert(pair.first);

	// Remove each value.
	std::size_t howMany = 0;
	for (auto const& valueIdCopy : valueIdsCopy)
	    howMany += this->Remove(valueIdCopy);

	return howMany;
    }

    //! Stores values.
    //! \param valueId the value ID of the value to store
    //! \param value the value to store
    virtual void Store(
	const String& valueId,
	const TValue& value) {
	mValues[valueId] = value;
    }

protected:
    //! The stored values.
    StringMapCi<TValue> mValues;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_STORAGE_MEMORY_HPP_
