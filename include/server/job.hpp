#ifndef _ECHIDNA_SERVER_JOB_HPP
#define _ECHIDNA_SERVER_JOB_HPP

#include <atomic>
#include <cstdint>
#include <string>

#include "server/task.hpp"

namespace echidna::server {
    //Contains information and state of a single job
    class Job {
        private:
            //Job description
            std::string shader;
            uint32_t total_frames;
            uint32_t fps;
            uint32_t width;
            uint32_t height;


            //The number of frames issued out to clients
            uint32_t frames_submitted;
            //The number of frames fully processed by clients
            std::atomic<uint32_t> frames_rendered;
        public:
            Job(const std::string&, uint32_t, uint32_t, uint32_t, uint32_t);

            //Checks if there are any more frames to process for this job
            bool hasTasks() const;
            //Returns a task describing the next frame to process
            Task getNextTask();

            //Callback to inform that a frame has been rendered, argument is called when all frames have finished
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
