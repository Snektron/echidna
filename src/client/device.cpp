#include "client/device.hpp"
#include "client/clerror.hpp"
#include "client/clutil.hpp"

#include <utility>
#include <algorithm>

namespace echidna::client {
    Frame::Frame(cl_context context):
        kernel_completed(createEvent(context, CL_COMPLETE)),
        target_downloaded(createEvent(context, CL_COMPLETE)),
        ready(std::make_unique<std::atomic_bool>(true)) {}

    Device::Device(cl_device_id device_id):
        device_id(device_id), type_mask(getDeviceType(device_id)),
        context(nullptr), command_queue(nullptr)
    {
        cl_int status;
        this->context = UniqueContext(clCreateContext(
            nullptr,
            1,
            &device_id,
            nullptr,
            nullptr,
            &status
        ));
        check(status);

        this->command_queue = UniqueCommandQueue(clCreateCommandQueue(this->context.get(), this->device_id, 0, &status));
        check(status);

        for (size_t i = 0; i < FRAMES; ++i) {
            this->frames.emplace_back(this->context.get());
        }
    }

    std::string Device::name() const {
        return getDeviceName(this->device_id);
    }

    UniqueKernel Device::buildKernelFromSource(std::string_view source, const char* kernel_name) {
        // Careful to catch errors caused by an invalid program and unrecoverable errors.

        cl_int status;
        size_t len = source.size();
        const char* source_data = source.data();
        auto program = UniqueProgram(clCreateProgramWithSource(
            this->context.get(),
            1,
            &source_data,
            &len,
            &status
        ));
        check(status);

        status = clBuildProgram(program.get(), 0, nullptr, nullptr, nullptr, nullptr);
        if (status == CL_BUILD_PROGRAM_FAILURE) {
            size_t size;
            check(clGetProgramBuildInfo(program.get(), this->device_id, CL_PROGRAM_BUILD_LOG, 0, nullptr, &size));
            std::string msg(size, 0);
            check(clGetProgramBuildInfo(program.get(), this->device_id, CL_PROGRAM_BUILD_LOG, size, msg.data(), nullptr));

            throw KernelCompilationException(msg);
        } else {
            check(status);
        }

        auto kernel = UniqueKernel(clCreateKernel(program.get(), kernel_name, &status));
        switch (status) {
            case CL_INVALID_KERNEL_NAME:
                throw KernelCompilationException("Program does not contain kernel '", kernel_name, "'");
            case CL_INVALID_KERNEL_DEFINITION:
                throw KernelCompilationException("Invalid defintion of kernel '", kernel_name, "'");
            default:
                check(status);
        }

        return kernel;
    }

    UniqueMemObject Device::create2DImage(uint32_t width, uint32_t height, cl_image_format format) {
        cl_int status;
        auto desc = cl_image_desc{
            CL_MEM_OBJECT_IMAGE2D,
            width,
            height,
            1,
            1,
            0,
            0,
            0,
            0,
            nullptr
        };

        auto image = UniqueMemObject(clCreateImage(
            this->context.get(),
            CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY,
            &format,
            &desc,
            nullptr,
            &status
        ));
        check(status);

        return image;
    }

    std::vector<cl_device_id> Device::deviceIDs(cl_device_type mask) {
        std::vector<cl_device_id> devices;

        cl_uint entries;
        check(clGetPlatformIDs(0, nullptr, &entries));
        std::vector<cl_platform_id> platforms(entries);
        check(clGetPlatformIDs(entries, platforms.data(), nullptr));

        for (auto platform_id : platforms) {
            cl_uint num_devices;
            check(clGetDeviceIDs(platform_id, mask, 0, nullptr, &num_devices));
            size_t orig_size = devices.size();
            devices.resize(orig_size + num_devices);
            check(clGetDeviceIDs(platform_id, mask, num_devices, devices.data() + orig_size, nullptr));
        }

        return devices;
    }
}
