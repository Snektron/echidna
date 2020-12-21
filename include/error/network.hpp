#ifndef _ECHIDNA_ERROR_NETWORK_HPP
#define _ECHIDNA_ERROR_NETWORK_HPP

#include "error/exception.hpp"

namespace echidna::error {
    //A generic network error
    class NetworkException : public Exception {
        public:
            template <typename... T>
            NetworkException(const T&... args) : Exception(args...) {}
            virtual ~NetworkException() = default;
    };

    //Error when using a socket
    class SocketException : public NetworkException {
        public:
            template <typename... T>
            SocketException(const T&... args) : NetworkException(args...) {}
            virtual ~SocketException() = default;
    };
}

#endif
