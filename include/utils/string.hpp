#ifndef _ECHIDNA_UTILS_STRING_HPP
#define _ECHIDNA_UTILS_STRING_HPP

#include <string>
#include <sstream>

namespace echidna::utils::string {
    template <typename... T>
    std::string make_string(const T&... args) {
        std::stringstream ss;
        (void) (ss << ... << args);
        return ss.str();
    }
}

#endif
