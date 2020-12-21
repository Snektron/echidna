#ifndef _ECHIDNA_SERVER_CLIMANAGER_HPP
#define _ECHIDNA_SERVER_CLIMANAGER_HPP

#include <thread>
#include <atomic>

#include "net/socket.hpp"
#include "net/serversocket.hpp"

namespace echidna::server {
    class JobQueue;

    //Class responsible for managing connections from command-line interfaces
    class CLIManager {
        private:
            //Server data
            net::ServerSocket socket;
            int server_port;
            JobQueue& job_queue;

            //Server thread and state
            std::thread server_thread;
            std::atomic<bool> active;

            //Main routine for the CLI processing server thread
            void serverThread();
            //Processes requests from a single socket
            void handleSocket(net::Socket*);

            //Handles an UPLOAD request
            void handleUpload(net::Socket*);
            //Handles a STATUS request
            void handleStatus(net::Socket*);
        public:
            CLIManager(int, JobQueue&);
            ~CLIManager();

            //Start/stop/join the server
            void run();
            void stop();
            void join();
    };
}

#endif
