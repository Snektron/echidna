#include <iostream>
#include <vector>
#include "CL/cl.h"
#include "client/clerror.hpp"
#include "client/renderer.hpp"
#include "client/device.hpp"
#include "utils/log.hpp"

namespace log = echidna::log;

auto kernel = R"(
kernel void render(write_only image2d_t target) {
    int2 coords = (int2)(get_global_id(0), get_global_id(1));
    write_imageui(target, coords, (uint4)(255, 0, 255, 255));
}
)";

int main() {
    log::LOGGER.addSink<log::ConsoleSink>();

    std::vector<uint32_t> timestamps;
    for (size_t i = 0; i < 10; ++i) {
        timestamps.push_back(i);
    }

    try {
        auto renderer = echidna::client::Renderer(4);
        auto task = renderer.createRenderTask({
            kernel,
            timestamps,
            0,
            60,
            1920,
            1080
        });

        renderer.runUntilCompletion(task);
    } catch (const echidna::client::NoDeviceException& err) {
        log::write("Failed to initialize any OpenCL device");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
