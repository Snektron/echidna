#ifndef _ECHIDNA_SERVER_SERVER_HPP
#define _ECHIDNA_SERVER_SERVER_HPP

#include "server/clientmanager.hpp"
#include "server/climanager.hpp"
#include "server/jobqueue.hpp"

namespace echidna::server {
    //Main server class, wraps all sub-servers and server-wide state
    class Server {
        private:
            JobQueue job_queue;
            ClientManager client_manager;
            CLIManager cli_manager;
        public:
            struct Config {
                int client_port; //Port to which the clients can connect
                int cli_port; //Port to which the CLI interfaces can connect
                size_t client_keepalive_timeout; //Keepalive interval
            };

            Server(const Config&);
            ~Server();

            void run();
            void join();
            void stop();
    };
}

#endif
