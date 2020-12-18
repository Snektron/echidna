#include <iostream>
#include <vector>
#include "CL/cl.h"
#include "client/clerror.hpp"
#include "client/renderer.hpp"
#include "client/device.hpp"
#include "utils/log.hpp"

namespace log = echidna::log;

int main() {
    log::LOGGER.addSink<log::ConsoleSink>();

    try {
        auto renderer = echidna::client::Renderer();
        // auto task = renderer.createRenderTask("__kernel void render(){}");
    } catch (const echidna::client::NoDeviceException& err) {
        log::write("Failed to initialize any OpenCL device");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
