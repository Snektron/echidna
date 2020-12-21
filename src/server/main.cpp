#include "server/server.hpp"
#include "error/exception.hpp"
#include "utils/log.hpp"

#include <cstdlib>

namespace log = echidna::log;

int main() {
    log::LOGGER.addSink<log::ConsoleSink>();

    echidna::server::Server::Config server_config;
    server_config.client_port = 4242;
    server_config.cli_port = 4243;
    server_config.client_keepalive_timeout = 30;

    echidna::server::Server command_server(server_config);

    try {
        log::write("Starting server");
        command_server.run();

        log::write("Server active");
        command_server.join();
    }
    catch(const echidna::error::Exception& e) {
        log::write("Error: ", e.what());
        command_server.stop();
        command_server.join();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
