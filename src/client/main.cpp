#include "client/clerror.hpp"
#include "client/renderer.hpp"
#include "client/device.hpp"
#include "client/client.hpp"
#include "client/renderqueue.hpp"
#include "utils/log.hpp"
#include "error/exception.hpp"

#include <CL/cl.h>
#include <iostream>
#include <vector>
#include <charconv>
#include <cstring>

namespace log = echidna::log;

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <host ip> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    uint16_t port;
    size_t len = std::strlen(argv[2]);
    auto [end, err] = std::from_chars(argv[2], argv[2] + len, port);
    if (err != std::errc() || end != argv[2] + len) {
        std::cerr << "Error: Expected 16-bit unsigned integer port" << std::endl;
        return EXIT_FAILURE;
    }

    log::LOGGER.addSink<log::ConsoleSink>();

    try {
        echidna::client::RenderQueue render_queue;
        echidna::client::Client client(argv[1], port, render_queue);
        client.start();

        auto renderer = echidna::client::Renderer(8);

        while(true) {
            auto task_info = render_queue.getTask([&] {
                client.requestMoreJobs();
            });

            auto task = renderer.createRenderTask(task_info);

            auto avg_frame_time = renderer.runUntilCompletion(task);

            client.updateServer(task_info.job_id, task_info.timestamps, avg_frame_time);
        }
    } catch (const echidna::error::Exception& err) {
        log::write("Error: ", err.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
