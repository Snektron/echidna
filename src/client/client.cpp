#include "client/client.hpp"
#include "client/rendertask.hpp"
#include "client/renderqueue.hpp"
#include "error/network.hpp"
#include "protocol/packet.hpp"

#include <cstring>

namespace echidna::client {
    Client::Client(const std::string& host, int host_port, RenderQueue& queue) : render_queue(queue) {
        this->socket.connect(host, host_port);
    }

    Client::~Client() {

    }

    void Client::start() {
        this->active = true;
        this->recv_thread = std::thread(&Client::handleRecv, this);
        this->send_thread = std::thread(&Client::handleUpdate, this);
    }

    void Client::join() {
        this->recv_thread.join();
    }

    void Client::stop() {
        this->active = false;

        this->socket.close();
    }

    bool Client::issueRequest(const void* data, size_t data_size) {
        std::unique_lock lock(this->write_mutex);

        try {
            this->socket.sendFully(data, data_size);
            return true;
        }
        catch(const error::NetworkException& e) {
            return false;
        }
    }

    void Client::handleRecv() {
        try {
            while(this->active) {
                protocol::ServerPacketID id = this->socket.recv<protocol::ServerPacketID>();
                switch(id) {
                    case protocol::ServerPacketID::KEEPALIVE: {
                            protocol::ClientPacketID id = protocol::ClientPacketID::KEEPALIVE;
                            this->issueRequest(&id, sizeof(id));
                        }
                        break;
                    case protocol::ServerPacketID::ISSUE_JOB: {
                            RenderTaskInfo render_task;
                            render_task.job_id = this->socket.recv<uint32_t>();
                            render_task.fps = this->socket.recv<uint32_t>();
                            render_task.image_width = this->socket.recv<uint32_t>();
                            render_task.image_height = this->socket.recv<uint32_t>();

                            uint64_t shader_source_size = this->socket.recv<uint64_t>();
                            render_task.kernel_source.resize(shader_source_size);
                            this->socket.recvFully(render_task.kernel_source.data(), shader_source_size);

                            uint32_t frame_count = this->socket.recv<uint32_t>();
                            this->socket.recvFully(render_task.timestamps.data(), frame_count * sizeof(uint32_t));

                            this->render_queue.addTask(render_task);
                        }
                        break;
                }
            }
        }
        catch(const error::NetworkException& e) {
            //TODO: log this
        }

        this->stop();
    }

    void Client::handleUpdate() {
        try {
            while(this->active) {
                std::unique_lock lock(this->send_queue_mutex);
                this->send_queue_cond.wait(lock, [&] {return this->send_queue.size() > 0 || !this->active;});
                if(this->send_queue.size() > 0) {
                    size_t request_size = 1 + sizeof(uint32_t) * (2 * this->send_queue.size() + 1);
                    std::unique_ptr<uint8_t[]> request(new uint8_t[1 + sizeof(uint32_t) * (2 * this->send_queue.size() + 1)]);
                    uint32_t send_queue_size = this->send_queue.size();
                    request[0] = static_cast<uint8_t>(protocol::ClientPacketID::UPDATE_JOB);
                    std::memcpy(&request[1], &send_queue_size, sizeof(uint32_t));

                    for(size_t i = 0; i < send_queue_size; ++i) {
                        std::memcpy(&request[1 + i * sizeof(uint32_t)], &this->send_queue[i], sizeof(uint32_t));
                    }

                    lock.release();

                    if(!this->issueRequest(request.get(), request_size)) {
                        this->stop();

                        //TODO: log
                    }

                    lock.lock();

                    this->send_queue.clear();
                }
            }
        }
        catch(const error::NetworkException& e) {
            //TODO: log this
        }
        this->stop();
    }

    void Client::updateServer(uint32_t job_id, uint32_t frame_id) {
        std::unique_lock lock(this->send_queue_mutex);
        this->send_queue.push_back(ClientPacket{job_id, frame_id});
    }
}