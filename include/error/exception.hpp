#ifndef _ECHIDNA_ERROR_EXCEPTION_HPP
#define _ECHIDNA_ERROR_EXCEPTION_HPP

#include "utils/string.hpp"

namespace echidna::error {
    class Exception : public std::runtime_error {
        public:
            template <typename... T>
            Exception(const T&... args) : std::runtime_error(utils::string::make_string(args...)) {}
            virtual ~Exception() = default;
    };
}

#endif
