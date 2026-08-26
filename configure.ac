AC_INIT([scratchmud], [1.0], [bugs@scratchmud.org])
AC_CONFIG_SRCDIR([src/scratch/main.cpp])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([src/include/conf.hpp])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_PROG_AR
AC_PROG_CXX
AC_LANG([C++])
LT_INIT
AX_CXX_COMPILE_STDCXX([14])

dnl WARN_CXXFLAGS / WARN_LDFLAGS are consumed via AM_CXXFLAGS / AM_LDFLAGS.
dnl Pass --disable-Werror or --enable-compile-warnings=yes to soften -Werror.
AX_COMPILER_FLAGS()
AX_BOOST_BASE([1.64])
AX_BOOST_ASIO
AX_BOOST_CHRONO
AX_BOOST_FILESYSTEM
AX_BOOST_SYSTEM

dnl Lua 5.3 or 5.4 (required).
AM_CONDITIONAL([LUAJIT], [false])
AX_PROG_LUA([5.3], [5.5])
AX_LUA_HEADERS
AX_LUA_LIBS

dnl Temporarily add LUA_INCLUDE so AC_CHECK_HEADERS can find lua.hpp, then
dnl restore CPPFLAGS; the build gets LUA_INCLUDE via AM_CPPFLAGS instead.
save_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$CPPFLAGS $LUA_INCLUDE"
AC_CHECK_HEADERS([lua.hpp], [],
    [AC_MSG_ERROR([Lua C++ header lua.hpp not found])])
CPPFLAGS=$save_CPPFLAGS

