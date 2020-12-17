#ifndef _ECHIDNA_SERVER_TASK_HPP
#define _ECHIDNA_SERVER_TASK_HPP

#include <string>
#include <cstdint>

namespace echidna::server {
    struct Task {
        const std::string shader;
        const uint32_t frame;
        const uint32_t fps;
        const uint32_t width;
        const uint32_t height;

        Task(const std::string&, uint32_t, uint32_t, uint32_t, uint32_t);
    };
}

#endif
