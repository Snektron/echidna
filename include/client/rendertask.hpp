#ifndef _ECHIDNA_CLIENT_RENDERTASK_HPP
#define _ECHIDNA_CLIENT_RENDERTASK_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace echidna::client {
    // A structure representing information about a task to be rendered.
    struct RenderTaskInfo {
        std::string kernel_source;
        std::vector<uint32_t> timestamps;
        uint32_t job_id;
        uint32_t fps;
        uint32_t image_width;
        uint32_t image_height;
    };
}

#endif
