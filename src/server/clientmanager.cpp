#include "server/clientmanager.hpp"
#include "server/clienthandler.hpp"
#include "error/network.hpp"

#include <vector>
#include <chrono>

namespace echidna::server {
    ClientManager::ClientManager(int server_port, size_t keepalive_time, JobQueue& job_queue) : server_port(server_port), keepalive_time(keepalive_time),
                                    job_queue(job_queue), active(false), client_id_offset(0) {
    }

    ClientManager::~ClientManager() {
        std::unique_lock lock(this->client_map_mutex);

        for(auto& it : this->client_map) {
            it.second->stop();
            it.second->join();
            delete it.second;
        }
    }

    void ClientManager::run() {
        this->server.bind("0.0.0.0", this->server_port);
        this->active = true;

        this->server_thread = std::thread(&ClientManager::handleClients, this);
        this->keepalive_thread = std::thread(&ClientManager::handleKeepAlive, this);
        this->job_distribution_thread = std::thread(&ClientManager::handleJobDistribution, this);
    }

    void ClientManager::join() {
        this->server_thread.join();
        this->keepalive_thread.join();

        std::shared_lock(this->client_map_mutex);
        for(auto& it : this->client_map) {
            it.second->join();
        }
    }

    void ClientManager::stop() {
        this->active = false;
        this->server.close();

        std::shared_lock(this->client_map_mutex);
        for(auto& it : this->client_map) {
            it.second->stop();
        }
    }


    void ClientManager::handleClient(std::unique_ptr<net::Socket> client) {
        uint32_t client_id = this->client_id_offset++;

        std::unique_lock(this->client_map_mutex);
        auto handler = new ClientHandler(std::move(client), *this, client_id);
        this->client_map[client_id] = handler;

        handler->run();
    }

    void ClientManager::handleClients() {
        try {
            while(this->active) {
                std::unique_ptr<net::Socket> client = this->server.accept();
                this->handleClient(std::move(client));
            }
        }
        catch(const error::NetworkException& e) {}
        this->active = false;
    }

    void ClientManager::handleKeepAlive() {
        auto offset_time = std::chrono::steady_clock::now();

        while(this->active) {
            offset_time += std::chrono::seconds(this->keepalive_time);
            std::this_thread::sleep_until(offset_time);

            std::vector<std::pair<uint32_t, ClientHandler*>> handlers;

            {
                std::shared_lock(this->client_map_mutex);
                for(auto& it : this->client_map) {
                    handlers.push_back(it);
                }
            }

            for(auto& it : handlers) {
                if(!it.second->tryKeepAlive()) {
                    it.second->stop();
                    it.second->join();

                    std::unique_lock(this->client_map_mutex);
                    delete it.second;
                    this->client_map.erase(it.first);
                }
            }
        }
    }

    void ClientManager::handleJobDistribution() {
        while(this->active) {
            uint32_t free_client_id;
            {
                std::unique_lock free_lock(this->free_client_mutex);
                this->free_client_cond.wait(free_lock, [&] {return this->free_clients.size() > 0 || !this->active;});

                if(this->free_clients.size() == 0)
                    continue;

                free_client_id = this->free_clients.front();
                this->free_clients.pop_front();
            }

            size_t job_capability;
            {
                std::shared_lock client_map_lock(this->client_map_mutex);
                if(this->client_map.count(free_client_id) == 0)
                    continue;

                ClientHandler* handler = this->client_map[free_client_id];
                job_capability = handler->getJobCapability();
            }

            std::vector<Task> tasks = this->job_queue.getJobs(job_capability);

            {
                std::shared_lock client_map_lock(this->client_map_mutex);
                if(this->client_map.count(free_client_id) == 0) {
                    client_map_lock.unlock();
                    this->job_queue.addTasks(tasks);
                    continue;
                }

                ClientHandler* handler = this->client_map[free_client_id];
                if(!handler->submitTasks(tasks)) {
                    client_map_lock.unlock();
                    this->job_queue.addTasks(tasks);
                    continue;
                }
            }
        }
    }

    void ClientManager::notifyUpdate(uint32_t job_id, uint32_t frame) {
        Job* job = this->job_queue.findJob(job_id);
        if(job == nullptr)
            return;
        job->frameRendered([&] {
            this->job_queue.finishJob(job_id);
        });
    }

    void ClientManager::notifyFinish(uint32_t client_id) {
        std::unique_lock lock(this->free_client_mutex);
        this->free_clients.push_back(client_id);

        this->free_client_cond.notify_all();
    }

    void ClientManager::returnJobs(const std::vector<Task>& jobs) {
        this->job_queue.addTasks(jobs);
    }

    void ClientManager::removeClient(uint32_t client_id) {
        std::unique_lock lock(this->client_map_mutex);
        ClientHandler* handler = this->client_map[client_id];
        std::vector<Task> tasks = handler->getJobs();
        this->returnJobs(tasks);
    }
}