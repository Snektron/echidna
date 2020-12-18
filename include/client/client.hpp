#ifndef _ECHIDNA_CLIENT_CLIENT_HPP
#define _ECHIDNA_CLIENT_CLIENT_HPP

#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstddef>
#include <cstdint>

#include "net/socket.hpp"

namespace echidna::client {
    class RenderQueue;

    class Client {
        private:
            net::Socket socket;
            RenderQueue& render_queue;

            std::thread recv_thread;
            std::mutex write_mutex;
            std::atomic<bool> active;

            void handleRecv();

            bool issueRequest(const void*, size_t);
        public:
            Client(const std::string&, int, RenderQueue&);
            ~Client();

            void start();
            void join();
            void stop();
    };
}

#endif
