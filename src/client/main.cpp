#include "client/clerror.hpp"
#include "client/renderer.hpp"
#include "client/client.hpp"
#include "client/renderqueue.hpp"
#include "error/exception.hpp"
#include "utils/log.hpp"
#include "utils/argparse.hpp"

#include <iostream>
#include <vector>
#include <thread>
#include <string_view>

namespace log = echidna::log;

void print_help(const char* prog) {
    std::cerr << "Usage: " << prog << " <host> [options...]\n"
        << "options:\n"
        << "--port <port>       Port to connect to (default: 4242)\n"
        << "--threads <amt>     The number of threads to use for image compression\n"
        << "                    (default: hardware concurrency)\n";
}

int main(int argc, const char* argv[]) {
    const char* host_opt = nullptr;
    const char* port_opt = nullptr;
    const char* threads_opt = nullptr;

    for (int i = 1; i < argc; ++i) {
        auto option = std::string_view(argv[i]);

        const char** arg = nullptr;
        if (option == "--port")
            arg = &port_opt;
        else if (option == "--threads")
            arg = &threads_opt;
        else if (!host_opt) {
            host_opt = argv[i];
            continue;
        } else {
            std::cerr << "Error: Unknown option '" << option << "'" << std::endl;
            return EXIT_FAILURE;
        }

        ++i;
        if (i >= argc) {
            std::cerr << "Error: Expected argument to " << option  << std::endl;
            return EXIT_FAILURE;
        }

        *arg = argv[i];
    }

    if (!host_opt) {
        std::cerr << "Error: Missing required option <hostname>" << std::endl;
        return EXIT_FAILURE;
    }

    uint16_t port = 4242;
    if (port_opt && !echidna::utils::parseIntArg(port_opt, port)) {
        std::cerr << "Error: Expected 16-bit unsigned integer for --port" << std::endl;
        return EXIT_FAILURE;
    }

    size_t threads = std::thread::hardware_concurrency();
    if (threads_opt && !echidna::utils::parseIntArg(threads_opt, threads)) {
        std::cerr << "Error: Invalid value '" << threads_opt << "' for --threads" << std::endl;
        return EXIT_FAILURE;
    }

    log::LOGGER.addSink<log::ConsoleSink>();

    try {
        echidna::client::RenderQueue render_queue;
        echidna::client::Client client(host_opt, port, render_queue);
        client.start();

        auto renderer = echidna::client::Renderer(threads);

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
