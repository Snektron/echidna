#ifndef _ECHIDNA_SERVER_SERVER_HPP
#define _ECHIDNA_SERVER_SERVER_HPP

#include "server/clientmanager.hpp"
#include "server/climanager.hpp"
#include "server/jobqueue.hpp"

namespace echidna::server {

    class Server {
        private:
            JobQueue job_queue;
            ClientManager client_manager;
            CLIManager cli_manager;
        public:
            struct Config {
                int client_port;
                int cli_port;
                size_t client_keepalive_timeout;
            };

            Server(const Config&);
            ~Server();

            void run();
            void join();
            void stop();
    };
}

#endif
