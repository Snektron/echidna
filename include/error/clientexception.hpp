#ifndef _ECHIDNA_ERROR_CLIENTEXCEPTION_HPP
#define _ECHIDNA_ERROR_CLIENTEXCEPTION_HPP

#include "error/exception.hpp"

namespace echidna::error {
    class ClientException : public Exception {
        public:
            template <typename... T>
            ClientException(const T&... args) : Exception(args...) {}
            virtual ~ClientException() = default;
    };

    class NoJobsAvailableException : public ClientException {
        public:
            template <typename... T>
            NoJobsAvailableException(const T&... args) : ClientException(args...) {}
            virtual ~NoJobsAvailableException() = default;
    };
}

#endif
