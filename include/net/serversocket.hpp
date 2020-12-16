#ifndef _ECHIDNA_NET_SERVERSOCKET_HPP
#define _ECHIDNA_NET_SERVERSOCKET_HPP

#include "net/socket.hpp"

#include <memory>

namespace echidna::net {
    struct serversocket_platform_data;

    class ServerSocket {
        private:
            serversocket_platform_data* platform_data;
        public:
            ServerSocket();
            ServerSocket(const ServerSocket&) = delete;
            ServerSocket(ServerSocket&&);
            ~ServerSocket();

            ServerSocket& operator=(const ServerSocket&) = delete;
            ServerSocket& operator=(ServerSocket&&);

            void bind(const std::string&, int);
            void close();

            std::unique_ptr<Socket> accept();
    };
}

#endif
