#ifndef _ECHIDNA_CLIENT_DEVICE_HPP
#define _ECHIDNA_CLIENT_DEVICE_HPP

#include "client/clutil.hpp"

#include <CL/cl.h>
#include <string>
#include <vector>
#include <string_view>

namespace echidna::client {
    struct Device {
        cl_device_id device_id;

        UniqueContext context;
        UniqueCommandQueue command_queue;

        Device(cl_device_id device_id);

        std::string name() const;

        UniqueKernel buildKernelFromSource(std::string_view source, const char* kernel_image);

        // Return all opencl devices of type `device_mask`.
        static std::vector<cl_device_id> deviceIDs(cl_device_type device_mask = CL_DEVICE_TYPE_ALL);
    };
}

#endif
