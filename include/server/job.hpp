#ifndef _ECHIDNA_SERVER_JOB_HPP
#define _ECHIDNA_SERVER_JOB_HPP

#include <atomic>
#include <cstdint>
#include <string>

#include "server/task.hpp"

namespace echidna::server {
    class Job {
        private:
            std::string shader;
            uint32_t total_frames;
            uint32_t fps;
            uint32_t width;
            uint32_t height;

            uint32_t frames_submitted;
            std::atomic<uint32_t> frames_rendered;
        public:
            Job(const std::string&, uint32_t, uint32_t, uint32_t, uint32_t);

            bool hasTasks() const;
            Task getNextTask();
            void frameRendered();

    };
}

#endif
