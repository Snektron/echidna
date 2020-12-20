#include <iostream>
#include <cstring>
#include <cstdint>
#include <memory>
#include <vector>

#include "net/socket.hpp"
#include "protocol/cli.hpp"

struct JobStatus {
    uint32_t id;
    uint32_t frames_rendered;
    uint32_t total_frames;
};

void submit_job(echidna::net::Socket& sock, const std::string& shader, uint32_t frames, uint32_t framerate, uint32_t width, uint32_t height) {
    uint32_t shader_size = shader.size();

    size_t buffer_size = 1 + 5 * sizeof(uint32_t) + shader.size();
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[buffer_size]);
    buffer[0] = static_cast<uint8_t>(echidna::protocol::CLIRequestID::UPLOAD);
    std::memcpy(&buffer[1], &framerate, sizeof(uint32_t));
    std::memcpy(&buffer[5], &frames, sizeof(uint32_t));
    std::memcpy(&buffer[9], &width, sizeof(uint32_t));
    std::memcpy(&buffer[13], &height, sizeof(uint32_t));
    std::memcpy(&buffer[17], &shader_size, sizeof(uint32_t));
    std::memcpy(&buffer[21], shader.data(), shader_size);

    sock.sendFully(buffer.get(), buffer_size);
}

std::vector<JobStatus> query_status(echidna::net::Socket& sock) {
    echidna::protocol::CLIRequestID request = echidna::protocol::CLIRequestID::STATUS;
    sock.sendFully(&request, sizeof(request));

    std::vector<JobStatus> result;
    uint32_t num_jobs = sock.recv<uint32_t>();
    for(size_t i = 0; i < num_jobs; ++i) {
        JobStatus s;
        s.id = sock.recv<uint32_t>();
        s.frames_rendered = sock.recv<uint32_t>();
        s.total_frames = sock.recv<uint32_t>();
        result.push_back(s);
    }

    return result;
}

int main(int argc, char* argv[]) {
    echidna::net::Socket socket;
    socket.connect("localhost", 4243);

    submit_job(socket, "DIT IS EEN SHADER", 100, 42, 100, 100);

    return 0;
}