/*
 *  Copyright (c), 2026, Alain Miniussi <alain.miniussi@oca.eu>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <string>

#if HIGHFIVE_USE_STRING_VIEW
#  ifndef __cpp_lib_string_view
#  error "std::string_view not available in this standard librarie
#  include <string_view>
#  endif
#endif

namespace HighFive {
/// \brief Build a path composed of all parameter separated with a "/".
/// \param s can be a std::string, a std::string_view (if supposrted) or a const char*.
/// \return The build path
inline std::string path(std::string s) { return s; }
/// \overload
template<typename... S> inline std::string path(std::string first, S... s);

#if HIGHFIVE_USE_STRING_VIEW
using H5Label = std::string_view;

/// \overload
inline std::string path(std::string_view s) { return std::string{s}; }
/// \overload
inline std::string path(char const* s) { return std::string{s}; }
/// \overload
template<typename... S> std::string path(std::string_view first, S... s);
template<typename... S> std::string path(char const* first, S... s);

#else // !HIGHFIVE_USE_STRING_VIEW

using H5Label = std::string;

#endif // HIGHFIVE_USE_STRING_VIEW

template<typename... S>
inline
std::string path(std::string first, S... s) {
    return first + "/" + path(s...);
}

#if HIGHFIVE_USE_STRING_VIEW

template<typename... S>
inline
std::string path(std::string_view first, S... s) {
    return std::string{first} + "/" + path(s...);
}

template<typename... S>
inline
std::string path(char const* first, S... s) {
    return std::string{first} + "/" + path(s...);
}

#endif
}
