//! \file data.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_DATA_HPP_
#define _SCRATCH_DATA_HPP_

#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Utility {

// Forward type declarations.
class Data;

//! The type of a shared data entry pointer.
using DataPtr = std::shared_ptr<Data>;

//! The type of a data entry structure.
using DataPtrMap = StringMapCi<DataPtr>;

//! The data node class. \{
class Data : std::enable_shared_from_this<Data> {
public:
    //! Default constructor.
    Data() noexcept;

    //! Copy constructor.
    Data(const Data& other) noexcept;

    //! Destructor.
    virtual ~Data() noexcept;

    //! Clears this data node.
    void Clear() noexcept;

    //! Searches for a data node.
    //! \param key the key that identifies the data node
    //! \return the data node indicated by the specified key or \c nullptr
    DataPtr Get(const String& key) noexcept;

    //! Searches for a data node.
    //! \param key the key that identifies the data node
    //! \param format the sscanf-style format specifier
    //! \return the number of values successfully scanned, or -1
    int GetFormatted(
	const String& key,
	const String& format, ...) noexcept;

    //! Searches for a data node.
    //! \param key the key that identifies the data node
    //! \param format the sscanf-style format specifier
    //! \return the number of values successfully scanned, or -1
    int GetFormatted(
	const String& key,
	const char* format, ...) noexcept;

    //! Searches for a data node.
    //! \param key the key that identifies the data node
    //! \param defaultValue the value to return if the specified key
    //!     cannot be resolved, or the value of the data node indicated
    //!     by the specified key cannot be converted to the desired type
    //! \return the value of the data node indicated by the specified key,
    //!     or the specified default value
    double GetNumber(
	const String& key,
	const double defaultValue = 0.0) noexcept;

    //! Searches for a data node.
    //! \param key the key that identifies the data node
    //! \param defaultValue the value to return if the specified key
    //!     cannot be resolved, or the value of the data node indicated
    //!     by the specified key cannot be converted to the desired type
    //! \return the value of the data node indicated by the specified key,
    //!     or the specified default value
    String GetString(
	const String& key,
	const String& defaultValue = String()) noexcept;

    //! Searches for a data node.
    //! \param key the key that identifies the data node
    //! \param defaultValue the value to return if the specified key
    //!     cannot be resolved, or the value of the data node indicated
    //!     by the specified key cannot be converted to the desired type
    //! \return the value of the data node indicated by the specified key,
    //!     or the specified default value
    std::time_t GetTime(
	const String& key,
	const std::time_t defaultValue = 0) noexcept;

    //! Searches for a data node.
    //! \param key the key that identifies the data node
    //! \param defaultValue the value to return if the specified key
    //!     cannot be resolved, or the value of the data node indicated
    //!     by the specified key cannot be converted to the desired type
    //! \return the value of the data node indicated by the specified key,
    //!     or the specified default value
    bool GetYesNo(
	const String& key,
	const bool defaultValue = false) noexcept;

    //! Loads this data node.
    //! \param fname the filename of the file to read
    //! \return true if the file indicated by the specified filename was
    //!     successfully loaded
    bool LoadFile(const String& fname) noexcept;

    //! Loads this data node.
    //! \param inStream the stream to read
    //! \return true if the specified stream was successfully loaded
    bool LoadStream(std::istream& inStream) noexcept;

    //! Inserts or updates a data node.
    //! \param key the key that identifies the data node
    //! \param value the value of the data node identified by the specified key
    //! \return the new or updated data node, or \c nullptr
    DataPtr Put(
	const String& key,
	const DataPtr value = nullptr) noexcept;

    //! Inserts or updates a data node.
    //! \param key the key that identifies the data node
    //! \param format the sprintf-style format specifier
    //! \return the new or updated data node, or \c nullptr
    DataPtr PutFormatted(
	const String& key,
	const String& format, ...) noexcept;

    //! Inserts or updates a data node.
    //! \param key the key that identifies the data node
    //! \param format the sprintf-style format specifier
    //! \return the new or updated data node, or \c nullptr
    DataPtr PutFormatted(
	const String& key,
	const char* format, ...) noexcept;

    //! Inserts or updates a data node.
    //! \param key the key that identifies the data node
    //! \param value the value of the data node identified by the specified key
    //! \return the new or updated data node, or \c nullptr
    DataPtr PutNumber(
	const String& key,
	const double value) noexcept;

