#include "utils/log.hpp"
#include <iostream>

namespace echidna::log {
    Logger LOGGER;

    void ConsoleSink::write(std::string_view line) {
        std::lock_guard<std::mutex> lk(this->mutex);

        std::clog << line << std::endl;
    }
}
