#ifndef _ECHIDNA_CLIENT_RENDERER_HPP
#define _ECHIDNA_CLIENT_RENDERER_HPP

#include "client/device.hpp"
#include "client/clutil.hpp"
#include "client/rendertask.hpp"

#include <CL/cl.h>
#include <vector>
#include <array>
#include <condition_variable>
#include <cstdint>

namespace echidna::client {
    struct DeviceTaskInfo {
        Device* device;
        UniqueKernel kernel;
        std::vector<UniqueMemObject> render_targets;
        std::vector<std::vector<uint32_t>> host_render_targets;
    };

    struct RenderTask {
        RenderTaskInfo task_info;
        std::vector<DeviceTaskInfo> device_info;
    };

    class Renderer {
        private:
            constexpr static const cl_image_format RENDER_TARGET_FORMAT = {CL_RGBA, CL_UNORM_INT8};

            std::vector<Device> devices;
            std::mutex mutex;
            std::condition_variable;

            void addDevice(cl_device_id device_id);
        public:
            explicit Renderer();
            ~Renderer();

            RenderTask createRenderTask(RenderTaskInfo task_info);

            void runUntilCompletion(RenderTask& task);

            void finishAll();

        private:
            std::pair<size_t, size_t> schedule(RenderTask& task);

            void launch(RenderTask& task, size_t device_index, size_t frame_index);

            static void targetDownloaded(cl_event event, cl_int status, void* user_data);
    };
}

#endif
