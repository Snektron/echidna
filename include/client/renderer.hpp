#ifndef _ECHIDNA_CLIENT_RENDERER_HPP
#define _ECHIDNA_CLIENT_RENDERER_HPP

#include "client/device.hpp"
#include "client/clutil.hpp"

#include <CL/cl.h>
#include <vector>
#include <array>
#include <cstdint>

namespace echidna::client {
    struct RenderTaskInfo {
        std::string kernel_source;
        std::vector<uint32_t> timestamps;
        uint32_t fps;
        uint32_t image_width;
        uint32_t image_height;
    };

    struct DeviceTaskInfo {
        Device* device;
        UniqueKernel kernel;
        std::vector<UniqueMemObject> render_targets;
    };

    struct RenderTask {
        RenderTaskInfo task_info;
        std::vector<DeviceTaskInfo> device_info;
    };

    class Renderer {
        private:
            constexpr static const cl_image_format RENDER_TARGET_FORMAT = {CL_RGBA, CL_UNORM_INT8};

            std::vector<Device> devices;

            void addDevice(cl_device_id device_id);
        public:
            explicit Renderer();
            ~Renderer();

            RenderTask createRenderTask(RenderTaskInfo task_info);

            void finishAll();
    };
}

#endif
