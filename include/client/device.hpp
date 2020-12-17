#ifndef _ECHIDNA_CLIENT_DEVICE_HPP
#define _ECHIDNA_CLIENT_DEVICE_HPP

#include "client/clutil.hpp"

#include <CL/cl.h>
#include <string>
#include <vector>

namespace echidna::client {
    struct Device {
        cl_device_id device_id;

        UniqueContext context;
        UniqueCommandQueue command_queue;

        Device(cl_device_id device_id);

        std::string name() const;

        // Return all opencl devices of type `device_mask`.
        static std::vector<cl_device_id> deviceIDs(cl_device_type device_mask = CL_DEVICE_TYPE_ALL);

        // Return all opencl devices of type `device_mask`, which are available and support at least OpenCL 1.2.
        static std::vector<cl_device_id> availableDeviceIDs(cl_device_type device_mask = CL_DEVICE_TYPE_ALL);
    };
}

#endif
