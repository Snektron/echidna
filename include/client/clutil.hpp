#ifndef _ECHIDNA_CLIENT_CLUTIL_HPP
#define _ECHIDNA_CLIENT_CLUTIL_HPP

#include "client/clerror.hpp"

#include <CL/cl.h>
#include <string>
#include <string_view>
#include <utility>
#include <cstdint>

namespace echidna::client {
    // A RAII handle wrapping an OpenCL handle. This makes managing lifetime of OpenCL
    // objects easier and less error prone.
    template <typename T, cl_int (*deleter)(T)>
    struct UniqueCLHandle {
        T handle;

        explicit UniqueCLHandle(T handle): handle(handle) {}

        UniqueCLHandle(UniqueCLHandle&& other):
            handle(std::exchange(other.handle, nullptr)) {}

        UniqueCLHandle(const UniqueCLHandle&) = delete;

        UniqueCLHandle& operator=(UniqueCLHandle&& other) {
            std::swap(this->handle, other.handle);
            return *this;
        }

        UniqueCLHandle& operator=(const UniqueCLHandle&) = delete;

        ~UniqueCLHandle() {
            if (this->handle) {
                check(deleter(this->handle));
            }
        }

        T& get() {
            return this->handle;
        }

        const T& get() const {
            return this->handle;
        }
    };

    // A list of OpenCL objects that are wrapped as a RAII type.
    using UniqueContext = UniqueCLHandle<cl_context, clReleaseContext>;
    using UniqueCommandQueue = UniqueCLHandle<cl_command_queue, clReleaseCommandQueue>;
    using UniqueProgram = UniqueCLHandle<cl_program, clReleaseProgram>;
    using UniqueKernel = UniqueCLHandle<cl_kernel, clReleaseKernel>;
    using UniqueMemObject = UniqueCLHandle<cl_mem, clReleaseMemObject>;
    using UniqueEvent = UniqueCLHandle<cl_event, clReleaseEvent>;

    // Return a device's name as a string.
    std::string getDeviceName(cl_device_id device_id);

    // Check whether the OpenCL implementation returns a device as availble/
    bool isDeviceAvailable(cl_device_id device_id);

    // Get the [major, minor] version of a specific device field.
    std::pair<uint32_t, uint32_t> getDeviceVersion(cl_device_id device_id, cl_device_info version_info);

    // Get the type of an OpenCL deviec.
    cl_device_type getDeviceType(cl_device_id device_id);

    // Get the status of an OpenCL event.
    cl_int getEventExecutionStatus(cl_event event);

    // Create a new OpenCL event, with an initial status.
    UniqueEvent createEvent(cl_context context, cl_int initial_status);

    // Parse any of OpenCL's returned version strings to a [major, minor] version format.
    std::pair<uint32_t, uint32_t> parseVersion(std::string_view version_str);
}

#endif
