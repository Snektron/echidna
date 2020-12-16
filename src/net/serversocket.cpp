#include "net/serversocket.hpp"
#include "error/network.hpp"


#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace echidna::net {
    struct serversocket_platform_data {
        int sock;
    };

    ServerSocket::ServerSocket() {
        this->platform_data = new serversocket_platform_data;

        this->platform_data->sock = -1;
    }

    ServerSocket::ServerSocket(ServerSocket&& other) : platform_data(other.platform_data) {
        other.platform_data = nullptr;
    }

    ServerSocket::~ServerSocket() {
        this->close();

        delete this->platform_data;
    }

    ServerSocket& ServerSocket::operator=(ServerSocket&& other) {
        std::swap(this->platform_data, other.platform_data);
        return *this;
    }

    void ServerSocket::bind(const std::string& addr, int port) {
        struct addrinfo hints = {};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        std::string port_str = std::to_string(port);

        struct addrinfo* addresses;

        int status = getaddrinfo(addr.c_str(), port_str.c_str(), &hints, &addresses);
        if(status != 0)
            throw error::SocketException("Failed to resolve hostname ", addr, " at port ", port, ", error", status);

        for(struct addrinfo* addr = addresses; addr != nullptr; addr = addr->ai_next) {
            int sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            if(sock == -1)
                continue;

            if(::bind(sock, addr->ai_addr, addr->ai_addrlen) != 0) {
                ::close(sock);
                continue;
            }

            if(::listen(sock, 0) != 0) {
                ::close(sock);
                continue;
            }

            freeaddrinfo(addresses);
            this->platform_data->sock = sock;
            return;
        }
        freeaddrinfo(addresses);
        throw error::SocketException("Failed to bind to ", addr, " at port ", port);
    }

    void ServerSocket::close() {
        if(this->platform_data != nullptr && this->platform_data->sock != -1) {
            ::close(this->platform_data->sock);
            this->platform_data->sock = -1;
        }
    }

    std::unique_ptr<Socket> ServerSocket::accept() {
        int result = ::accept(this->platform_data->sock, NULL, NULL);
        if(result == -1)
            throw error::SocketException("Failed to except a connection on the server socket, error: ", errno);
        return std::unique_ptr<Socket>(new Socket(result));
    }
}