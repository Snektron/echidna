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

    void check(cl_int status);
}
