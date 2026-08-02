//! \file storage_file.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_STORAGE_FILE_HPP_
#define _SCRATCH_STORAGE_FILE_HPP_

#include <scratch/data.hpp>
#include <scratch/logger.hpp>
#include <scratch/scratch.hpp>
#include <scratch/storage.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Storage {

// ScratchMUD types.
using Data = Scratch::Utility::Data;

//! The file storage class. \{
//! \tparam ThingT the domain thing type
//! \remark Stores the whole collection in a single flatfile.
//! \sa Repository
//! \sa Storage
template<typename ThingT>
class FileStorage : public Storage<ThingT> {
public:
    using ThingPtr = typename Storage<ThingT>::ThingPtr;
    using Map = typename Storage<ThingT>::Map;

    //! Constructor.
    //! \param root the data directory
    //! \param label the singular type label
    //! \param ext the file extension, including the leading dot
    FileStorage(String root, String label, String ext) noexcept :
	    Storage<ThingT>(std::move(label)),
	    path_((boost::filesystem::path(root) /
		    (this->GetLabel() + ext)).string()) {
	// Nothing.
    }

    //! Destructor.
    virtual ~FileStorage() noexcept {
	// Nothing.
    }

    //! Removes a thing.
    //! \param thingId the thing ID of the thing to remove
    //! \param remaining the in-memory collection after the erase
    //! \remark Rewrites the flatfile from \p remaining.
    //! \sa #WriteIndex(const Map&) const
    //! \sa #Write(const String&, const ThingPtr&, const Map&) const
    void Erase(
	    const String& thingId,
	    const Map& remaining) const noexcept {
	if (thingId.empty()) {
	    LOGGER_ASSERT() << "Invalid `thingId` String.";
	} else {
	    this->WriteIndex(remaining);
	}
    }

    //! Reads a thing.
    //! \param thingId the thing ID of the thing to load
    //! \param out the thing pointer to receive the loaded thing
    //! \return \c true if the thing was successfully loaded
    //! \sa #ReadIndex(Map&)
    //! \sa #Write(const String&, const ThingPtr&, const Map&) const
    bool Read(const String& thingId, ThingPtr& out) noexcept {
	out.reset();
	if (thingId.empty()) {
	    LOGGER_ASSERT() << "Invalid `thingId` String.";
	    return false;
	}

	Map all;
	if (!this->ReadIndex(all))
	    return false;

	auto found = all.find(thingId);
	if (found == std::end(all) || !found->second)
	    return false;

	out = found->second;
	return true;
    }

    //! Reads the thing index.
    //! \param out the map to receive the loaded things; cleared on entry
    //! \return \c true if the thing index was successfully loaded
    //! \remark Creates an empty file if missing.
    //! \sa #WriteIndex(const Map&) const
    bool ReadIndex(Map& out) noexcept {
	out.clear();

	boost::system::error_code ec;
	if (!boost::filesystem::exists(path_, ec)) {
	    LOGGER_STORAGE() << "Missing " << path_ << "; creating empty repository.";
	    return this->WriteIndex(out);
	}

	auto root = std::make_shared<Data>();
	if (!root->LoadFile(path_)) {
	    LOGGER_STORAGE() << "Couldn't load " << path_ << ".";
	    return false;
	}

	for (const auto& pair : root->GetEntries()) {
	    if (pair.first.empty() || !pair.second)
		continue;

	    auto thing = std::make_shared<ThingT>();
	    thing->ReadData(pair.second);
	    auto thingId = this->GetThingId(*thing);
	    if (thingId.empty()) {
		thingId = pair.first;
		this->SetThingId(*thing, thingId);
	    }
	    out[thingId] = thing;
	}

	const auto nThingBytes = boost::filesystem::file_size(path_, ec);
	LOGGER_STORAGE() << "Loaded " << (ec ? 0 : nThingBytes) << " byte(s) in "
			 << out.size() << " " << this->GetLabel() << "(s).";
	return true;
    }

    //! Writes a thing.
    //! \param thingId the thing ID of the thing to save
    //! \param thing the thing to save
    //! \param all the in-memory collection used to rewrite the flatfile
    //! \return \c true if the file was successfully written
    //! \sa #WriteIndex(const Map&) const
    //! \sa #Read(const String&, ThingPtr&)
    //! \sa #Erase(const String&, const Map&) const
    bool Write(
	    const String& thingId,
	    const ThingPtr& thing,
	    const Map& all) const noexcept {
	if (thingId.empty()) {
	    LOGGER_ASSERT() << "Invalid `thingId` String.";
	    return false;
	}
	if (!thing) {
	    LOGGER_ASSERT() << "Invalid `thing` ThingPtr.";
	    return false;
	}
	if (all.find(thingId) == std::end(all)) {
	    LOGGER_STORAGE() << thingId << " not in " << this->GetLabel()
			     << " repository.";
	    return false;
	}
	return this->WriteIndex(all);
    }

    //! Writes the thing index.
    //! \param in the things to write
    //! \return \c true if the thing index was successfully saved
    //! \sa #ReadIndex(Map&)
    bool WriteIndex(const Map& in) const noexcept {
	auto root = std::make_shared<Data>();
	std::size_t nThings = 0;
	for (const auto& pair : in) {
	    if (!pair.second)
		continue;

	    auto node = std::make_shared<Data>();
	    pair.second->WriteData(node);
	    if (!root->Put(pair.first, node)) {
		LOGGER_STORAGE() << "Couldn't store " << this->GetLabel() << " "
				 << pair.first << ".";
		return false;
	    }
	    ++nThings;
	}

	if (!root->SaveFile(path_)) {
	    LOGGER_STORAGE() << "Couldn't save " << path_ << ".";
	    return false;
	}

	LOGGER_STORAGE() << "Saved " << nThings << " " << this->GetLabel() << "(s).";
	return true;
    }

protected:
    //! The flatfile path.
    String path_;
};
//! \}

}; // namespace Storage
}; // namespace Scratch

#endif // _SCRATCH_STORAGE_FILE_HPP_
