#ifndef _ECHIDNA_CLIENT_RENDERER_HPP
#define _ECHIDNA_CLIENT_RENDERER_HPP

#include "client/device.hpp"
#include "client/clutil.hpp"
#include "client/rendertask.hpp"

#include <CL/cl.h>
#include <vector>
#include <array>
#include <cstdint>

namespace echidna::client {
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

            void runUntilCompletion(const RenderTask& task);

            void finishAll();

        private:
            std::pair<size_t, size_t> schedule(const RenderTask& task);
            void launch(const RenderTask& task, size_t device_index, size_t image_index);
    };
}

#endif
