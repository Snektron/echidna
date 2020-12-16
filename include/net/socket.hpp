#ifndef _ECHIDNA_NET_SOCKET_HPP
#define _ECHIDNA_NET_SOCKET_HPP

#include <string>
#include <cstddef>

namespace echidna::net {
    struct socket_platform_data;

    class Socket {
        private:
            socket_platform_data* platform_data;
        public:
            Socket();
            Socket(const Socket&) = delete;
            Socket(Socket&&);
            Socket(int);
            ~Socket();

            Socket& operator=(const Socket&) = delete;
            Socket& operator=(Socket&&);

            void connect(const std::string&, int);
            void close();

            size_t send(const void*, size_t);
            size_t recv(void*, size_t);

            void sendFully(const void*, size_t);
            void recvFully(void*, size_t);

            template <typename T>
            T recv() {
                T result;
                this->recvFully(&result, sizeof(T));
                return result;
            }
    };
}

#endif