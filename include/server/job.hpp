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

            template <typename T>
            void frameRendered(T callback) {
                uint32_t total_rendered = ++this->frames_rendered;

                if(total_rendered == this->total_frames) {
                    callback();
                }
            }

            inline uint32_t getTotalFrames() const {
                return this->total_frames;
            }
            inline uint32_t getRenderedFrames() const {
                return this->frames_rendered;
            }
    };
}

#endif
