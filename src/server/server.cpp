#include "server/server.hpp"

namespace echidna::server {
    Server::Server(const Config& config) :
        client_manager(config.client_port, config.client_keepalive_timeout, this->job_queue),
        cli_manager(config.cli_port, this->job_queue) {}

    Server::~Server() {}

    void Server::run() {
        this->client_manager.run();
        this->cli_manager.run();
    }

    void Server::join() {
        this->client_manager.join();
        this->cli_manager.join();
    }

    void Server::stop() {
        this->client_manager.stop();
        this->cli_manager.stop();
    }
}