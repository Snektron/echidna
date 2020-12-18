#include "client/client.hpp"
#include "client/rendertask.hpp"
#include "client/renderqueue.hpp"
#include "error/network.hpp"
#include "protocol/packet.hpp"

namespace echidna::client {
    Client::Client(const std::string& host, int host_port, RenderQueue& queue) : render_queue(queue) {
        this->socket.connect(host, host_port);
    }

    Client::~Client() {

    }

    void Client::start() {
        this->active = true;
        this->recv_thread = std::thread(&Client::handleRecv, this);
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
}