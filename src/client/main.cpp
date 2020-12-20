#include <iostream>
#include <vector>
#include "CL/cl.h"
#include "client/clerror.hpp"
#include "client/renderer.hpp"
#include "client/device.hpp"
#include "client/client.hpp"
#include "client/renderqueue.hpp"
#include "utils/log.hpp"

namespace log = echidna::log;

int main(int argc, char* argv[]) {
    std::string hostname = "localhost";
    int port = 4242;

    try {
        echidna::client::RenderQueue render_queue;
        echidna::client::Client client(hostname, port, render_queue);

        log::LOGGER.addSink<log::ConsoleSink>();

        std::vector<uint32_t> timestamps;
        for (size_t i = 0; i < 1; ++i) {
            timestamps.push_back(i);
        }

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
        log::write(err.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
