#ifndef _ECHIDNA_UTILS_ARGPARSE_HPP
#define _ECHIDNA_UTILS_ARGPARSE_HPP

#include <charconv>
#include <cstring>
#include <cstdint>

namespace echidna::utils {
    template <typename T>
    bool parseIntArg(const char* arg, T& value) {
        size_t len = std::strlen(arg);
        auto [end, err] = std::from_chars(arg, arg + len, value);
        if (err != std::errc() || end != arg + len) {
            return false;
        }

        return true;
    }

    bool parseDimArg(const char* arg, uint32_t& w, uint32_t& h);
}

#endif
