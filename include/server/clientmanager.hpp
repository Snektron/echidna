#ifndef _ECHIDNA_SERVER_CLIENTMANAGER_HPP
#define _ECHIDNA_SERVER_CLIENTMANAGER_HPP

#include "net/socket.hpp"
#include "net/serversocket.hpp"

#include "server/jobqueue.hpp"

#include <thread>
#include <unordered_map>
#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <deque>
#include <condition_variable>

namespace echidna::server {
    class ClientHandler;

    //Manages a group of clients, responsible for task distribution and client keepalive
    class ClientManager {
        private:
            //Server properties
            net::ServerSocket server;
            int server_port;
            size_t keepalive_time;
            JobQueue& job_queue;

            //Threading and synchronization mechanisms
            std::thread server_thread;
            std::thread keepalive_thread;
            std::thread job_distribution_thread;
            std::atomic<bool> active;
            std::shared_mutex client_map_mutex;
            std::mutex free_client_mutex;
            std::condition_variable free_client_cond;

            //Client state
            std::atomic<uint32_t> client_id_offset;
            std::unordered_map<uint32_t, ClientHandler*> client_map;
            std::deque<uint32_t> free_clients;

            //Processes one new client, adds it to the client map
            void handleClient(std::unique_ptr<net::Socket>);
            //Main function of the thread accepting new client connections
            void handleClients();
            //Thread responsible for handling keepalive messages
            void handleKeepAlive();
            //Thread responsible for distributing jobs to clients
            void handleJobDistribution();
        public:
            ClientManager(int, size_t, JobQueue&);
            ~ClientManager();

            //Starts/joins/stops the server
            void run();
            void join();
            void stop();

            //Called to update the server that a new (job,frame) pair has finished
            void notifyUpdate(uint32_t, uint32_t);
            //Called to update the server that a client is ready to receive new jobs
            void notifyFinish(uint32_t);
            //Returns a set of jobs to the job queue, indicating failure
            void returnJobs(const std::vector<Task>&);
            //Removes a client from the set of all clients
            void removeClient(uint32_t);
    };
}

#endif
