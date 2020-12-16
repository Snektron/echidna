#include "server/clienthandler.hpp"
#include "server/clientmanager.hpp"

#include "error/network.hpp"

namespace echidna::server {
    ClientHandler::ClientHandler(std::unique_ptr<net::Socket>&& socket, ClientManager& manager, uint32_t client_id) : socket(std::move(socket)), manager(manager), client_id(client_id) {}
    ClientHandler::~ClientHandler() {}

    void ClientHandler::run() {
        this->active_thread = std::thread([&]() {
            try {
                //TODO: handle client
            }
            catch(const error::NetworkException& e) {

            }

            this->socket->close();
        });
    }

    void ClientHandler::join() {
        this->active_thread.join();
    }
}