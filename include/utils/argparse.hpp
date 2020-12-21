#ifndef _ECHIDNA_UTILS_ARGPARSE_HPP
#define _ECHIDNA_UTILS_ARGPARSE_HPP

#include <charconv>
#include <cstring>
#include <cstdint>

namespace echidna::utils {
    // Parse an int argument, returns false when its not in a valid format.
    template <typename T>
    bool parseIntArg(const char* arg, T& value) {
        size_t len = std::strlen(arg);
        auto [end, err] = std::from_chars(arg, arg + len, value);
        if (err != std::errc() || end != arg + len) {
            return false;
        }

        return true;
    }

    // Parse a <w>x<h> dimension. Returns false when `arg` is not in a valid format.
    bool parseDimArg(const char* arg, uint32_t& w, uint32_t& h);
}

#endif
