#ifndef _ECHIDNA_UTILS_LOG_HPP
#define _ECHIDNA_UTILS_LOG_HPP

#include <vector>
#include <memory>
#include <utility>
#include <mutex>
#include <sstream>
#include <ctime>

namespace echidna::log {
    struct Sink {
        virtual ~Sink() = default;
        virtual void write(std::string_view line) = 0;
    };

    struct ConsoleSink final : public Sink {
        std::mutex mutex;

        ConsoleSink() = default;
        ~ConsoleSink() override = default;
        void write(std::string_view line) override;
    };

    class Logger {
        std::vector<std::unique_ptr<Sink>> sinks;

        public:
            template <typename T, typename... Args>
            void addSink(Args&&... args);

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

    extern Logger LOGGER;

    template <typename... Args>
    void write(const Args&... args) {
        LOGGER.write(args...);
    }
}

#endif
