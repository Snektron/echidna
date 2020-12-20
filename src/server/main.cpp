#include <iostream>

#include "server/server.hpp"
#include "error/exception.hpp"

int main() {
    try {
        echidna::server::Server::Config server_config;
        server_config.client_port = 4242;
        server_config.cli_port = 4243;
        server_config.client_keepalive_timeout = 30;

        echidna::server::Server command_server(server_config);

        std::cout << "Starting server" << std::endl;

        command_server.run();

        std::cout << "Server active" << std::endl;
        command_server.join();
    }
    catch(const echidna::error::Exception& e) {
        std::cerr << "Program threw error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}