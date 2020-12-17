#include <iostream>
#include <vector>
#include "CL/cl.h"
#include "client/clerror.hpp"
#include "client/renderer.hpp"
#include "client/device.hpp"

using echidna::client::Device;

int main() {
    auto device_ids = Device::availableDeviceIDs();

    for (auto device_id : device_ids) {
        auto device = Device(device_id);
        std::cout << device.name() << std::endl;
    }
}
