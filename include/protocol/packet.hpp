#ifndef _ECHIDNA_PROTOCOL_PACKET_HPP
#define _ECHIDNA_PROTOCOL_PACKET_HPP

#include <cstdint>

namespace echidna::protocol {
    //Server->client packet IDs
    enum class ServerPacketID : uint8_t {
        KEEPALIVE = 1,
        ISSUE_JOB = 2
    };

    //Client->server packet IDs
    enum class ClientPacketID : uint8_t {
        CONNECT = 0,
        KEEPALIVE = 1,
        UPDATE_JOB = 2,
        FINISH_JOB = 3
    };
}

#endif
