//! \file scratch.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2025 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_SCRATCH_HPP_
#define _SCRATCH_SCRATCH_HPP_

#include "../conf.hpp"

#ifdef HAVE_ALGORITHM
#include <algorithm>
#endif // HAVE_ALGORITHM

#ifdef HAVE_ANY
#include <any>
#endif // HAVE_ANY

#ifdef HAVE_ARPA_TELNET_H
#include <arpa/telnet.h>
#endif // HAVE_ARPA_TELNET_H

#ifdef HAVE_ATOMIC
#include <atomic>
#endif // HAVE_ATOMIC

#ifdef HAVE_BITSET
#include <bitset>
#endif // HAVE_BITSET

#ifdef HAVE_BOOST_ALGORITH_STRING_HPP
#include <boost/algorithm/string.hpp>
#endif // HAVE_BOOST_ALGORITH_STRING_HPP

#ifdef HAVE_BOOST_ASIO_HPP
#include <boost/asio.hpp>
#endif // HAVE_BOOST_ASIO_HPP

#ifdef HAVE_BOOST_CHRONO_HPP
#include <boost/chrono.hpp>
#endif // HAVE_BOOST_CHRONO_HPP

#ifdef HAVE_BOOST_FILESYSTEM_HPP
#include <boost/filesystem.hpp>
#endif // HAVE_BOOST_FILESYSTEM_HPP

#ifdef HAVE_BOOST_FILESYSTEM_FSTREAM_HPP
#include <boost/filesystem/fstream.hpp>
#endif // HAVE_BOOST_FILESYSTEM_FSTREAM_HPP

#ifdef HAVE_BOOST_LEXICAL_CAST_HPP
#include <boost/lexical_cast.hpp>
#endif // HAVE_BOOST_LEXICAL_CAST_HPP

#ifdef HAVE_BOOST_NONCOPYABLE_HPP
#include <boost/noncopyable.hpp>
#endif // HAVE_BOOST_NONCOPYABLE_HPP

#ifdef HAVE_BOOST_RANDOM_HPP
#include <boost/random.hpp>
#endif // HAVE_BOOST_RANDOM_HPP

#ifdef HAVE_BOOST_SYSTEM_ERROR_CODE_HPP
#include <boost/system/error_code.hpp>
#endif // HAVE_BOOST_SYSTEM_ERROR_CODE_HPP

#ifdef HAVE_CCTYPE
#include <cctype>
#endif // HAVE_CCTYPE

#ifdef HAVE_CERRNO
#include <cerrno>
#endif // HAVE_CERRNO

#ifdef HAVE_CMATH
#include <cmath>
#endif // HAVE_CMATH

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif // HAVE_CRYPT_H

#ifdef HAVE_CSIGNAL
#include <csignal>
#endif // HAVE_CSIGNAL

#ifdef HAVE_CSTDARG
#include <cstdarg>
#endif // HAVE_CSTDARG

#ifdef HAVE_CSTDBOOL
#include <cstdbool>
#endif // HAVE_CSTDBOOL

#ifdef HAVE_CSTDDEF
#include <cstddef>
#endif // HAVE_CSTDDEF

#ifdef HAVE_CSTRING
#include <cstring>
#endif // HAVE_CSTRING

#ifdef HAVE_CTIME
#include <ctime>
#endif // HAVE_CTIME

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif // HAVE_DLFCN_H

#ifdef HAVE_FSTREAM
#include <fstream>
#endif // HAVE_FSTREAM

#ifdef HAVE_FUNCTIONAL
#include <functional>
#endif // HAVE_FUNCTIONAL

#ifdef HAVE_IOMANIP
#include <iomanip>
#endif // HAVE_IOMANIP

#ifdef HAVE_IOSTREAM
#include <iostream>
#endif // HAVE_IOSTREAM

#ifdef HAVE_ITERATOR
#include <iterator>
#endif // HAVE_ITERATOR

#ifdef HAVE_LIMITS
#include <limits>
#endif // HAVE_LIMITS

#ifdef HAVE_LIST
#include <list>
#endif // HAVE_LIST

#ifdef HAVE_MAP
#include <map>
#endif // HAVE_MAP

#ifdef HAVE_MEMORY
#include <memory>
#endif // HAVE_MEMORY

#ifdef HAVE_OPTIONAL
#include <optional>
#endif // HAVE_OPTIONAL

#ifdef HAVE_REGEX
#include <regex>
#endif // HAVE_REGEX

#ifdef HAVE_SET
#include <set>
#endif // HAVE_SET

#ifdef HAVE_SSTREAM
#include <sstream>
#endif // HAVE_SSTREAM

#ifdef HAVE_STDEXCEPT
#include <stdexcept>
#endif // HAVE_STDEXCEPT

#ifdef HAVE_STRING
#include <string>
#endif // HAVE_STRING

#ifdef HAVE_STRING_VIEW
#include <string_view>
#endif // HAVE_STRING_VIEW

#ifdef HAVE_THREAD
#include <thread>
#endif // HAVE_THREAD

#ifdef HAVE_TYPE_TRAITS
#include <type_traits>
#endif // HAVE_TYPE_TRAITS

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif // HAVE_UNISTD_H

#ifdef HAVE_UTILITY
#include <utility>
#endif // HAVE_UTILITY

#ifdef HAVE_VARIANT
#include <variant>
#endif // HAVE_VARIANT

#ifdef HAVE_VECTOR
#include <vector>
#endif // HAVE_VECTOR

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H

namespace Scratch {
//! The maximum length of a user input buffer.
const std::size_t MaxInput = 256;

//! The maximum length of a static buffer.
const std::size_t MaxString = 8192;

//! The ScratchMUD string type.
using String = std::string;

//! The ScratchMUD string-view type.
using StringView = std::string_view;
}; // namespace Scratch

#endif // _SCRATCH_SCRATCH_HXX_
