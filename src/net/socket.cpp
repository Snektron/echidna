#include "net/socket.hpp"
#include "error/network.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace echidna::net {
    struct socket_platform_data {
        int sock;
    };

    Socket::Socket() {
        this->platform_data = new socket_platform_data;

        this->platform_data->sock = -1;
    }

    Socket::Socket(Socket&& other) : platform_data(other.platform_data) {
        other.platform_data = nullptr;
    }

    Socket::Socket(int sock) {
        this->platform_data = new socket_platform_data;

        this->platform_data->sock = sock;
    }

    Socket::~Socket() {
        this->close();

        delete this->platform_data;
    }

    Socket& Socket::operator=(Socket&& other) {
        std::swap(this->platform_data, other.platform_data);
        return *this;
    }

    void Socket::connect(const std::string& host, int port) {
        struct addrinfo hints = {};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        std::string port_str = std::to_string(port);

        struct addrinfo* addresses;

        int status = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &addresses);
        if(status != 0)
            throw error::SocketException("Failed to resolve hostname ", host, " at port ", port, ", error ", status);

        for(struct addrinfo* addr = addresses; addr != nullptr; addr = addr->ai_next) {
            int sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            if(sock == -1)
                continue;

            if(::connect(sock, addr->ai_addr, addr->ai_addrlen) != 0) {
                ::close(sock);
                continue;
            }

            freeaddrinfo(addresses);

            this->platform_data->sock = sock;
            return;
        }

        freeaddrinfo(addresses);
        throw error::SocketException("Failed to connect to host ", host, " at port ", port);
    }

    void Socket::close() {
        if(this->platform_data != nullptr && this->platform_data->sock != -1) {
            ::close(this->platform_data->sock);
            this->platform_data->sock = -1;
        }
    }

    size_t Socket::send(const void* data, size_t data_size) {
        ssize_t retval = ::send(this->platform_data->sock, data, data_size, 0);
        if(retval == -1)
            throw error::SocketException("Failed to send data");
        return retval;
    }

    size_t Socket::recv(void* buf, size_t buf_size) {
        ssize_t retval = ::recv(this->platform_data->sock, buf, buf_size, 0);
        if(retval == -1)
            throw error::SocketException("Failed to receive data");
        return retval;
    }

    void Socket::sendFully(const void* data, size_t data_size) {
        char* data_chr = (char*)data;
        size_t sent = 0;
        while(sent != data_size) {
            size_t data_sent = this->send(data_chr + sent, data_size - sent);
            if(data_sent == 0)
                throw error::SocketException("Failed to fully send buffer, failed at offset ", sent);
            sent += data_sent;
        }
    }

    void Socket::recvFully(void* data, size_t data_size) {
        char* data_chr = (char*)data;
        size_t received = 0;
        while(received != data_size) {
            size_t data_received = this->recv(data_chr + received, data_size - received);
            if(data_received == 0)
                throw error::SocketException("Failed to fully receive buffer, failed at offset ", received);
            received += data_received;
        }
    }
}