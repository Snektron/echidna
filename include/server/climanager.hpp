#ifndef _ECHIDNA_SERVER_CLIMANAGER_HPP
#define _ECHIDNA_SERVER_CLIMANAGER_HPP

#include <thread>
#include <atomic>

#include "net/socket.hpp"
#include "net/serversocket.hpp"

namespace echidna::server {
    class JobQueue;

    class CLIManager {
        private:
            net::ServerSocket socket;
            int server_port;
            JobQueue& job_queue;

            std::thread server_thread;
            std::atomic<bool> active;

            void serverThread();
            void handleSocket(net::Socket*);

            void handleUpload(net::Socket*);
            void handleStatus(net::Socket*);
        public:
            CLIManager(int, JobQueue&);
            ~CLIManager();

            void run();
            void stop();
            void join();
    };
}

#endif
