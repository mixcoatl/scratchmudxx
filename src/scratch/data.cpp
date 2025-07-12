//! \file data.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_DATA_CPP_

#include <scratch/data.hpp>
#include <scratch/logger.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

using namespace Scratch::Algorithm;

namespace Scratch {
namespace Utility {

//! Default constructor.
Data::Data() noexcept :
	std::enable_shared_from_this<Scratch::Utility::Data>(),
	entries_(),
	value_() {
    // Nothing.
}

//! Copy constructor.
Data::Data(const Data& other) noexcept :
	std::enable_shared_from_this<Scratch::Utility::Data>(),
	entries_(other.entries_),
	value_(other.value_) {
    // Nothing.
}

//! Destructor.
Data::~Data() noexcept {
    // Nothing.
}

//! Clears this data node.
void Data::Clear() noexcept {
    entries_.clear();
    value_.clear();
}

//! Searches for a data node.
//! \param key the key that identifies the data node
//! \return the data node indicated by the specified key or \c nullptr
DataPtr Data::Get(const String& key) noexcept {
    if (key.length() == 0) {
	return this->shared_from_this();
    } else {
	auto found = entries_.find(key);
	if (entries_.end() != found) {
	    return found->second;
	}
    }
    return nullptr;
}

//! Searches for a data node.
//! \param key the key that identifies the data node
//! \param format the sscanf-style format specifier
//! \return the number of values successfully scanned, or -1
int Data::GetFormatted(
	const String& key,
	const String& format, ...) noexcept {
    std::va_list args;
    va_start(args, format);
    const int result = this->GetFormattedVa(key, format, args);
    va_end(args);
    return result;
}

//! Searches for a data node.
//! \param key the key that identifies the data node
//! \param format the sscanf-style format specifier
//! \return the number of values successfully scanned, or -1
int Data::GetFormatted(
	const String& key,
	const char* format, ...) noexcept {
    std::va_list args;
    va_start(args, const_cast<char*>(format));
    const int result = this->GetFormattedVa(key, format, args);
    va_end(args);
    return result;
}

//! Searches for a data node.
//! \param key the key that identifies the data node
//! \param format the sscanf-style format specifier
//! \param args the variadic argument list
//! \return the number of values successfully scanned, or -1
int Data::GetFormattedVa(
	const String& key,
	const String& format, std::va_list args) noexcept {
    return this->GetFormattedVa(key, format.c_str(), args);
}

//! Searches for a data node.
//! \param key the key that identifies the data node
//! \param format the sscanf-style format specifier
//! \param args the variadic argument list
//! \return the number of values successfully scanned, or -1
int Data::GetFormattedVa(
	const String& key,
	const char* format, std::va_list args) noexcept {
    const auto found = this->Get(key);
    if (found && found->value_.length()) {
	return std::vsscanf(found->value_.c_str(), format, args);
    }
    return -1;
}

//! Searches for a data node.
//! \param key the key that identifies the data node
//! \param defaultValue the value to return if the specified key
//!     cannot be resolved, or the value of the data node indicated
//!     by the specified key cannot be converted to the desired type
//! \return the value of the data node indicated by the specified key,
//!     or the specified default value
double Data::GetNumber(
	const String& key,
	const double defaultValue) noexcept {
    auto result = defaultValue;
    return this->GetFormatted(key, " %lg ", &result) == 1 ? result : defaultValue;
}

//! Searches for a data node.
//! \param key the key that identifies the data node
//! \param defaultValue the value to return if the specified key
//!     cannot be resolved, or the value of the data node indicated
//!     by the specified key cannot be converted to the desired type
//! \return the value of the data node indicated by the specified key,
//!     or the specified default value
String Data::GetString(
	const String& key,
	const String& defaultValue) noexcept {
    auto const result = this->Get(key);
    return result && result->value_.length() ? result->value_ : defaultValue;
}

//! Searches for a data node.
//! \param key the key that identifies the data node
//! \param defaultValue the value to return if the specified key
//!     cannot be resolved, or the value of the data node indicated
//!     by the specified key cannot be converted to the desired type
//! \return the value of the data node indicated by the specified key,
//!     or the specified default value
std::time_t Data::GetTime(
	const String& key,
	const std::time_t defaultValue) noexcept {
    struct tm time;
    std::memset(&time, '\0', sizeof(time));

    // Scan time fields from value.
    auto const N = this->GetFormatted(key,
	" %d-%d-%d %d:%d:%d %d ",
	&time.tm_year, &time.tm_mon, &time.tm_mday,
	&time.tm_hour, &time.tm_min, &time.tm_sec,
	&time.tm_isdst);

    // Must provide 3, 6, or 7 values.
    if (N != 3 && N != 6 && N != 7)
	return defaultValue;

    time.tm_mon  -= 1;    // Month must be 0-11.
    time.tm_year -= 1900; // Year must be 114 for 2014.
    return std::mktime(&time);
}

//! Searches for a data node.
//! \param key the key that identifies the data node
//! \param defaultValue the value to return if the specified key
//!     cannot be resolved, or the value of the data node indicated
//!     by the specified key cannot be converted to the desired type
//! \return the value of the data node indicated by the specified key,
//!     or the specified default value
bool Data::GetYesNo(
	const String& key,
	const bool defaultValue) noexcept {
    // Search for string value.
    const auto found = this->GetString(key);

    if (found.length()) {
	// Check for number value.
	double value = 0.0;
	if (std::sscanf(found.c_str(), " %lg ", &value) == 1)
	    return /* NaN */ value == value && value != 0.0;

	// Check for 'Yes' values.
	if (!StringCompareCi(found, "Y") || !StringCompareCi(found, "Yes"))
	    return true;

	// Check for 'No' values.
	if (!StringCompareCi(found, "N") || !StringCompareCi(found, "No"))
	    return false;
    }
    return defaultValue;
}

//! Loads this data node.
//! \param fname the filename of the file to read
//! \return true if the file indicated by the specified filename was
//!     successfully loaded
bool Data::LoadFile(const String& fname) noexcept {
    try {
	std::ifstream inStream(fname);
	return this->LoadStream(inStream);
    } catch (const std::exception& ex) {
	LOGGER_STORAGE() << "Error loading file " << fname << ": " << ex.what() << ".";
	return false;
    }
}

//! Loads this data node.
//! \param inStream the stream to read
//! \return true if the specified stream was successfully loaded
bool Data::LoadStream(std::istream& inStream) noexcept {
    this->Clear();
    this->ReadStructure(inStream);
    return !inStream.fail();
}

//! Inserts or updates a data node.
//! \param key the key that identifies the data node
//! \param value the value of the data node identified by the specified key
//! \return the new or updated data node, or \c nullptr
DataPtr Data::Put(
	const String& key,
	const DataPtr value) noexcept {
    DataPtr result(nullptr);
    if (!key.length()) {
	LOGGER_ASSERT() << "Invalid `key` String.";
    } else if (!value) {
	LOGGER_ASSERT() << "Invalid `value` DataPtr.";
    } else {
	// Make key copy.
	auto realKey = key;

	// Compute real key from std::max(key) + 1.
	if (realKey.compare("%") == 0) {
	    std::uint32_t highest = 0;
	    for (const auto& pair: entries_) {
		std::uint32_t parsed = 0;
		if (std::sscanf(pair.first.c_str(), " %u ", &parsed) == 1) {
		    highest = std::max(highest, parsed);
		}
	    }
	    // Format key using preceding zeros.
	    std::ostringstream oss;
	    oss << std::setw(4) << std::setfill('0') << (highest + 1);
	    realKey = oss.str();
	}
	// Store value pointer.
	entries_[realKey] = value;
	value_.clear();
	result = value;
    }
    return result;
}

//! Inserts or updates a data node.
//! \param key the key that identifies the data node
//! \param format the sprintf-style format specifier
//! \return the new or updated data node, or \c nullptr
DataPtr Data::PutFormatted(
	const String& key,
	const String& format, ...) noexcept {
    std::va_list args;
    va_start(args, format);
    auto result = this->PutFormattedVa(key, format, args);
    va_end(args);
    return result;
}

//! Inserts or updates a data node.
//! \param key the key that identifies the data node
//! \param format the sprintf-style format specifier
//! \return the new or updated data node, or \c nullptr
DataPtr Data::PutFormatted(
	const String& key,
	const char* format, ...) noexcept {
    std::va_list args;
    va_start(args, format);
    auto result = this->PutFormattedVa(key, format, args);
    va_end(args);
    return result;
}

//! Inserts or updates a data node.
//! \param key the key that identifies the data node
//! \param format the sprintf-style format specifier
//! \param args the variadic argument list
//! \return the new or updated data node, or \c nullptr
DataPtr Data::PutFormattedVa(
	const String& key,
	const String& format, std::va_list args) noexcept {
    return this->PutFormattedVa(key, format.c_str(), args);
}

//! Inserts or updates a data node.
//! \param key the key that identifies the data node
//! \param format the sprintf-style format specifier
//! \param args the variadic argument list
//! \return the new or updated data node, or \c nullptr
DataPtr Data::PutFormattedVa(
	const String& key,
	const char* format, std::va_list args) noexcept {
  char messg[MaxString] = {'\0'};
  const int N = std::vsnprintf(messg, sizeof(messg), format, args);
  if (N >= 0 && static_cast<std::size_t>(N) <= sizeof(messg) - 1) {
    return this->PutString(key, messg);
  }
  return DataPtr();
}

//! Inserts or updates a data node.
//! \param key the key that identifies the data node
//! \param value the value of the data node identified by the specified key
//! \return the new or updated data node, or \c nullptr
DataPtr Data::PutNumber(
	const String& key,
	const double value) noexcept {
    return this->PutFormatted(key, "%lg", value);
}

//! Inserts or updates a data node.
//! \param key the key that identifies the data node
//! \param value the value of the data node identified by the specified key
//! \return the new or updated data node, or \c nullptr
DataPtr Data::PutString(
	const String& key,
	const String& value) noexcept {
    DataPtr result(nullptr);
    if (!key.length()) {
	result = this->shared_from_this();
    } else {
	result = std::make_shared<Data>();
	if (this->Put(key, result) != result)
	    result.reset();
    }
    if (result) {
	result->entries_.clear();
	result->value_ = value;
    }
    return result;
}

//! Inserts or updates a data node.
//! \param key the key that identifies the data node
//! \param value the value of the data node identified by the specified key
//! \return the new or updated data node, or \c nullptr
DataPtr Data::PutString(
	const String& key,
	const char* value) noexcept {
    return this->PutString(key, String(value && *value != '\0' ? value : ""));
}

//! Inserts or updates a data node.
//! \param key the key that identifies the data node
//! \param value the value of the data node identified by the specified key
//! \return the new or updated data node, or \c nullptr
DataPtr Data::PutTime(
	const String& key,
	const std::time_t value) noexcept {
    // Current time.
    struct tm time;
    if (localtime_r(&value, &time) != &time) {
	LOGGER_ASSERT() << "localtime_r() failed: errno=" << errno << ".";
	return DataPtr();
    }
    // Insert or update data node.
    return this->PutFormatted(key,
	"%-4.4d-%-2.2d-%-2.2d %-2.2d:%-2.2d:%-2.2d %d",
	time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
	time.tm_hour, time.tm_min, time.tm_sec,
	time.tm_isdst);
}

//! Inserts or updates a data node.
//! \param key the key that identifies the data node
//! \param value the value of the data node identified by the specified key
//! \return the new or updated data node, or \c nullptr
DataPtr Data::PutYesNo(
	const String& key,
	const bool value) noexcept {
    return this->PutString(key, value ? "Yes" : "No");
}


//! Reads a string from a stream.
//! \param inStream the stream from which to read
void Data::ReadString(std::istream& inStream) noexcept {
    std::ostringstream oss;
    while (inStream) {
	// Read string character.
	const int ch = inStream.get();

	// EOF and EOL not valid here.
	if (ch == EOF || ch == '\n') {
	    inStream.setstate(std::ios::failbit);
	// Special handling for tilde character.
	} else if (ch == '~') {
	    // Double tilde is escaped tilde.
	    if (inStream.peek() != '~') {
		value_ = oss.str();
		return;
	    }
	    inStream.ignore();
	}

	// Accumulate string character.
	if (inStream && ch != '\r')
	    oss << (char) ch;
    }
}

//! Reads a string block from a stream.
//! \param inStream the stream from which to read
void Data::ReadStringBlock(std::istream& inStream) noexcept {
    std::vector<String> lines;
    std::ostringstream oss;

    // Skip to end-of-line.
    inStream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    while (inStream) {
	// Read string character.
	const int ch = inStream.get();

	// EOF not valid here.
	if (ch == EOF) {
	    inStream.setstate(std::ios::failbit);
	// EOL enqueues accumulated line.
	} else if (ch == '\n') {
	    oss << '\r' << '\n';
	    lines.push_back(oss.str());
	    oss.str(""); // Reset.
	// Special handling for tilde character.
	} else if (ch == '~') {
	    if (inStream.peek() != '~') {
		if (oss.tellp() > 0)
		    lines.push_back(oss.str());
		break;
	    }
	    inStream.ignore();
	}

	// Omit new-line and carriage-return characters.
	if (inStream && !std::strchr("\r\n", ch))
	    oss << (char) ch;
    }

    // Error processing string block.
    if (!inStream)
	return;

    // Indicates no leading whitespace found.
    std::size_t howManySpaces = String::npos;

    // Count leading spaces.
    for (auto line: lines) {
	auto index = line.find_first_not_of("\r\n\t\r\x20");
	if (index != String::npos) {
	    howManySpaces = std::min(howManySpaces, index);
	} else {
	    howManySpaces = line.length();
	}
    }

    // No leading whitespace found.
    if (howManySpaces == String::npos)
	howManySpaces = 0;

    // Reconstruct string block.
    oss.str("");
    for (auto line: lines) {
	oss << line.substr(howManySpaces);
    }
    value_ = oss.str();
}

//! Reads a structure from a stream.
//! \param inStream the stream from which to read
void Data::ReadStructure(std::istream& inStream) noexcept {
    // Read until end-of-structure.
    while (inStream >> std::ws && inStream.peek() != '~') {
	// Read structure entry.
	this->ReadStructureEntry(inStream);
    }

    // Skip over end-of-structure marker.
    if (inStream && inStream.peek() == '~')
	inStream.ignore();
}

//! Reads a structure entry from a stream.
//! \param inStream the stream from which to read
void Data::ReadStructureEntry(std::istream& inStream) noexcept {
    // Read structure key.
    String entryKey;
    std::getline(inStream, entryKey, ':');

    // Read structure value.
    auto entryValue = std::make_shared<Data>();
    if (inStream && inStream.peek() == '-') {
	inStream.ignore();
	entryValue->ReadStringBlock(inStream);
    } else {
	// Skip leading whitespace.
	while (inStream && std::isspace(inStream.peek()) && inStream.peek() != '\n') {
	    inStream.ignore();
	}
	if (inStream && inStream.peek() == '\n') {
	    entryValue->ReadStructure(inStream);
	} else if (inStream) {
	    entryValue->ReadString(inStream);
	}
    }
    if (inStream)
	this->Put(entryKey, entryValue);
}

//! Removes a data node.
//! \param key the key that identifies the data node remove.
//! \return true if the data node indicated by the specified key
//!     was successfully removed
bool Data::Remove(const String& key) noexcept {
    return entries_.erase(key);
}

//! Saves this data node.
//! \param fname the filename of the file to write
//! \return true if the file indicated by the specified filename
//!     was successfully written
//! \sa #SaveStream(std::ostream&) const
bool Data::SaveFile(const String& fname) const noexcept {
    bool result = false;
    if (!fname.length()) {
	LOGGER_ASSERT() << "Invalid `fname` String.";
    } else {
	const auto tempfname = fname + ".tmp";
	try {
	    // Write output file.
	    std::ofstream outStream(tempfname);
	    result = this->SaveStream(outStream);
	    outStream.close();

	    // Rename temporary file.
	    if (result && boost::filesystem::exists(tempfname)) {
		boost::filesystem::rename(tempfname, fname);
	    }

	    // Remove temporary file if it still exists.
	    if (boost::filesystem::exists(tempfname)) {
		boost::filesystem::remove(tempfname);
	    }
	} catch (const std::exception& ex) {
	    LOGGER_STORAGE() << "Error saving file " << fname << ": " << ex.what() << ".";
	}
    }
    return result;
}

//! Saves this data node.
//! \param outStream the stream to which to write
//! \return true if the specified stream was successfully written
//! \sa #SaveFile(const String&) const
bool Data::SaveStream(std::ostream& outStream) const noexcept {
    this->WriteStructure(outStream);
    return !outStream.fail();
}

//! Writes a string to a stream.
//! \param outStream the stream to which to write
//! \param indent the indentation level
void Data::WriteString(
	std::ostream& outStream,
	const std::size_t indent) const noexcept {
    auto begin = value_.cbegin(), end = value_.cend();
    if (value_.find('\n') != String::npos) {
	outStream << std::setfill(' ') << std::setw(indent * 2) << "" << std::setw(0);
    } else {
	// Find first non-whitespace character.
	while (begin != end && std::isspace(*begin))
	    ++begin;
    }
    for (; outStream && begin != end; ++begin) {
	if (*begin != '\r')
	    outStream << *begin;
	if (*begin == '~')
	    outStream << *begin;
	if (*begin == '\n')
	    outStream << std::setfill(' ') << std::setw(indent * 2) << "" << std::setw(0);
    }
    outStream << '~' << std::endl;
}

//! Writes a structure to a stream.
//! \param outStream the stream to which to write
//! \param indent the indentation level
void Data::WriteStructure(
	std::ostream& outStream,
	const std::size_t indent) const noexcept {
    // Iterate structure entries.
    for (auto pair: entries_) {
	this->WriteStructureEntry(outStream, indent, pair);
    }
    // Closing tilde.
    outStream << std::setfill(' ') << std::setw((indent) * 2) << "" <<
		 std::setw(0) << '~' << std::endl;
}

//! Writes a structure entry to a stream.
//! \param outStream the stream to which to write
//! \param indent the indentation level
void Data::WriteStructureEntry(
	std::ostream& outStream,
	const std::size_t indent,
	const DataPtrMap::value_type& entry) const noexcept {
    // Write indent level.
    outStream << std::setfill(' ') << std::setw(indent * 2) << "" << std::setw(0);

    // Write structure entry key.
    for (auto const& ch: entry.first) {
	if (!std::isalnum(ch) && ch != '_' && ch != '$')
	    outStream.setstate(std::ios::failbit);
	outStream << ch;
    }

    // Write colon.
    outStream << ':';

    if (entry.second->Size()) {
	outStream << std::endl;
	entry.second->WriteStructure(outStream, indent + 1);
    } else if (entry.second->value_.find('\n') != String::npos) {
	outStream << '-' << std::endl;
        entry.second->WriteString(outStream, indent + 1);
    } else {
	outStream << ' ';
        entry.second->WriteString(outStream, indent + 1);
    }
}

}; // namespace Utility
}; // namespace Scratch
