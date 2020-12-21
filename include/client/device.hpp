#ifndef _ECHIDNA_CLIENT_DEVICE_HPP
#define _ECHIDNA_CLIENT_DEVICE_HPP

#include "client/clutil.hpp"

#include <CL/cl.h>
#include <string>
#include <vector>
#include <string_view>
#include <array>

namespace echidna::client {
    constexpr const size_t FRAMES = ECHIDNA_CONCURRENT_FRAMES; // set via meson

    struct Frame {
        UniqueEvent kernel_completed;
        UniqueEvent target_downloaded;
        bool ready;

        Frame(cl_context context);
    };

    struct Device {
        cl_device_id device_id;
        cl_device_type type_mask;

        UniqueContext context;
        UniqueCommandQueue command_queue;
        std::vector<Frame> frames;

        Device(cl_device_id device_id);

        std::string name() const;

        UniqueKernel buildKernelFromSource(std::string_view source, const char* kernel_image);

        UniqueMemObject create2DImage(uint32_t width, uint32_t height, cl_image_format format);

        // Return all opencl devices of type `device_mask`.
        static std::vector<cl_device_id> deviceIDs(cl_device_type device_mask = CL_DEVICE_TYPE_ALL);
    };
}

#endif
