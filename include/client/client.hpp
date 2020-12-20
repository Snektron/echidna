#ifndef _ECHIDNA_CLIENT_CLIENT_HPP
#define _ECHIDNA_CLIENT_CLIENT_HPP

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "net/socket.hpp"

namespace echidna::client {
    class RenderQueue;

    struct ClientPacket {
        uint32_t job_id;
        uint32_t frame_id;
    };

    class Client {
        private:
            net::Socket socket;
            RenderQueue& render_queue;

            std::thread recv_thread;
            std::thread send_thread;
            std::mutex write_mutex;
            std::mutex send_queue_mutex;
            std::condition_variable send_queue_cond;
            std::atomic<bool> active;
            std::vector<ClientPacket> send_queue;

            void handleRecv();
            void handleUpdate();

            bool issueRequest(const void*, size_t);
        public:
            Client(const std::string&, int, RenderQueue&);
            ~Client();

            void start();
            void join();
            void stop();

            void updateServer(uint32_t, const std::vector<uint32_t>&);
            void requestMoreJobs();
    };
}

#endif
