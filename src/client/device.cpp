#include "client/device.hpp"
#include "client/clerror.hpp"

#include <utility>
#include <algorithm>

namespace echidna::client {
    Device::Device(cl_device_id device_id):
        device_id(device_id) {
        cl_int status;
        this->context = clCreateContext(
            nullptr,
            1,
            &device_id,
            nullptr,
            nullptr,
            &status
        );
        check(status);

        this->command_queue = clCreateCommandQueue(this->context, this->device_id, 0, &status);
        check(status);
    }

    Device::~Device() {
        if (this->context) {
            check(clReleaseContext(this->context));
        }

        if (this->command_queue) {
            check(clReleaseCommandQueue(this->command_queue));
        }
    }

    Device::Device(Device&& other):
        device_id(std::exchange(other.device_id, nullptr)),
        context(std::exchange(other.context, nullptr)),
        command_queue(std::exchange(other.command_queue, nullptr)) {}

    Device& Device::operator=(Device&& other) {
        std::swap(this->device_id, other.device_id);
        std::swap(this->command_queue, other.command_queue);
        return *this;
    }

    std::string Device::name() const {
        size_t name_size;
        check(clGetDeviceInfo(device_id, CL_DEVICE_NAME, 0, nullptr, &name_size));
        std::string name(name_size, 0);
        check(clGetDeviceInfo(device_id, CL_DEVICE_NAME, name_size, name.data(), nullptr));
        return name;
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

    std::vector<cl_device_id> Device::availableDeviceIDs(cl_device_type mask) {
        auto devices = deviceIDs();
        auto end = std::copy_if(devices.begin(), devices.end(), devices.begin(), [](cl_device_id device_id) {
            cl_bool available;
            check(clGetDeviceInfo(device_id, CL_DEVICE_AVAILABLE, sizeof available, &available, nullptr));
            if (!available)
                return false;
            return true;
        });
        devices.resize(std::distance(devices.begin(), end));
        return devices;
    }
}
