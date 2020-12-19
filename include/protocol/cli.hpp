#ifndef _ECHIDNA_PROTOCOL_CLI_HPP
#define _ECHIDNA_PROTOCOL_CLI_HPP

namespace echidna::protocol {
    enum class CLIRequestID : uint8_t {
        UPLOAD = 0,
        STATUS = 1
    };
}

#endif
