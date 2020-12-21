#ifndef _ECHIDNA_SERVER_CLIENTHANDLER_HPP
#define _ECHIDNA_SERVER_CLIENTHANDLER_HPP

#include "net/socket.hpp"
#include "protocol/packet.hpp"
#include "server/task.hpp"

#include <memory>
#include <cstdint>
#include <vector>

#include <atomic>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <chrono>

namespace echidna::server {
    class ClientManager;

    const size_t MIN_JOB_CAPABILITY = 4;
    const size_t BASE_JOB_CAPABILITY = MIN_JOB_CAPABILITY;
    const size_t MAX_JOB_CAPABILITY = 2048;
    const auto ESTIMATED_JOB_TIMEOUT = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(2));

    class ClientHandler {
        private:
            std::unique_ptr<net::Socket> socket;
            ClientManager& manager;
            uint32_t client_id;
            std::vector<Task> active_tasks;
            std::atomic<size_t> rendered_frames = 0;

            std::atomic<size_t> job_capability = BASE_JOB_CAPABILITY;

            std::thread active_thread;
            std::thread issue_thread;
            std::mutex send_mutex;
            std::mutex keepalive_mutex;
            std::mutex task_mutex;
            std::condition_variable keepalive_cond;
            std::condition_variable job_update_cond;

            std::atomic<bool> active;
            std::atomic<bool> keepalive;
            std::atomic<bool> new_jobs;
            std::atomic<bool> has_received;

            protocol::ClientPacketID last_packet;

            bool issueRequest(const void*, size_t);
            void handleResponse();
            void handleIssue();
        public:
            ClientHandler(std::unique_ptr<net::Socket>&&, ClientManager&, uint32_t);
            ~ClientHandler();

            void run();
            void join();
            void stop();

            bool tryKeepAlive();

            size_t getJobCapability() const;
            void submitTasks(const std::vector<Task>&);

            std::vector<Task> getJobs();
    };
}

#endif
