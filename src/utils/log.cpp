#include "utils/log.hpp"
#include <iostream>

namespace echidna::log {
    Logger LOGGER;

    void ConsoleSink::write(std::string_view line) {
        std::clog << line << std::endl;
    }
}
