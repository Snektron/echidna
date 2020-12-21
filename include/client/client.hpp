#ifndef _ECHIDNA_CLIENT_CLIENT_HPP
#define _ECHIDNA_CLIENT_CLIENT_HPP

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "net/socket.hpp"

namespace echidna::client {
    class RenderQueue;

    //A structure describing a single frame
    struct ClientPacket {
        uint32_t job_id;
        uint32_t frame_id;
    };

    //The main network client, connects to the server and receives jobs
    class Client {
        private:
            //Basic properties of the client
            net::Socket socket;
            RenderQueue& render_queue;

            //Thread management and synchronization
            std::thread recv_thread;
            std::thread send_thread;
            std::mutex write_mutex;
            std::mutex send_queue_mutex;
            std::condition_variable send_queue_cond;

            //Client state
            std::atomic<bool> active;
            std::atomic<uint64_t> processing_time;

            //Buffer of outgoing UPDATE_JOB packets
            std::vector<ClientPacket> send_queue;

            //Main thread for handling messages received from the server
            void handleRecv();
            //Main thread responsible for sending updates to the server
            void handleUpdate();

            //Send a request to the server, making sure the entire packet is sent as one whole packet
            bool issueRequest(const void*, size_t);
        public:
            Client(const std::string&, int, RenderQueue&);
            ~Client();

            //Start/join/stop the client
            void start();
            void join();
            void stop();

            //Send an update to the server, informing it which frames for which job have been processed
            void updateServer(uint32_t, const std::vector<uint32_t>&, uint64_t);
            //Requests more jobs from the server
            void requestMoreJobs();
    };
}

#endif
