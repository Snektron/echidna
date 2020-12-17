#ifndef _ECHIDNA_UTILS_LOG_HPP
#define _ECHIDNA_UTILS_LOG_HPP

#include <vector>
#include <memory>
#include <utility>
#include <mutex>

namespace echidna::log {
    struct Sink {
        virtual ~Sink() = default;
        virtual void write(std::string_view line) = 0;
    };

    struct ConsoleSink final : public Sink {
        ConsoleSink() = default;
        ~ConsoleSink() override = default;
        void write(std::string_view line) override;
    };

    class Logger {
        std::vector<std::unique_ptr<Sink>> sinks;

        public:
            template <typename T, typename... Args>
            void addSink(Args&&... args);

            void write(std::string_view line);
    };

    template <typename T, typename... Args>
    void Logger::addSink(Args&&... args) {
        this->sinks.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    extern Logger LOGGER;

    void write(std::string_view line);
}

#endif
