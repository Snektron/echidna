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

    //Constants defining how performance is measured for a client
    const size_t MIN_JOB_CAPABILITY = 4;
    const size_t BASE_JOB_CAPABILITY = MIN_JOB_CAPABILITY;
    const size_t MAX_JOB_CAPABILITY = 2048;
    const auto ESTIMATED_JOB_TIMEOUT = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(2));

    //Responsible for managing a single client
    class ClientHandler {
        private:
            //Basic client properties
            std::unique_ptr<net::Socket> socket;
            ClientManager& manager;
            uint32_t client_id;

            //Job properties
            std::vector<Task> active_tasks;
            std::atomic<size_t> rendered_frames = 0;
            std::atomic<size_t> job_capability = BASE_JOB_CAPABILITY;

            //Threads and synchronisation
            std::thread active_thread;
            std::thread issue_thread;
            std::mutex send_mutex;
            std::mutex keepalive_mutex;
            std::mutex task_mutex;
            std::condition_variable keepalive_cond;
            std::condition_variable job_update_cond;

            //State variables
            std::atomic<bool> active;
            std::atomic<bool> keepalive;
            std::atomic<bool> new_jobs;
            std::atomic<bool> has_received;

            protocol::ClientPacketID last_packet;

            //Sends a request to the client, ensures the entire buffer is sent as one continous packet
            bool issueRequest(const void*, size_t);
            //Callback to handle client data
            void handleResponse();
            //Main routine for the job sending thread
            void handleIssue();
        public:
            ClientHandler(std::unique_ptr<net::Socket>&&, ClientManager&, uint32_t);
            ~ClientHandler();

            //Functions to start/stop/join the client
            void run();
            void join();
            void stop();

            //Sends a keepalive packet to the client, returns whether a keepalive was returned
            bool tryKeepAlive();

            //Returns the number of tasks the client can process in a reasonable timeframe
            size_t getJobCapability() const;
            //Schedules a set of tasks to run on the client
            bool submitTasks(const std::vector<Task>&);

            //Fetches the jobs the client is currently executing
            std::vector<Task> getJobs();
    };
}

#endif
