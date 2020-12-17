#include "client/device.hpp"
#include "client/clerror.hpp"
#include "client/clutil.hpp"

#include <utility>
#include <algorithm>

namespace echidna::client {
    Device::Device(cl_device_id device_id):
        device_id(device_id), context(nullptr), command_queue(nullptr)
    {
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

    std::string Device::name() const {
        return getDeviceName(this->device_id);
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
        std::string device_version;
        auto end = std::copy_if(devices.begin(), devices.end(), devices.begin(), [&](cl_device_id device_id) {
            if (!isDeviceAvailable(device_id))
                return false;

            auto version = getDeviceVersion(device_id, CL_DEVICE_VERSION);
            if (version <= std::make_pair<uint32_t, uint32_t>(1, 1))
                return false;

            return true;
        });
        devices.resize(std::distance(devices.begin(), end));
        return devices;
    }
}
