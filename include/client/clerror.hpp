#ifndef _ECHIDNA_CLIENT_CLERROR_HPP
#define _ECHIDNA_CLIENT_CLERROR_HPP

#include "error/exception.hpp"

#include <CL/cl.h>
#include <string_view>

namespace echidna::client {
    struct CLException : public error::Exception {
        cl_int status;

        CLException(cl_int status) : Exception(errorName(status)) {}
        virtual ~CLException() = default;

        std::string_view errorName(cl_int status);
    };

    struct InvalidCLVersionStringException : public error::Exception {
        InvalidCLVersionStringException() : Exception() {}
        virtual ~InvalidCLVersionStringException() = default;
    };

    struct NoDeviceException : public error::Exception {
        NoDeviceException() : Exception("No suitable OpenCL devices") {}
        virtual ~NoDeviceException() = default;
    };

    void check(cl_int status);
}

#endif
