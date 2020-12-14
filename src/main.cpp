#include <iostream>

#include "net/socket.hpp"

const char REQUEST[] = "GET / HTTP/1.1\r\nHost: pythons.space\r\nConnection: close\r\n\r\n";

int main() {
    echidna::net::Socket socket;
    socket.connect("pythons.space", 80);

    socket.sendFully(REQUEST, sizeof(REQUEST));

    size_t result;
    do {
        char buf[1024];
        result = socket.recv(buf, sizeof(buf));

        std::cout.write(buf, result);
    } while(result != 0);

    return 0;
}