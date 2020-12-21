#ifndef _ECHIDNA_NET_SERVERSOCKET_HPP
#define _ECHIDNA_NET_SERVERSOCKET_HPP

#include "net/socket.hpp"

#include <memory>

namespace echidna::net {
    struct serversocket_platform_data;

    //A class defining a single TCP server socket
    class ServerSocket {
        private:
            //Platform-specific socket data
            serversocket_platform_data* platform_data;
        public:
            ServerSocket();
            ServerSocket(const ServerSocket&) = delete;
            ServerSocket(ServerSocket&&);
            ~ServerSocket();

            ServerSocket& operator=(const ServerSocket&) = delete;
            ServerSocket& operator=(ServerSocket&&);

            //Binds the server socket to a given address
            void bind(const std::string&, int);
            //Closes the socket
            void close();

            //Accept a socket, can only be called once bound
            std::unique_ptr<Socket> accept();
    };
}

#endif
