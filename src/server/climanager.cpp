#include "server/climanager.hpp"
#include "server/jobqueue.hpp"
#include "error/network.hpp"
#include "protocol/cli.hpp"

namespace echidna::server {
    CLIManager::CLIManager(int port, JobQueue& job_queue) : server_port(port), job_queue(job_queue) {}
    CLIManager::~CLIManager() {}

    void CLIManager::run() {
        this->socket.bind("0.0.0.0", this->server_port);

        this->active = true;
        this->server_thread = std::thread(&CLIManager::serverThread, this);
    }

    void CLIManager::stop() {
        this->active = false;
        this->socket.close();
    }

    void CLIManager::join() {
        this->server_thread.join();
    }

    void CLIManager::serverThread() {
        try {
            while(this->active) {
                std::unique_ptr<net::Socket> sock = this->socket.accept();

                this->handleSocket(sock.get());
            }
        }
        catch(const error::NetworkException& n) {}

        this->stop();
    }

    void CLIManager::handleSocket(net::Socket* sock) {
        protocol::CLIRequestID command = sock->recv<protocol::CLIRequestID>();

        switch(command) {
            case protocol::CLIRequestID::UPLOAD:
                this->handleUpload(sock);
                break;
            case protocol::CLIRequestID::STATUS:
                this->handleStatus(sock);
                break;
        }
    }

    void CLIManager::handleUpload(net::Socket* sock) {
        uint32_t fps = sock->recv<uint32_t>();
        uint32_t frames = sock->recv<uint32_t>();
        uint32_t width = sock->recv<uint32_t>();
        uint32_t height = sock->recv<uint32_t>();
        uint32_t shader_size = sock->recv<uint32_t>();

        std::string shader_source;
        shader_source.resize(shader_size);
        sock->recvFully(shader_source.data(), shader_size);

        this->job_queue.addJob(shader_source, frames, fps, width, height);
    }

    void CLIManager::handleStatus(net::Socket* sock) {

    }
}