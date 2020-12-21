#ifndef _ECHIDNA_CLIENT_CLERROR_HPP
#define _ECHIDNA_CLIENT_CLERROR_HPP

#include "error/exception.hpp"

#include <CL/cl.h>
#include <string_view>

namespace echidna::client {
    // Exception thrown when an unexpected OpenCL error has been returned.
    struct CLException : public error::Exception {
        cl_int status;

        CLException(cl_int status) : Exception(errorName(status), " (", status, ")") {}
        virtual ~CLException() = default;

        // Get a string describing an OpenCL error status.
        std::string_view errorName(cl_int status);
    };

    // Exception thrown when the OpenCL implementation returns an invalid version string.
    struct InvalidCLVersionStringException : public error::Exception {
        InvalidCLVersionStringException() : Exception() {}
        virtual ~InvalidCLVersionStringException() = default;
    };

    // Exception thrown when there is no device suitable for rendering.
    struct NoDeviceException : public error::Exception {
        NoDeviceException() : Exception("No suitable OpenCL devices") {}
        virtual ~NoDeviceException() = default;
    };

    // Exception thrown when a kernel has failed to compile.
    struct KernelCompilationException : public error::Exception {
        template <typename... T>
        KernelCompilationException(const T&... args) : error::Exception(args...) {}
        virtual ~KernelCompilationException() = default;
    };

    void check(cl_int status);
}

#endif
