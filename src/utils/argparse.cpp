#include "utils/argparse.hpp"

namespace echidna::utils {
    bool parseDimArg(const char* arg, uint32_t& w, uint32_t& h) {
        const char* x = std::strchr(arg, 'x');
        if (!x)
            return false;

        auto [wend, werr] = std::from_chars(arg, x, w);
        if (werr != std::errc() || wend != x)
            return false;

        size_t len = std::strlen(arg);
        auto [hend, herr] = std::from_chars(x + 1, arg + len, h);
        if (herr != std::errc() || hend != arg + len)
            return false;

        return true;
    }
}