    //! Inserts or updates a data node.
    //! \param key the key that identifies the data node
    //! \param value the value of the data node identified by the specified key
    //! \return the new or updated data node, or \c nullptr
    DataPtr PutString(
	const String& key,
	const String& value) noexcept;

    //! Inserts or updates a data node.
    //! \param key the key that identifies the data node
    //! \param value the value of the data node identified by the specified key
    //! \return the new or updated data node, or \c nullptr
    DataPtr PutString(
	const String& key,
	const char* value) noexcept;

    //! Inserts or updates a data node.
    //! \param key the key that identifies the data node
    //! \param value the value of the data node identified by the specified key
    //! \return the new or updated data node, or \c nullptr
    DataPtr PutTime(
	const String& key,
	const std::time_t value) noexcept;

    //! Inserts or updates a data node.
    //! \param key the key that identifies the data node
    //! \param value the value of the data node identified by the specified key
    //! \return the new or updated data node, or \c nullptr
    DataPtr PutYesNo(
	const String& key,
	const bool value) noexcept;

    //! Removes a data node.
    //! \param key the key that identifies the data node to remove
    //! \return true if the data node indicated by the specified key
    //!     was successfully removed
    bool Remove(const String& key) noexcept;

    //! Saves this data node.
    //! \param fname the filename of the file to write
    //! \return true if the file indicated by the specified filename
    //!     was successfully written
    //! \sa #SaveStream(std::ostream&) const
    bool SaveFile(const String& fname) const noexcept;

    //! Saves this data node.
    //! \param outStream the stream to which to write
    //! \return true if the specified stream was successfully written
    //! \sa #SaveFile(const String&) const
    bool SaveStream(std::ostream& outStream) const noexcept;

    //! Returns the data done size.
    std::size_t Size() const noexcept {
	return entries_.size();
    }

protected:
    //! Searches for a data node.
    //! \param key the key that identifies the data node
    //! \param format the sscanf-style format specifier
    //! \param args the variadic argument list
    //! \return the number of values successfully scanned, or -1
    int GetFormattedVa(
	const String& key,
	const String& format, std::va_list args) noexcept;

    //! Searches for a data node.
    //! \param key the key that identifies the data node
    //! \param format the sscanf-style format specifier
    //! \param args the variadic argument list
    //! \return the number of values successfully scanned, or -1
    int GetFormattedVa(
	const String& key,
	const char* format, std::va_list args) noexcept;

    //! Inserts or updates a data node.
    //! \param key the key that identifies the data node
    //! \param format the sprintf-style format specifier
    //! \param args the variadic argument list
    //! \return the new or updated data node, or \c nullptr
    DataPtr PutFormattedVa(
	const String& key,
	const String& format, std::va_list args) noexcept;

    //! Inserts or updates a data node.
    //! \param key the key that identifies the data node
    //! \param format the sprintf-style format specifier
    //! \param args the variadic argument list
    //! \return the new or updated data node, or \c nullptr
    DataPtr PutFormattedVa(
	const String& key,
	const char* format, std::va_list args) noexcept;

    //! Reads a string from a stream.
    //! \param inStream the stream from which to read
    void ReadString(std::istream& inStream) noexcept;

    //! Reads a string block from a stream.
    //! \param inStream the stream from which to read
    void ReadStringBlock(std::istream& inStream) noexcept;

    //! Reads a structure from a stream.
    //! \param inStream the stream from which to read
    void ReadStructure(std::istream& inStream) noexcept;

    //! Reads a structure entry from a stream.
    //! \param inStream the stream from which to read
    void ReadStructureEntry(std::istream& inStream) noexcept;

    //! Writes a string to a stream.
    //! \param outStream the stream to which to write
    //! \param indent the indentation level
    void WriteString(
	std::ostream& outStream,
	const std::size_t indent) const noexcept;

    //! Writes a structure to a stream.
    //! \param outStream the stream to which to write
    //! \param indent the indentation level
    void WriteStructure(
	std::ostream& outStream,
	const std::size_t indent = 0) const noexcept;

    //! Writes a structure entry to a stream.
    //! \param outStream the stream to which to write
    //! \param indent the indentation level
    void WriteStructureEntry(
	std::ostream& outStream,
	const std::size_t indent,
	const DataPtrMap::value_type& entry) const noexcept;

    //! The structure value.
    DataPtrMap entries_;

    //! The scalar value.
    String value_;
};
//! \}

}; // namespace Utility
}; // namespace Scratch

#endif // _SCRATCH_DATA_HPP_
