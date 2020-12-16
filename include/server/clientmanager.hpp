#ifndef _ECHIDNA_SERVER_CLIENTMANAGER_HPP
#define _ECHIDNA_SERVER_CLIENTMANAGER_HPP

#include "net/socket.hpp"
#include "net/serversocket.hpp"

#include <thread>
#include <unordered_map>
#include <cstdint>
#include <shared_mutex>
#include <atomic>

namespace echidna::server {
    class ClientHandler;

    class ClientManager {
        private:
            net::ServerSocket server;
            int server_port;

            std::thread server_thread;
            std::atomic<bool> active;
            std::shared_mutex client_map_mutex;

            uint32_t client_id_offset;
            std::unordered_map<uint32_t, ClientHandler*> client_map;

            void handleClient(std::unique_ptr<net::Socket>);
            void handleClients();
            void handleKeepAlive();
        public:
            ClientManager(int);
            ~ClientManager();

            void run();
            void join();
    };
}

#endif
