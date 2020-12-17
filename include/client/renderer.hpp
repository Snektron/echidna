#ifndef _ECHIDNA_CLIENT_RENDERER_HPP
#define _ECHIDNA_CLIENT_RENDERER_HPP

#include "client/device.hpp"

#include <CL/cl.h>
#include <vector>

namespace echidna::client {
    class Renderer {
        private:
            std::vector<Device> devices;

            void addDevice(cl_device_id device_id);
        public:
            explicit Renderer();
    };
}

#endif
