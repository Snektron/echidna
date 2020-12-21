#ifndef _ECHIDNA_UTILS_LOG_HPP
#define _ECHIDNA_UTILS_LOG_HPP

#include <vector>
#include <memory>
#include <utility>
#include <mutex>
#include <sstream>
#include <ctime>

namespace echidna::log {
    // Structure representing a device that can be logged to. This needs to be thread-safe.
    struct Sink {
        virtual ~Sink() = default;
        virtual void write(std::string_view line) = 0;
    };

    // A sink to log to the standard error stream.
    struct ConsoleSink final : public Sink {
        std::mutex mutex;

        ConsoleSink() = default;
        ~ConsoleSink() override = default;
        void write(std::string_view line) override;
    };

    // Structure representing a logger, which logs messages to an internal list of sinks.
    class Logger {
        std::vector<std::unique_ptr<Sink>> sinks;

        public:
            // Add a new sink to this logger.
            template <typename T, typename... Args>
            void addSink(Args&&... args);

            // Write a message to all sinks.
            template <typename... Args>
            void write(const Args&... args);
    };

    template <typename T, typename... Args>
    void Logger::addSink(Args&&... args) {
        this->sinks.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    template <typename... Args>
    void Logger::write(const Args&... args) {
        auto ss = std::stringstream();
        std::time_t t = std::time(nullptr);
        std::tm* local = std::localtime(&t);

        ss << "[" << local->tm_hour << ":" << local->tm_min << ":" << local->tm_sec << "] ";
        (void) (ss << ... << args);
        auto msg = ss.str();
        for (auto& sink : this->sinks) {
            sink->write(msg);
        }
    }

    // The global logger. This has by default no sinks attached.
    extern Logger LOGGER;

    // Write to the global logger.
    template <typename... Args>
    void write(const Args&... args) {
        LOGGER.write(args...);
    }
}

#endif
