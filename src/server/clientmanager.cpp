#include "server/clientmanager.hpp"
#include "server/clienthandler.hpp"
#include "error/network.hpp"

#include <vector>
#include <chrono>

namespace echidna::server {
    ClientManager::ClientManager(int server_port, size_t keepalive_time) : server_port(server_port), keepalive_time(keepalive_time), active(false), client_id_offset(0) {
    }

    ClientManager::~ClientManager() {
        std::shared_lock(this->client_map_mutex);

        for(auto& it : this->client_map) {
            delete it.second;
        }
    }

    void ClientManager::run() {
        this->server.bind("0.0.0.0", this->server_port);
        this->active = true;

        this->server_thread = std::thread(&ClientManager::handleClients, this);
        this->keepalive_thread = std::thread(&ClientManager::handleKeepAlive, this);
    }

    void ClientManager::join() {
        this->server_thread.join();

        std::shared_lock(this->client_map_mutex);
        for(auto& it : this->client_map) {
            it.second->join();
        }
    }

    void ClientManager::stop() {
        this->active = false;
        this->server.close();

        std::shared_lock(this->client_map_mutex);
        for(auto& it : this->client_map) {
            it.second->stop();
        }
    }


    void ClientManager::handleClient(std::unique_ptr<net::Socket> client) {
        uint32_t client_id = this->client_id_offset++;

        std::unique_lock(this->client_map_mutex);
        auto handler = new ClientHandler(std::move(client), *this, client_id);
        this->client_map[client_id] = handler;

        handler->run();
    }

    void ClientManager::handleClients() {
        try {
            while(this->active) {
                std::unique_ptr<net::Socket> client = this->server.accept();
                this->handleClient(std::move(client));
            }
        }
        catch(const error::NetworkException& e) {}
        this->active = false;
    }

    void ClientManager::handleKeepAlive() {
        auto offset_time = std::chrono::steady_clock::now();

        while(this->active) {
            offset_time += std::chrono::seconds(this->keepalive_time);
            std::this_thread::sleep_until(offset_time);

            std::vector<std::pair<uint32_t, ClientHandler*>> handlers;

            {
                std::shared_lock(this->client_map_mutex);
                for(auto& it : this->client_map) {
                    handlers.push_back(it);
                }
            }

            for(auto& it : handlers) {
                if(!it.second->tryKeepAlive()) {
                    it.second->stop();
                    it.second->join();

                    std::unique_lock(this->client_map_mutex);
                    delete it.second;
                    this->client_map.erase(it.first);
                }
            }
        }
    }
}