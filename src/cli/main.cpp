#include <iostream>
#include <cstring>
#include <cstdint>
#include <memory>
#include <vector>
#include <charconv>
#include <fstream>
#include <sstream>
#include <string_view>

#include "net/socket.hpp"
#include "protocol/cli.hpp"
#include "error/network.hpp"

struct JobStatus {
    uint32_t id;
    uint32_t frames_rendered;
    uint32_t total_frames;
};

uint32_t submit_job(echidna::net::Socket& sock, const std::string& shader, uint32_t frames, uint32_t fps, uint32_t width, uint32_t height) {
    uint32_t shader_size = shader.size();

    size_t buffer_size = 1 + 5 * sizeof(uint32_t) + shader.size();
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[buffer_size]);
    buffer[0] = static_cast<uint8_t>(echidna::protocol::CLIRequestID::UPLOAD);
    std::memcpy(&buffer[1], &fps, sizeof(uint32_t));
    std::memcpy(&buffer[5], &frames, sizeof(uint32_t));
    std::memcpy(&buffer[9], &width, sizeof(uint32_t));
    std::memcpy(&buffer[13], &height, sizeof(uint32_t));
    std::memcpy(&buffer[17], &shader_size, sizeof(uint32_t));
    std::memcpy(&buffer[21], shader.data(), shader_size);

    sock.sendFully(buffer.get(), buffer_size);

    return sock.recv<uint32_t>();
}

int submit(echidna::net::Socket& sock, const char* kernel, uint32_t frames, uint32_t fps, uint32_t w, uint32_t h) {
    std::ifstream kernel_src(kernel);
    if (!kernel_src) {
        std::cerr << "Error: Cannot open file '" << kernel << "'" << std::endl;
        return EXIT_FAILURE;
    }

    std::stringstream ss;
    ss << kernel_src.rdbuf();

    uint32_t job = submit_job(sock, ss.str(), frames, fps, w, h);

    std::cout << "Submitted job with id " << job << std::endl;

    return EXIT_SUCCESS;
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

int query(echidna::net::Socket& sock) {
    for (const auto& status : query_status(sock)) {
        std::cout << status.id << ": ";
        if (status.frames_rendered == 0 && status.total_frames == 0) {
            std::cout << " finished" << std::endl;
        } else {
            std::cout << status.frames_rendered << "/" << status.total_frames << std::endl;
        }
    }

    return EXIT_SUCCESS;
}

void print_help(const char* prog) {
    std::cerr << "Usage: " << prog << " [options...]\n"
        << "options:\n"
        << "--host <addr>       Echidna server to connect to\n"
        << "--port <port>       Port to connect to (default: 4243)\n"
        << "--submit <kernel>   Submit file <kernel> to be rendered. This also requires\n"
        << "                     arguments --frames, --dim and optionally --fps. This\n"
        << "                     option is incompatible with --query\n"
        << "--frames <amt>      Render <amt> frames in total\n"
        << "--dim <w>x<h>       Render frames of <w>x<h>\n"
        << "--fps <amt>         Render <fps> frames per second (default: 60)\n"
        << "--query             Query Echidna status. This option is incompatible\n"
        << "                    with --submit\n";
}

template <typename T>
bool parseIntArg(const char* arg, T& value, const char* opt_name) {
    size_t len = std::strlen(arg);
    auto [end, err] = std::from_chars(arg, arg + len, value);
    if (err != std::errc() || end != arg + len) {
        std::cerr << "Error: Invalid value '" << arg << "' for " << opt_name << std::endl;
        return false;
    }

    return true;
}

bool parseDimArg(const char* arg, uint32_t& w, uint32_t& h) {
    const char* x = std::strchr(arg, 'x');
    if (!x)
        return false;

    auto [wend, werr] = std::from_chars(arg, x, w);
    if (werr != std::errc() || wend != x)
        return false;

    size_t len = std::strlen(arg);
    auto [hend, herr] = std::from_chars(x + 1, arg + len, w);
    if (herr != std::errc() || hend != arg + len)
        return false;

    return true;
}

int main(int argc, char* argv[]) {
    bool query_opt = false;
    const char* host_opt = nullptr;
    const char* port_opt = nullptr;
    const char* submit_opt = nullptr;
    const char* frames_opt = nullptr;
    const char* dim_opt = nullptr;
    const char* fps_opt = nullptr;

    for (int i = 1; i < argc; ++i) {
        auto option = std::string_view(argv[i]);
        if (option == "--query") {
            query_opt = true;
            continue;
        } else if (option == "-h" || option == "--help") {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        }

        const char** arg = nullptr;
        if (option == "--host")
            arg = &host_opt;
        else if (option == "--port")
            arg = &port_opt;
        else if (option == "--submit")
            arg = &submit_opt;
        else if (option == "--frames")
            arg = &frames_opt;
        else if (option == "--dim")
            arg = &dim_opt;
        else if (option == "--fps")
            arg = &fps_opt;
        else {
            std::cerr << "Error: Unknown option '" << option << "'" << std::endl;
            return EXIT_FAILURE;
        }

        ++i;
        if (i >= argc) {
            std::cerr << "Error: Expected argument to " << option  << std::endl;
            return EXIT_FAILURE;
        }

        *arg = argv[i];
    }

    if (!host_opt) {
        std::cerr << "Error: Missing required argument --host" << std::endl;
        return EXIT_FAILURE;
    }

    if (!submit_opt && !query_opt) {
        std::cerr << "Error: Either --submit or --query is required" << std::endl;
        return EXIT_FAILURE;
    }

    if (submit_opt) {
        if (query_opt) {
            std::cerr << "Error: --submit and --query are mutually exclusive" << std::endl;
            return EXIT_FAILURE;
        }

        if (!frames_opt) {
            std::cerr << "Error: --submit requires --frames" << std::endl;
            return EXIT_FAILURE;
        }

        if (!dim_opt) {
            std::cerr << "Error: --submit requires --frames" << std::endl;
            return EXIT_FAILURE;
        }
    }

    uint16_t port = 4243;
    if (port_opt && !parseIntArg(port_opt, port, "--port")) {
        return EXIT_FAILURE;
    }

    uint32_t frames;
    if (frames_opt && !parseIntArg(frames_opt, frames, "--frames")) {
        return EXIT_FAILURE;
    }

    uint32_t fps = 60;
    if (fps_opt && !parseIntArg(fps_opt, fps, "--fps")) {
        return EXIT_FAILURE;
    }

    uint32_t w, h;
    if (dim_opt && !parseDimArg(dim_opt, w, h)) {
        std::cerr << "Error: Invalid value '" << dim_opt << "' for --dim" << std::endl;
        return EXIT_FAILURE;
    }

    echidna::net::Socket sock;

    try {
        sock.connect(host_opt, port);
    } catch (const echidna::error::SocketException& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        return EXIT_FAILURE;
    }

    if (query_opt) {
        return query(sock);
    } else {
        return submit(sock, submit_opt, frames, fps, w, h);
    }
}
