#include "server/clienthandler.hpp"
#include "server/clientmanager.hpp"

#include "error/network.hpp"

#include <unordered_set>
#include <sstream>
#include <cstring>
#include <map>

namespace echidna::server {
    struct JobRequest {
        uint32_t job_id;
        uint32_t fps;
        uint32_t image_width;
        uint32_t image_height;
        std::string shader;
        std::vector<uint32_t> frames;
    };

    ClientHandler::ClientHandler(std::unique_ptr<net::Socket>&& socket, ClientManager& manager, uint32_t client_id) :
                                        socket(std::move(socket)), manager(manager), client_id(client_id), active(false), keepalive(false), new_jobs(false), has_received(false) {}
    ClientHandler::~ClientHandler() {}

    void ClientHandler::run() {
        std::cout << "Client handler active for client " << this->client_id << std::endl;
        this->active = true;
        this->active_thread = std::thread(&ClientHandler::handleResponse, this);
        this->issue_thread = std::thread(&ClientHandler::handleIssue, this);
    }

    void ClientHandler::join() {
        this->active_thread.join();
        this->issue_thread.join();
    }

    void ClientHandler::stop() {
        std::cout << "Stop called" << std::endl;
        this->active = false;
        this->socket->close();

        this->job_update_cond.notify_all();
        this->keepalive_cond.notify_all();
    }

    bool ClientHandler::tryKeepAlive() {
        this->keepalive = false;

        protocol::ServerPacketID id = protocol::ServerPacketID::KEEPALIVE;
        if(!this->issueRequest(&id, sizeof(id)))
            return false;

        std::unique_lock keepalive_lock(this->keepalive_mutex);
        this->keepalive_cond.wait(keepalive_lock, [&] {return this->keepalive || !this->active; });
        return this->keepalive;
    }

    bool ClientHandler::issueRequest(const void* buffer, size_t buffer_size) {
        std::unique_lock(this->send_mutex);

        try {
            this->socket->sendFully(buffer, buffer_size);
            return true;
        }
        catch(const error::NetworkException& e) {
            return false;
        }
    }

    void ClientHandler::handleResponse() {
        try {
            while(this->active) {
                protocol::ClientPacketID id = this->socket->recv<protocol::ClientPacketID>();

                switch(id) {
                    case protocol::ClientPacketID::CONNECT:
                        this->manager.notifyFinish(this->client_id);
                        break;
                    case protocol::ClientPacketID::KEEPALIVE: {
                            std::unique_lock lock(this->keepalive_mutex);
                            this->keepalive = true;
                            this->keepalive_cond.notify_all();
                        }
                        break;
                    case protocol::ClientPacketID::UPDATE_JOB: {
                            uint32_t num_updates = this->socket->recv<uint32_t>();
                            for(uint32_t j = 0; j < num_updates; ++j) {
                                uint32_t job_id = this->socket->recv<uint32_t>();
                                uint32_t frame_id = this->socket->recv<uint32_t>();

                                this->manager.notifyUpdate(job_id, frame_id);
                            }

                            this->has_received = true;
                        }
                        break;
                    case protocol::ClientPacketID::FINISH_JOB: {
                            uint64_t time_per_frame = this->socket->recv<uint64_t>();
                            size_t frames = time_per_frame == 0 ? MAX_JOB_CAPABILITY : ESTIMATED_JOB_TIMEOUT.count() / time_per_frame;

                            if(frames > MAX_JOB_CAPABILITY)
                                frames = MAX_JOB_CAPABILITY;
                            else if(frames < MIN_JOB_CAPABILITY)
                                frames = MIN_JOB_CAPABILITY;

                            if(this->has_received)
                                this->job_capability = frames;
                            this->manager.notifyFinish(this->client_id);
                        }
                        break;
                }
            }
        }
        catch(const error::NetworkException& e) {
            std::cout << "Recv error: " << e.what() << std::endl;
        }

        this->manager.returnJobs(this->active_tasks);

        this->active = false;
        this->keepalive_cond.notify_all();

        this->stop();
    }

    void ClientHandler::handleIssue() {
        while(this->active) {

            {
                std::unique_lock lock(this->task_mutex);
                this->job_update_cond.wait(lock, [&] {return this->new_jobs || !this->active;});
                if(!this->active)
                    break;
                this->new_jobs = false;

                std::map<uint32_t, JobRequest> requests;

                this->rendered_frames = this->active_tasks.size();
                for(size_t i = 0; i < this->active_tasks.size(); ++i) {
                    bool already_exists = requests.count(this->active_tasks[i].job) > 0;
                    JobRequest& r = requests[this->active_tasks[i].job];
                    if(!already_exists) {
                        r.job_id = this->active_tasks[i].job;
                        r.fps = this->active_tasks[i].fps;
                        r.image_width = this->active_tasks[i].width;
                        r.image_height = this->active_tasks[i].height;
                        r.shader = this->active_tasks[i].shader;
                    }
                    r.frames.push_back(this->active_tasks[i].frame);
                }

                for(auto& it : requests) {
                    uint64_t shader_size = it.second.shader.size();
                    uint32_t frame_size = it.second.frames.size();

                    size_t packet_size = 1 + 5 * sizeof(uint32_t) + sizeof(uint64_t) + shader_size + frame_size * sizeof(uint32_t);
                    std::unique_ptr<uint8_t[]> packet(new uint8_t[packet_size]);
                    packet[0] = static_cast<uint8_t>(protocol::ServerPacketID::ISSUE_JOB);
                    std::memcpy(&packet[1], &it.second.job_id, sizeof(uint32_t));
                    std::memcpy(&packet[1 + sizeof(uint32_t)], &it.second.fps, sizeof(uint32_t));
                    std::memcpy(&packet[1 + 2 * sizeof(uint32_t)], &it.second.image_width, sizeof(uint32_t));
                    std::memcpy(&packet[1 + 3 * sizeof(uint32_t)], &it.second.image_height, sizeof(uint32_t));
                    std::memcpy(&packet[1 + 4 * sizeof(uint32_t)], &shader_size, sizeof(uint64_t));
                    std::memcpy(&packet[1 + 4 * sizeof(uint32_t) + sizeof(uint64_t)], it.second.shader.data(), shader_size);

                    size_t packet_offset = 1 + 4 * sizeof(uint32_t) + sizeof(uint64_t) + shader_size;

                    std::memcpy(&packet[packet_offset], &frame_size, sizeof(uint32_t));

                    packet_offset += sizeof(uint32_t);

                    for(size_t i = 0; i < frame_size; ++i) {
                        std::memcpy(&packet[packet_offset], &it.second.frames[i], sizeof(uint32_t));
                        packet_offset += sizeof(uint32_t);
                    }

                    if(!this->issueRequest(packet.get(), packet_size)) {
                        this->manager.returnJobs(this->active_tasks);
                        this->active_tasks.clear();
                        this->stop();
                        return;
                    }
                }
            }
        }
    }

    size_t ClientHandler::getJobCapability() const {
        return this->job_capability;
    }

    void ClientHandler::submitTasks(const std::vector<Task>& tasks) {
        std::unique_lock lock(this->task_mutex);
        this->active_tasks = tasks;
        this->new_jobs = true;

        this->job_update_cond.notify_all();
    }
}