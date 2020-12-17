#include "utils/log.hpp"
#include <iostream>
#include <sstream>

namespace echidna::log {
    Logger LOGGER;

    void ConsoleSink::write(std::string_view line) {
        std::clog << line << std::endl;
    }

    void Logger::write(std::string_view line) {
        auto ss = std::stringstream();
        std::time_t t = std::time(nullptr);
        std::tm* local = std::localtime(&t);

        ss << "[" << local->tm_hour << ":" << local->tm_min << ":" << local->tm_sec << "] "
            << line;
        auto msg = ss.str();

        for (auto& sink : this->sinks) {
            sink->write(msg);
        }
    }

    void write(std::string_view line) {
        LOGGER.write(line);
    }
}
