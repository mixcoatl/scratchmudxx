//! \file storage_file_multi.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_STORAGE_FILE_MULTI_HPP_
#define _SCRATCH_STORAGE_FILE_MULTI_HPP_

#include <scratch/data.hpp>
#include <scratch/logger.hpp>
#include <scratch/scratch.hpp>
#include <scratch/storage.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Storage {

// ScratchMUD types.
using Data = Scratch::Utility::Data;

//! The multi-file storage class. \{
//! \tparam ThingT the domain thing type
//! \remark Stores each thing in its own file under type-scoped buckets.
//! \sa Repository
//! \sa Storage
template<typename ThingT>
class MultiFileStorage : public Storage<ThingT> {
public:
    using ThingPtr = typename Storage<ThingT>::ThingPtr;
    using Map = typename Storage<ThingT>::Map;

    //! Constructor.
    //! \param root the data directory
    //! \param label the singular type label
    //! \param ext the file extension, including the leading dot
    MultiFileStorage(String root, String label, String ext) noexcept :
	    Storage<ThingT>(std::move(label)),
	    ext_(std::move(ext)),
	    root_(std::move(root)) {
	// Nothing.
    }

    //! Destructor.
    virtual ~MultiFileStorage() noexcept {
	// Nothing.
    }

    //! Removes a thing.
    //! \param thingId the thing ID of the thing to remove
    //! \param remaining unused; accepted for uniformity with #FileStorage
    //! \sa #Write(const String&, const ThingPtr&, const Map&) const
    //! \sa #WriteIndex(const Map&) const
    void Erase(
	    const String& thingId,
	    const Map& /*remaining*/) const noexcept {
	const auto stem = Scratch::Algorithm::Strings::SanitizeCopy(thingId);
	const auto path = this->GetThingFilePath(stem);
	if (path.empty()) {
	    LOGGER_ASSERT() << "Invalid `thingId` String.";
	} else {
	    boost::system::error_code ec;
	    if (boost::filesystem::exists(path, ec)) {
		boost::filesystem::remove(path, ec);
		if (ec) {
		    LOGGER_STORAGE() << "Couldn't remove " << path << ": "
				     << ec.message() << ".";
		}
	    }
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
	const auto stem = Scratch::Algorithm::Strings::SanitizeCopy(thingId);
	const auto path = this->GetThingFilePath(stem);
	if (path.empty()) {
	    LOGGER_ASSERT() << "Invalid `thingId` String.";
	    return false;
	}
	return this->ReadThingFile(path, stem, out);
    }

    //! Reads the thing index.
    //! \param out the map to receive the loaded things; cleared on entry
    //! \return \c true if the thing index was successfully loaded
    //! \remark Succeeds with an empty collection when the type directory
    //!	    under \c root is missing.
    //! \sa #WriteIndex(const Map&) const
    bool ReadIndex(Map& out) noexcept {
	out.clear();

	const auto typeRoot = this->GetThingTypeRoot();
	boost::system::error_code ec;
	if (!boost::filesystem::exists(typeRoot, ec)) {
	    LOGGER_STORAGE() << "Missing " << typeRoot.string()
			     << "; starting empty repository.";
	    return true;
	}
	if (!boost::filesystem::is_directory(typeRoot, ec)) {
	    LOGGER_STORAGE() << "Couldn't load " << typeRoot.string()
			     << ": not a directory.";
	    return false;
	}

	std::size_t nThingBytes = 0;
	const auto nPrefixes =
		sizeof(DirectoryPrefixes) / sizeof(DirectoryPrefixes[0]);
	for (std::size_t i = 0; i < nPrefixes; ++i) {
	    const boost::filesystem::path bucket =
		    typeRoot / DirectoryPrefixes[i];
	    if (!boost::filesystem::is_directory(bucket, ec))
		continue;

	    boost::system::error_code scanEc;
	    for (boost::filesystem::directory_iterator it(bucket, scanEc), end;
		    !scanEc && it != end; it.increment(scanEc)) {
		const auto& filePath = it->path();
		if (!boost::filesystem::is_regular_file(filePath, ec))
		    continue;
		if (filePath.extension().string() != ext_)
		    continue;

		const auto stem = filePath.stem().string();
		if (stem.empty())
		    continue;

		ThingPtr thing;
		if (!this->ReadThingFile(filePath.string(), stem, thing))
		    return false;
		const auto fileBytes = boost::filesystem::file_size(filePath, ec);
		if (!ec)
		    nThingBytes += static_cast<std::size_t>(fileBytes);
		out[this->GetThingId(*thing)] = thing;
	    }
	    if (scanEc) {
		LOGGER_STORAGE() << "Couldn't scan " << bucket.string() << ": "
				 << scanEc.message() << ".";
		return false;
	    }
	}

	LOGGER_STORAGE() << "Loaded " << out.size() << " " << this->GetLabel()
			 << "(s), " << nThingBytes << " byte(s).";
	return true;
    }

    //! Writes a thing.
    //! \param thingId the thing ID of the thing to save
    //! \param thing the thing to save
    //! \param all unused; accepted for uniformity with #FileStorage
    //! \return \c true if the file was successfully written
    //! \sa #WriteIndex(const Map&) const
    //! \sa #Read(const String&, ThingPtr&)
    //! \sa #Erase(const String&, const Map&) const
    bool Write(
	    const String& thingId,
	    const ThingPtr& thing,
	    const Map& /*all*/) const noexcept {
	if (!thing) {
	    LOGGER_ASSERT() << "Invalid `thing` ThingPtr.";
	    return false;
	}

	const auto stem = Scratch::Algorithm::Strings::SanitizeCopy(thingId);
	const auto path = this->GetThingFilePath(stem);
	if (path.empty()) {
	    LOGGER_ASSERT() << "Invalid `thingId` String.";
	    return false;
	}

	boost::system::error_code ec;
	boost::filesystem::create_directories(
		boost::filesystem::path(path).parent_path(), ec);
	if (ec) {
	    LOGGER_STORAGE() << "Couldn't create directory for " << path << ": "
			     << ec.message() << ".";
	    return false;
	}

	auto node = std::make_shared<Data>();
	thing->WriteData(node);
	if (!node->SaveFile(path)) {
	    LOGGER_STORAGE() << "Couldn't save " << path << ".";
	    return false;
	}
	return true;
    }

    //! Writes the thing index.
    //! \param in the things to write
    //! \return \c true if the thing index was successfully saved
    //! \remark Removes orphan files under the type-scoped bucket directories.
    //! \sa #ReadIndex(Map&)
    bool WriteIndex(const Map& in) const noexcept {
	StringSetCi expectedStems;
	std::size_t nThings = 0;
	for (const auto& pair : in) {
	    if (!pair.second)
		continue;
	    if (!this->Write(pair.first, pair.second, in))
		return false;
	    ++nThings;
	    const auto stem = Scratch::Algorithm::Strings::SanitizeCopy(pair.first);
	    if (!stem.empty())
		expectedStems.insert(stem);
	}

	this->RemoveOrphans(expectedStems);

	LOGGER_STORAGE() << "Saved " << nThings << " " << this->GetLabel() << "(s).";
	return true;
    }

protected:
    //! The directory-prefix names used for bucket layout.
    //! \sa #GetThingDirectoryPrefix(const String&) const
    static const String DirectoryPrefixes[6];

    //! Gets the directory prefix for a sanitized stem.
    //! \param stem the sanitized lowercase filename stem
    //! \return one of #DirectoryPrefixes
    //! \sa #GetThingFilePath(const String&) const
    String GetThingDirectoryPrefix(const String& stem) const {
	if (stem.empty())
	    return "zzz";

	const char ch = stem[0];
	if (ch >= 'a' && ch <= 'e')
	    return "a_e";
	if (ch >= 'f' && ch <= 'j')
	    return "f_j";
	if (ch >= 'k' && ch <= 'o')
	    return "k_o";
	if (ch >= 'p' && ch <= 't')
	    return "p_t";
	if (ch >= 'u' && ch <= 'z')
	    return "u_z";
	return "zzz";
    }

    //! Gets the type-scoped directory under the data root.
    //! \return the type-scoped directory path
    //! \sa #GetThingFilePath(const String&) const
    boost::filesystem::path GetThingTypeRoot() const {
	return boost::filesystem::path(root_) / this->GetLabel();
    }

    //! Gets the on-disk path for a sanitized thing-ID stem.
    //! \param stem the sanitized lowercase filename stem
    //! \return the full path, or an empty string if \p stem or the label is empty
    //! \remark Layout is \c root/label/<bucket>/stem.ext so multiple
    //!	    #MultiFileStorage instances can share one data root.
    //! \sa #GetThingDirectoryPrefix(const String&) const
    //! \sa #GetThingTypeRoot() const
    String GetThingFilePath(const String& stem) const {
	if (stem.empty() || this->GetLabel().empty())
	    return String();

	auto path = this->GetThingTypeRoot();
	path /= this->GetThingDirectoryPrefix(stem);
	path /= stem + ext_;
	return path.string();
    }

    //! Reads a thing from a file.
    //! \param path the filename of the file to read
    //! \param fallbackId the sanitized stem used if the thing has no ID after ReadData
    //! \param out the thing pointer to receive the loaded thing
    //! \return \c true if the file was successfully loaded
    bool ReadThingFile(
	    const String& path,
	    const String& fallbackId,
	    ThingPtr& out) const noexcept {
	out.reset();

	auto node = std::make_shared<Data>();
	if (!node->LoadFile(path)) {
	    LOGGER_STORAGE() << "Couldn't load " << path << ".";
	    return false;
	}

	out = std::make_shared<ThingT>();
	out->ReadData(node);
	if (this->GetThingId(*out).empty())
	    this->SetThingId(*out, fallbackId);
	return true;
    }

    //! Removes orphan things.
    //! \param expectedStems the sanitized stems that should remain on disk
    //! \remark Logs and continues after a removal failure.
    void RemoveOrphans(const StringSetCi& expectedStems) const noexcept {
	const auto typeRoot = this->GetThingTypeRoot();
	boost::system::error_code ec;
	if (!boost::filesystem::is_directory(typeRoot, ec))
	    return;

	std::vector<boost::filesystem::path> orphans;
	const auto nPrefixes =
		sizeof(DirectoryPrefixes) / sizeof(DirectoryPrefixes[0]);
	for (std::size_t i = 0; i < nPrefixes; ++i) {
	    const boost::filesystem::path bucket =
		    typeRoot / DirectoryPrefixes[i];
	    if (!boost::filesystem::is_directory(bucket, ec))
		continue;

	    boost::system::error_code scanEc;
	    for (boost::filesystem::directory_iterator it(bucket, scanEc), end;
		    !scanEc && it != end; it.increment(scanEc)) {
		const auto& filePath = it->path();
		if (!boost::filesystem::is_regular_file(filePath, ec))
		    continue;
		if (filePath.extension().string() != ext_)
		    continue;

		const auto stem = filePath.stem().string();
		if (expectedStems.find(stem) != std::end(expectedStems))
		    continue;
		orphans.push_back(filePath);
	    }
	    if (scanEc) {
		LOGGER_STORAGE() << "Couldn't scan " << bucket.string() << ": "
				 << scanEc.message() << ".";
	    }
	}

	for (const auto& filePath : orphans) {
	    boost::filesystem::remove(filePath, ec);
	    if (ec) {
		LOGGER_STORAGE() << "Couldn't remove orphan " << filePath.string()
				 << ": " << ec.message() << ".";
	    }
	}
    }

    //! The thing file extension.
    //! \remark Includes the leading dot.
    String ext_;

    //! The data directory.
    //! \remark Typically \c "data"; type files live under \c root/label/.
    String root_;
};
//! \}

template<typename ThingT>
const String MultiFileStorage<ThingT>::DirectoryPrefixes[6] = {
    "a_e", "f_j", "k_o", "p_t", "u_z", "zzz"
};

}; // namespace Storage
}; // namespace Scratch

#endif // _SCRATCH_STORAGE_FILE_MULTI_HPP_
