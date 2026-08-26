//! \file repository.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_REPOSITORY_HPP_
#define _SCRATCH_REPOSITORY_HPP_

#include <scratch/data.hpp>
#include <scratch/logger.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>
#include <functional>

namespace Scratch {
namespace Storage {

// ScratchMUD types.
using Data = Scratch::Utility::Data;

//! The repository class. \{
//! \tparam ThingT the domain thing type
//! \tparam StorageT the storage type
//! \remark The in-memory map is a write-through cache; #Store, #Erase, and
//!	    #Clear update storage immediately. \c StorageT should expose
//!	    \c ThingPtr and \c Map nested types analogous to this class.
template<typename ThingT, typename StorageT>
class Repository {
public:
    //! The type of a shared thing pointer.
    using ThingPtr = typename StorageT::ThingPtr;

    //! The type of the in-memory thing map.
    using Map = typename StorageT::Map;

    //! Constructor.
    //! \param storage the storage backend
    explicit Repository(StorageT storage) noexcept :
	    storage_(std::move(storage)),
	    things_(),
	    changeHook_(),
	    reloadHook_() {
	// Nothing.
    }

    //! Sets the single-object change hook.
    //! \param hook the hook to invoke after a successful change
    void SetChangeHook(std::function<void(const String&)> hook) {
	changeHook_ = std::move(hook);
    }

    //! Sets the index reload hook.
    //! \param hook the hook to invoke after a successful index reload
    void SetReloadHook(std::function<void()> hook) {
	reloadHook_ = std::move(hook);
    }

    //! Clears the repository.
    //! \sa #SaveIndex() const
    void Clear() noexcept {
	things_.clear();
	storage_.WriteIndex(things_);
    }

    //! Removes a thing.
    //! \param thingId the thing ID of the thing to remove
    //! \return \c true if a thing was removed from memory
    //! \sa #Store(const String&, const ThingPtr&)
    bool Erase(const String& thingId) noexcept {
	if (things_.erase(thingId) == 0)
	    return false;
	storage_.Erase(thingId, things_);
	if (changeHook_)
	    changeHook_(thingId);
	return true;
    }

    //! Gets a thing.
    //! \param thingId the thing ID of the thing to return
    //! \return the thing indicated by the specified thing ID, or \c nullptr
    //! \sa #GetIds() const
    //! \sa #Store(const String&, const ThingPtr&)
    ThingPtr Get(const String& thingId) const noexcept {
	auto found = things_.find(thingId);
	return found != std::end(things_) ? found->second : nullptr;
    }

    //! Returns whether this repository owns \p thing.
    //! \param thing the thing to test
    bool Contains(const ThingPtr& thing) const noexcept {
	if (!thing)
	    return false;
	for (const auto& pair: things_) {
	    if (pair.second == thing)
		return true;
	}
	return false;
    }

    //! Gets the thing IDs.
    //! \sa #Get(const String&) const
    StringSetCi GetIds() const noexcept {
	StringSetCi ids;
	for (const auto& pair : things_) {
	    if (pair.second)
		ids.insert(pair.first);
	}
	return ids;
    }

    //! Returns whether \p thingId is a valid repository thing ID.
    //! \param thingId the candidate thing ID
    //! \sa Storage::IsValidThingId(const String&) const
    //! \sa #Store(const String&, const ThingPtr&)
    bool IsValidThingId(const String& thingId) const noexcept {
	return storage_.IsValidThingId(thingId);
    }

    //! Loads a thing.
    //! \param thingId the thing ID of the thing to load
    //! \return \c true if the thing was loaded
    //! \sa #LoadIndex()
    //! \sa #Save(const String&) const
    bool Load(const String& thingId) noexcept {
	ThingPtr thing;
	if (!storage_.Read(thingId, thing) || !thing)
	    return false;

	auto found = things_.find(thingId);
	if (found != std::end(things_) && found->second) {
	    this->CopyContents(thing, found->second);
	    storage_.SetThingId(*found->second, thingId);
	} else {
	    storage_.SetThingId(*thing, thingId);
	    things_[thingId] = std::move(thing);
	}
	if (changeHook_)
	    changeHook_(thingId);
	return true;
    }

    //! Loads the thing index.
    //! \return \c true if the thing index was successfully loaded
    //! \sa #Load(const String&)
    //! \sa #SaveIndex() const
    bool LoadIndex() noexcept {
	Map loaded;
	if (!storage_.ReadIndex(loaded))
	    return false;

	for (const auto& pair: loaded) {
	    auto found = things_.find(pair.first);
	    if (found != std::end(things_) && found->second) {
		this->CopyContents(pair.second, found->second);
		storage_.SetThingId(*found->second, pair.first);
	    } else {
		things_[pair.first] = pair.second;
	    }
	}
	for (const auto& id: this->GetIds()) {
	    if (loaded.find(id) == std::end(loaded))
		things_.erase(id);
	}
	if (reloadHook_)
	    reloadHook_();
	return true;
    }

    //! Saves a thing.
    //! \param thingId the thing ID of the thing to save
    //! \return \c true if the thing was successfully saved
    //! \sa #Load(const String&)
    //! \sa #SaveIndex() const
    bool Save(const String& thingId) const noexcept {
	auto thing = this->Get(thingId);
	if (!thing) {
	    LOGGER_STORAGE() << thingId << " not in " << storage_.GetLabel() << " repository.";
	    return false;
	}
	return storage_.Write(thingId, thing, things_);
    }

    //! Saves the thing index.
    //! \return \c true if the thing index was successfully saved
    //! \sa #LoadIndex()
    //! \sa #Save(const String&) const
    bool SaveIndex() const noexcept {
	return storage_.WriteIndex(things_);
    }

    //! Stores a thing.
    //! \param thingId the thing ID of the thing to store
    //! \param thing the thing to store
    //! \return \c true if the thing was stored
    //! \sa #IsValidThingId(const String&) const
    //! \sa #Erase(const String&)
    //! \sa #Get(const String&) const
    bool Store(const String& thingId, const ThingPtr& thing) noexcept {
	bool result = false;
	if (!thing) {
	    LOGGER_ASSERT() << "Invalid `thing` ThingPtr.";
	} else if (!this->IsValidThingId(thingId)) {
	    LOGGER_ASSERT() << "Invalid `thingId` String.";
	} else {
	    auto found = things_.find(thingId);
	    ThingPtr canonical;
	    if (found != std::end(things_) && found->second) {
		canonical = found->second;
		if (canonical.get() != thing.get())
		    this->CopyContents(thing, canonical);
	    } else {
		canonical = std::make_shared<ThingT>();
		this->CopyContents(thing, canonical);
		things_[thingId] = canonical;
	    }

	    storage_.SetThingId(*canonical, thingId);
	    result = storage_.Write(thingId, canonical, things_);
	    if (changeHook_)
		changeHook_(thingId);
	}
	return result;
    }

protected:
    //! Copies thing contents.
    //! \param thing the source thing
    //! \param into the destination thing
    void CopyContents(
	    const ThingPtr& thing,
	    const ThingPtr& into) const noexcept {
	auto node = std::make_shared<Data>();
	thing->WriteData(node);
	into->ReadData(node);
    }

    //! The storage backend.
    StorageT storage_;

    //! The things.
    //! \sa #Get(const String&) const
    //! \sa #GetIds() const
    Map things_;

    //! The single-object change hook.
    std::function<void(const String&)> changeHook_;

    //! The index reload hook.
    std::function<void()> reloadHook_;
};
//! \}

}; // namespace Storage
}; // namespace Scratch

#endif // _SCRATCH_REPOSITORY_HPP_
