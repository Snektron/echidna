#ifndef _ECHIDNA_SERVER_CLIENTHANDLER_HPP
#define _ECHIDNA_SERVER_CLIENTHANDLER_HPP

#include "net/socket.hpp"

#include <memory>
#include <cstdint>

#include <thread>

namespace echidna::server {
    class ClientManager;

    class ClientHandler {
        private:
            std::unique_ptr<net::Socket> socket;
            ClientManager& manager;
            uint32_t client_id;

            std::thread active_thread;

            void handleClients();
            void handleKeepAlive();
        public:
            ClientHandler(std::unique_ptr<net::Socket>&&, ClientManager&, uint32_t);
            ~ClientHandler();

            void run();
            void join();
    };
}

#endif
