#include <iostream>

#include "server/server.hpp"

int main() {
    echidna::server::Server::Config server_config;
    server_config.client_port = 4242;
    server_config.cli_port = 4243;
    server_config.client_keepalive_timeout = 30;

    echidna::server::Server command_server(server_config);
    command_server.run();
    command_server.join();
    return 0;
}