#include <iostream>

#include "server/server.hpp"

int main() {
    echidna::server::Server command_server;
    command_server.run();
    command_server.join();
    return 0;
}