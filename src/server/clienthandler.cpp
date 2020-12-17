#include "server/clienthandler.hpp"
#include "server/clientmanager.hpp"

#include "error/network.hpp"

namespace echidna::server {
    ClientHandler::ClientHandler(std::unique_ptr<net::Socket>&& socket, ClientManager& manager, uint32_t client_id) : socket(std::move(socket)), manager(manager), client_id(client_id) {}
    ClientHandler::~ClientHandler() {}

    void ClientHandler::run() {
        this->active = true;
        this->active_thread = std::thread(&ClientHandler::handleResponse, this);
    }

    void ClientHandler::join() {
        this->active_thread.join();
    }

    void ClientHandler::stop() {
        this->active = false;

        this->socket->close();
    }

    bool ClientHandler::tryKeepAlive() {
        this->keepalive = false;

        protocol::ServerPacketID id = protocol::ServerPacketID::KEEPALIVE;
        if(!this->issueRequest(&id, sizeof(id)))
            return false;

        std::unique_lock keepalive_lock(this->keepalive_mutex);
        this->keepalive_cond.wait(keepalive_lock, [&] {return this->keepalive || !this->active; });
        return this->keepalive;
    }

    bool ClientHandler::issueRequest(const void* buffer, size_t buffer_size) {
        std::unique_lock(this->send_mutex);

        try {
            this->socket->sendFully(buffer, buffer_size);
            return true;
        }
        catch(const error::NetworkException& e) {
            return false;
        }
    }

    void ClientHandler::handleResponse() {
        try {
            while(this->active) {
                protocol::ClientPacketID id = this->socket->recv<protocol::ClientPacketID>();



                switch(id) {
                    case protocol::ClientPacketID::CONNECT:
                        break;
                    case protocol::ClientPacketID::KEEPALIVE: {
                            std::unique_lock(this->keepalive_mutex);
                            this->keepalive = true;
                            this->keepalive_cond.notify_all();
                        }
                        break;
                    case protocol::ClientPacketID::UPDATE_JOB:
                        //TODO
                        break;
                    case protocol::ClientPacketID::FINISH_JOB:
                        //TODO
                        break;
                }
            }
        }
        catch(const error::NetworkException& e) {

        }

        this->active = false;
        this->keepalive_cond.notify_all();

        this->stop();
    }

    size_t ClientHandler::getJobCapability() const {
        //TODO
        return 5;
    }

    void ClientHandler::submitTasks(const std::vector<Task>& tasks) {

    }
}