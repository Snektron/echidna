#include <iostream>
#include <cstring>
#include <cstdint>

#include "net/socket.hpp"
#include "protocol/cli.hpp"

char shader_source[] = "OEFAUWIEPYTHON";

int main(int argc, char* argv[]) {
    echidna::net::Socket socket;

    std::cout << "Connecting to server" << std::endl;
    socket.connect("localhost", 4243);

    uint32_t framerate = 42000;
    uint32_t num_frames = 20;
    uint32_t width = 420;
    uint32_t height = 421;
    uint32_t shader_size = sizeof(shader_source);

    size_t buffer_size = 1 + 5 * sizeof(uint32_t) + sizeof(shader_source);
    uint8_t* buffer = new uint8_t[buffer_size];
    buffer[0] = static_cast<uint8_t>(echidna::protocol::CLIRequestID::UPLOAD);
    std::memcpy(&buffer[1], &framerate, sizeof(uint32_t));
    std::memcpy(&buffer[5], &num_frames, sizeof(uint32_t));
    std::memcpy(&buffer[9], &width, sizeof(uint32_t));
    std::memcpy(&buffer[13], &height, sizeof(uint32_t));
    std::memcpy(&buffer[17], &shader_size, sizeof(uint32_t));
    std::memcpy(&buffer[21], &shader_source, sizeof(shader_source));

    std::cout << "Sending source" << std::endl;

    socket.sendFully(buffer, buffer_size);
    delete[] buffer;

    std::cout << "Send completed" << std::endl;

    return 0;
}