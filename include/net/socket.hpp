#ifndef _ECHIDNA_NET_SOCKET_HPP
#define _ECHIDNA_NET_SOCKET_HPP

#include <string>
#include <cstddef>

namespace echidna::net {
    struct socket_platform_data;

    //A TCP client socket class
    class Socket {
        private:
            //Platform-specific socket data
            socket_platform_data* platform_data;
        public:
            Socket();
            Socket(const Socket&) = delete;
            Socket(Socket&&);
            Socket(int);
            ~Socket();

            Socket& operator=(const Socket&) = delete;
            Socket& operator=(Socket&&);

            //Connects to a given server and port
            void connect(const std::string&, int);
            //Closes the socket
            void close();

            //Send a buffer up the the number of bytes specified
            size_t send(const void*, size_t);
            //Receives into a buffer, up to the number of bytes specified
            size_t recv(void*, size_t);

            //Fully sends a buffer, throwing an error if not all bytes could be sent
            void sendFully(const void*, size_t);
            //Fully fill a buffer, throwing an error if not enough bytes were received
            void recvFully(void*, size_t);

            //Utility function to receive an arbitrary datatype
            template <typename T>
            T recv() {
                T result;
                this->recvFully(&result, sizeof(T));
                return result;
            }
    };
}

#endif