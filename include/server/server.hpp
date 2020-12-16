#ifndef _ECHIDNA_SERVER_SERVER_HPP
#define _ECHIDNA_SERVER_SERVER_HPP

#include "server/clientmanager.hpp"

namespace echidna::server {

    class Server {
        private:
            ClientManager client_manager;
        public:
            struct Config {
                int client_port;
            };

            Server(const Config&);
            ~Server();

            void run();
            void join();
    };
}

#endif
