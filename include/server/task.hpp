#ifndef _ECHIDNA_SERVER_TASK_HPP
#define _ECHIDNA_SERVER_TASK_HPP

#include <string>
#include <cstdint>

namespace echidna::server {
    //Contains the full description of the rendering of a single frame
    struct Task {
        std::string shader;
        uint32_t frame;
        uint32_t fps;
        uint32_t width;
        uint32_t height;
        uint32_t job;

        Task(const std::string&, uint32_t, uint32_t, uint32_t, uint32_t);
    };
}

#endif