dnl Probe headers used by the project; results drive the HAVE_* guards in scratch.hpp.
AC_CHECK_HEADER([algorithm], [AC_DEFINE([HAVE_ALGORITHM], [1], [Define to 1 if you have the <algorithm> header file.])])
AC_CHECK_HEADER([arpa/telnet.h], [AC_DEFINE([HAVE_ARPA_TELNET_H], [1], [Define to 1 if you have the <arpa/telnet.h> header file.])])
AC_CHECK_HEADER([atomic], [AC_DEFINE([HAVE_ATOMIC], [1], [Define to 1 if you have the <atomic> header file.])])
AC_CHECK_HEADER([bitset], [AC_DEFINE([HAVE_BITSET], [1], [Define to 1 if you have the <bitset> header file.])])
AC_CHECK_HEADER([boost/algorithm/string.hpp], [AC_DEFINE([HAVE_BOOST_ALGORITHM_STRING_HPP], [1], [Define to 1 if you have the <boost/algorithm/string.hpp> header file.])])
AC_CHECK_HEADER([boost/asio.hpp], [AC_DEFINE([HAVE_BOOST_ASIO_HPP], [1], [Define to 1 if you have the <boost/asio.hpp> header file.])])
AC_CHECK_HEADER([boost/chrono.hpp], [AC_DEFINE([HAVE_BOOST_CHRONO_HPP], [1], [Define to 1 if you have the <boost/chrono.hpp> header file.])])
AC_CHECK_HEADER([boost/endian/conversion.hpp], [AC_DEFINE([HAVE_BOOST_ENDIAN_CONVERSION_HPP], [1], [Define to 1 if you have the <boost/endian/conversion.hpp> header file.])])
AC_CHECK_HEADER([boost/filesystem.hpp], [AC_DEFINE([HAVE_BOOST_FILESYSTEM_HPP], [1], [Define to 1 if you have the <boost/filesystem.hpp> header file.])])
AC_CHECK_HEADER([boost/filesystem/fstream.hpp], [AC_DEFINE([HAVE_BOOST_FILESYSTEM_FSTREAM_HPP], [1], [Define to 1 if you have the <boost/filesystem/fstream.hpp> header file.])])
AC_CHECK_HEADER([boost/lexical_cast.hpp], [AC_DEFINE([HAVE_BOOST_LEXICAL_CAST_HPP], [1], [Define to 1 if you have the <boost/lexical_cast.hpp> header file.])])
AC_CHECK_HEADER([boost/random.hpp], [AC_DEFINE([HAVE_BOOST_RANDOM_HPP], [1], [Define to 1 if you have the <boost/random.hpp> header file.])])
AC_CHECK_HEADER([boost/system/error_code.hpp], [AC_DEFINE([HAVE_BOOST_SYSTEM_ERROR_CODE_HPP], [1], [Define to 1 if you have the <boost/system/error_code.hpp> header file.])])
AC_CHECK_HEADER([crypt.h], [AC_DEFINE([HAVE_CRYPT_H], [1], [Define to 1 if you have the <crypt.h> header file.])])
AC_CHECK_HEADER([cctype], [AC_DEFINE([HAVE_CCTYPE], [1], [Define to 1 if you have the <cctype> header file.])])
AC_CHECK_HEADER([cerrno], [AC_DEFINE([HAVE_CERRNO], [1], [Define to 1 if you have the <cerrno> header file.])])
AC_CHECK_HEADER([cmath], [AC_DEFINE([HAVE_CMATH], [1], [Define to 1 if you have the <cmath> header file.])])
AC_CHECK_HEADER([csignal], [AC_DEFINE([HAVE_CSIGNAL], [1], [Define to 1 if you have the <csignal> header file.])])
AC_CHECK_HEADER([cstdarg], [AC_DEFINE([HAVE_CSTDARG], [1], [Define to 1 if you have the <cstdarg> header file.])])
AC_CHECK_HEADER([cstdbool], [AC_DEFINE([HAVE_CSTDBOOL], [1], [Define to 1 if you have the <cstdbool> header file.])])
AC_CHECK_HEADER([cstddef], [AC_DEFINE([HAVE_CSTDDEF], [1], [Define to 1 if you have the <cstddef> header file.])])
AC_CHECK_HEADER([cstring], [AC_DEFINE([HAVE_CSTRING], [1], [Define to 1 if you have the <cstring> header file.])])
AC_CHECK_HEADER([ctime], [AC_DEFINE([HAVE_CTIME], [1], [Define to 1 if you have the <ctime> header file.])])
AC_CHECK_HEADER([dlfcn.h], [AC_DEFINE([HAVE_DLFCN_H], [1], [Define to 1 if you have the <dlfcn.h> header file.])])
AC_CHECK_HEADER([fstream], [AC_DEFINE([HAVE_FSTREAM], [1], [Define to 1 if you have the <fstream> header file.])])
AC_CHECK_HEADER([functional], [AC_DEFINE([HAVE_FUNCTIONAL], [1], [Define to 1 if you have the <functional> header file.])])
AC_CHECK_HEADER([iomanip], [AC_DEFINE([HAVE_IOMANIP], [1], [Define to 1 if you have the <iomanip> header file.])])
AC_CHECK_HEADER([iostream], [AC_DEFINE([HAVE_IOSTREAM], [1], [Define to 1 if you have the <iostream> header file.])])
AC_CHECK_HEADER([iterator], [AC_DEFINE([HAVE_ITERATOR], [1], [Define to 1 if you have the <iterator> header file.])])
AC_CHECK_HEADER([limits], [AC_DEFINE([HAVE_LIMITS], [1], [Define to 1 if you have the <limits> header file.])])
AC_CHECK_HEADER([list], [AC_DEFINE([HAVE_LIST], [1], [Define to 1 if you have the <list> header file.])])
AC_CHECK_HEADER([map], [AC_DEFINE([HAVE_MAP], [1], [Define to 1 if you have the <map> header file.])])
AC_CHECK_HEADER([memory], [AC_DEFINE([HAVE_MEMORY], [1], [Define to 1 if you have the <memory> header file.])])
AC_CHECK_HEADER([regex], [AC_DEFINE([HAVE_REGEX], [1], [Define to 1 if you have the <regex> header file.])])
AC_CHECK_HEADER([set], [AC_DEFINE([HAVE_SET], [1], [Define to 1 if you have the <set> header file.])])
AC_CHECK_HEADER([sstream], [AC_DEFINE([HAVE_SSTREAM], [1], [Define to 1 if you have the <sstream> header file.])])
AC_CHECK_HEADER([stdexcept], [AC_DEFINE([HAVE_STDEXCEPT], [1], [Define to 1 if you have the <stdexcept> header file.])])
AC_CHECK_HEADER([string], [AC_DEFINE([HAVE_STRING], [1], [Define to 1 if you have the <string> header file.])])
AC_CHECK_HEADER([type_traits], [AC_DEFINE([HAVE_TYPE_TRAITS], [1], [Define to 1 if you have the <type_traits> header file.])])
AC_CHECK_HEADER([unistd.h], [AC_DEFINE([HAVE_UNISTD_H], [1], [Define to 1 if you have the <unistd.h> header file.])])
AC_CHECK_HEADER([utility], [AC_DEFINE([HAVE_UTILITY], [1], [Define to 1 if you have the <utility> header file.])])
AC_CHECK_HEADER([vector], [AC_DEFINE([HAVE_VECTOR], [1], [Define to 1 if you have the <vector> header file.])])
AC_CHECK_HEADER([windows.h], [AC_DEFINE([HAVE_WINDOWS_H], [1], [Define to 1 if you have the <windows.h> header file.])])
AC_CHECK_LIB([crypt], [crypt])
AC_CHECK_LIB([dl], [dlsym])

AC_CONFIG_FILES([Makefile src/Makefile src/scratch/Makefile])
AC_OUTPUT
