#include "server/server.hpp"

namespace echidna::server {
    Server::Server(const Config& config) : client_manager(config.client_port, config.client_keepalive_timeout, this->job_queue) {}

    Server::~Server() {}

    void Server::run() {
        this->client_manager.run();
    }

    void Server::join() {
        this->client_manager.join();
    }
}