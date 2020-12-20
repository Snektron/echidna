#include "server/job.hpp"

namespace echidna::server {
    Job::Job(const std::string& shader, uint32_t total_frames, uint32_t fps,
                uint32_t width, uint32_t height) : shader(shader), total_frames(total_frames),
                                                    fps(fps), width(width), height(height),
                                                    frames_submitted(0), frames_rendered(0) {}

    bool Job::hasTasks() const {
        return this->frames_submitted < this->total_frames;
    }

    Task Job::getNextTask() {
        return Task(this->shader, this->frames_submitted++, this->fps, this->width, this->height);
    }
}