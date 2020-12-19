#ifndef _ECHIDNA_CLIENT_CLUTIL_HPP
#define _ECHIDNA_CLIENT_CLUTIL_HPP

#include "client/clerror.hpp"

#include <CL/cl.h>
#include <string>
#include <string_view>
#include <utility>
#include <cstdint>

namespace echidna::client {
    std::string getDeviceName(cl_device_id device_id);

    bool isDeviceAvailable(cl_device_id device_id);

    std::pair<uint32_t, uint32_t> getDeviceVersion(cl_device_id device_id, cl_device_info version_info);

    cl_device_type getDeviceType(cl_device_id device_id);

    cl_int getEventExecutionStatus(cl_event event);

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

        operator T&() {
            return this->handle;
        }

        operator const T&() const {
            return this->handle;
        }
    };

    using UniqueContext = UniqueCLHandle<cl_context, clReleaseContext>;
    using UniqueCommandQueue = UniqueCLHandle<cl_command_queue, clReleaseCommandQueue>;
    using UniqueProgram = UniqueCLHandle<cl_program, clReleaseProgram>;
    using UniqueKernel = UniqueCLHandle<cl_kernel, clReleaseKernel>;
    using UniqueMemObject = UniqueCLHandle<cl_mem, clReleaseMemObject>;
    using UniqueEvent = UniqueCLHandle<cl_event, clReleaseEvent>;

    std::pair<uint32_t, uint32_t> parseVersion(std::string_view version_str);
}

#endif
