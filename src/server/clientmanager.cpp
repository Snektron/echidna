#include "server/clientmanager.hpp"
#include "server/clienthandler.hpp"

namespace echidna::server {
    ClientManager::ClientManager(int server_port) : server_port(server_port), active(false), client_id_offset(0) {
    }

    ClientManager::~ClientManager() {}

    void ClientManager::run() {
        this->server.bind("0.0.0.0", this->server_port);
        this->active = true;

        this->server_thread = std::thread(&ClientManager::handleClients, this);


    }

    void ClientManager::join() {
        this->server_thread.join();
    }


    void ClientManager::handleClient(std::unique_ptr<net::Socket> client) {
        uint32_t client_id = this->client_id_offset++;

        std::unique_lock(this->client_map_mutex);
        auto handler = new ClientHandler(std::move(client), *this, client_id);
        this->client_map[client_id] = handler;

        handler->run();
    }

    void ClientManager::handleClients() {
        while(this->active) {
            std::unique_ptr<net::Socket> client = this->server.accept();
            this->handleClient(std::move(client));
        }
    }

    void ClientManager::handleKeepAlive() {

    }
}