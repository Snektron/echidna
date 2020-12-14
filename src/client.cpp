#include <iostream>
#include <vector>
#include "CL/cl.h"

int main() {
    cl_uint entries;
    clGetPlatformIDs(0, nullptr, &entries);
    std::vector<cl_platform_id> platforms(entries);
    clGetPlatformIDs(entries, platforms.data(), nullptr);

    std::vector<char> name;
    for (auto platform_id : platforms) {
        size_t name_size;
        clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, 0, nullptr, &name_size);
        name.resize(name_size);
        clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, name_size, name.data(), nullptr);

        cl_uint num_devices;
        clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 0, nullptr, &num_devices);
        std::vector<cl_device_id> devices(num_devices);
        clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, num_devices, devices.data(), nullptr);

        std::cout.write(name.data(), name_size);
        std::cout << std::endl;

        for (auto device_id : devices) {
            clGetDeviceInfo(device_id, CL_DEVICE_NAME, 0, nullptr, &name_size);
            name.resize(name_size);
            clGetDeviceInfo(device_id, CL_DEVICE_NAME, name_size, name.data(), nullptr);

            std::cout << "    " << device_id << ": ";
            std::cout.write(name.data(), name_size);
            std::cout << std::endl;
        }
    }
}
