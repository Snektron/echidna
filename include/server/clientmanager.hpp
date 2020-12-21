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

    class ClientManager {
        private:
            net::ServerSocket server;
            int server_port;
            size_t keepalive_time;
            JobQueue& job_queue;

            std::thread server_thread;
            std::thread keepalive_thread;
            std::thread job_distribution_thread;
            std::atomic<bool> active;
            std::shared_mutex client_map_mutex;
            std::mutex free_client_mutex;
            std::condition_variable free_client_cond;

            std::atomic<uint32_t> client_id_offset;
            std::unordered_map<uint32_t, ClientHandler*> client_map;
            std::deque<uint32_t> free_clients;

            void handleClient(std::unique_ptr<net::Socket>);
            void handleClients();
            void handleKeepAlive();
            void handleJobDistribution();
        public:
            ClientManager(int, size_t, JobQueue&);
            ~ClientManager();

            void run();
            void join();
            void stop();

            void notifyUpdate(uint32_t, uint32_t);
            void notifyFinish(uint32_t);
            void returnJobs(const std::vector<Task>&);
            void removeClient(uint32_t);
    };
}

#endif
