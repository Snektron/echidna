#include "server/job.hpp"

namespace echidna::server {
    Job::Job(const std::string& shader, uint32_t total_frames, uint32_t fps,
                uint32_t width, uint32_t height) : shader(shader), total_frames(total_frames),
                                                    fps(fps), width(width), height(height) {}

    bool Job::hasTasks() const {
        return this->frames_submitted == this->total_frames;
    }

    Task Job::getNextTask() {
        return Task(this->shader, this->frames_submitted++, this->fps, this->width, this->height);
    }

    void Job::frameRendered() {
        uint32_t total_rendered = ++this->frames_rendered;
        ((void)(total_rendered));

        if(this->frames_rendered == this->total_frames) {
            //TODO: trigger video callback
        }
    }
}