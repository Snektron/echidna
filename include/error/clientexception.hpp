#ifndef _ECHIDNA_ERROR_CLIENTEXCEPTION_HPP
#define _ECHIDNA_ERROR_CLIENTEXCEPTION_HPP

#include "error/exception.hpp"

namespace echidna::error {
    //Generic exception on the client
    class ClientException : public Exception {
        public:
            template <typename... T>
            ClientException(const T&... args) : Exception(args...) {}
            virtual ~ClientException() = default;
    };

    //Exception thrown when no jobs could be obtained
    class NoJobsAvailableException : public ClientException {
        public:
            template <typename... T>
            NoJobsAvailableException(const T&... args) : ClientException(args...) {}
            virtual ~NoJobsAvailableException() = default;
    };
}

#endif
