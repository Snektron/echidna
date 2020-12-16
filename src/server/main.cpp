#include <iostream>

#include "server/server.hpp"

int main() {
    echidna::server::Server::Config server_config;
    server_config.client_port = 4242;

    echidna::server::Server command_server(server_config);
    command_server.run();
    command_server.join();
    return 0;
}