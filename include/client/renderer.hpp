#ifndef _ECHIDNA_CLIENT_RENDERER_HPP
#define _ECHIDNA_CLIENT_RENDERER_HPP

#include "client/device.hpp"
#include "client/clutil.hpp"

#include <CL/cl.h>
#include <vector>

namespace echidna::client {
    struct DeviceTaskInfo {
        Device* device;
        UniqueKernel kernel;
    };

    struct RenderTask {
        std::vector<DeviceTaskInfo> device_info;
    };

    class Renderer {
        private:
            std::vector<Device> devices;

            void addDevice(cl_device_id device_id);
        public:
            explicit Renderer();

            RenderTask createRenderTask(std::string_view kernel);
    };
}

#endif
