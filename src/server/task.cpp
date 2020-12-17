#include "server/task.hpp"

namespace echidna::server {
    Task::Task(const std::string& shader, uint32_t frame, uint32_t fps, uint32_t width, uint32_t height)
                : shader(shader), frame(frame), fps(fps), width(width), height(height) {}

}