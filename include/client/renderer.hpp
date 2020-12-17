#ifndef _ECHIDNA_CLIENT_RENDERER_HPP
#define _ECHIDNA_CLIENT_RENDERER_HPP

#include "client/device.hpp"

#include <CL/cl.h>
#include <vector>

namespace echidna::client {
    class Renderer {
        private:
            std::vector<Device> devices;

        public:
            Renderer(std::vector<Device>&& devices);
    };
}

#endif
