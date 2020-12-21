#ifndef _ECHIDNA_CLIENT_DEVICE_HPP
#define _ECHIDNA_CLIENT_DEVICE_HPP

#include "client/clutil.hpp"

#include <CL/cl.h>
#include <string>
#include <vector>
#include <string_view>
#include <array>

namespace echidna::client {
    // The number of frame slots that can be rendered concurrently
    constexpr const size_t FRAMES = ECHIDNA_CONCURRENT_FRAMES; // set via meson

    // A structure representing all the information that can be re-used between rendering
    // different frames of different rendering tasks (of which there are FRAMES per device).
    struct Frame {
        // The event that is signalled after the kernel has finished rendering the frame
        UniqueEvent kernel_completed;

        // The event that is signalled after the resulting image has downloaded back to host
        // memory.
        UniqueEvent target_downloaded;

        // A value signalling that this frame is ready to render another image, and that the
        // associated host buffer of the previous image has been processed.
        bool ready;

        // Initialize a frame with two events (set to CL_COMPLETE) and ready set to true,
        Frame(cl_context context);
    };

    // A structure representing an OpenCL device, along with associated OpenCL context,
    // command queue, and frame slots.
    struct Device {
        // The OpenCL device handle.
        cl_device_id device_id;

        // The type of device - GPU, CPU, etc.
        cl_device_type type_mask;

        // The context required to perform operations with this device. Each device has its
        // own context, even if they are from the same platform.
        UniqueContext context;

        // The command queue that is used to issue commands to the device.
        UniqueCommandQueue command_queue;

        // Vector of FRAMES frame slots.
        std::vector<Frame> frames;

        // Initialize OpenCL device handle `device_id` to a Device.
        Device(cl_device_id device_id);

        // Retrieve the name of this device.
        std::string name() const;

        // Build an OpenCL kernel from a single source file. The main function is passed via `kernel_name`,
        // and is not allowed to be called 'main'.
        UniqueKernel buildKernelFromSource(std::string_view source, const char* kernel_name);

        // Create a 2-dimensional OpenCL image object associated to the device.
        UniqueMemObject create2DImage(uint32_t width, uint32_t height, cl_image_format format);

        // Return all opencl devices of type `device_mask`.
        static std::vector<cl_device_id> deviceIDs(cl_device_type device_mask = CL_DEVICE_TYPE_ALL);
    };
}

#endif
