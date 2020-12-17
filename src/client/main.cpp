#include <iostream>
#include <vector>
#include "CL/cl.h"
#include "client/clerror.hpp"
#include "client/renderer.hpp"
#include "client/device.hpp"
#include "utils/log.hpp"

using echidna::client::Device;
namespace log = echidna::log;

int main() {
    log::LOGGER.addSink<echidna::log::ConsoleSink>();

    auto device_ids = Device::availableDeviceIDs();

    for (auto device_id : device_ids) {
        auto device = Device(device_id);
        log::write(device.name());
    }
}
