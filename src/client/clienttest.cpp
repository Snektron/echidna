#include <iostream>
#include <thread>
#include <chrono>

#include "client/client.hpp"
#include "client/renderqueue.hpp"

int main() {
    echidna::client::RenderQueue render_queue;

    std::cout << "Creating client" << std::endl;
    echidna::client::Client client("localhost", 4242, render_queue);
    std::cout << "Starting client" << std::endl;
    client.start();
    std::cout << "Client active" << std::endl;

    while(true) {
        echidna::client::RenderTaskInfo render_info = render_queue.getTask([&] {
            client.requestMoreJobs();
        });

        std::cout << "Received render task: " << std::endl;
        std::cout << "    Shader: " << render_info.kernel_source << std::endl
                << "    Job id: " << render_info.job_id << std::endl
                << "    FPS: " << render_info.fps << std::endl
                << "    Image width: " << render_info.image_width << std::endl
                << "    Image height: " << render_info.image_height << std::endl;
        std::cout << "    Timestamps: ";
        for(uint32_t t : render_info.timestamps) {
            std::cout << t << ", ";
        }
        std::this_thread::sleep_for(std::chrono::microseconds(10 * render_info.timestamps.size()));
        client.updateServer(render_info.job_id, render_info.timestamps, 10);
        std::cout << std::endl;
    }
    return 0;
}