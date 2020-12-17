#ifndef _ECHIDNA_SERVER_CLIENTHANDLER_HPP
#define _ECHIDNA_SERVER_CLIENTHANDLER_HPP

#include "net/socket.hpp"
#include "protocol/packet.hpp"

#include <memory>
#include <cstdint>

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace echidna::server {
    class ClientManager;

    class ClientHandler {
        private:
            std::unique_ptr<net::Socket> socket;
            ClientManager& manager;
            uint32_t client_id;

            std::thread active_thread;
            std::mutex send_mutex;
            std::mutex keepalive_mutex;
            std::condition_variable keepalive_cond;

            std::atomic<bool> active;
            std::atomic<bool> keepalive;

            protocol::ClientPacketID last_packet;


            bool issueRequest(const void*, size_t);
            void handleResponse();
        public:
            ClientHandler(std::unique_ptr<net::Socket>&&, ClientManager&, uint32_t);
            ~ClientHandler();

            void run();
            void join();
            void stop();

            bool tryKeepAlive();
    };
}

#endif
