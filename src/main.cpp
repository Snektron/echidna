#include <iostream>
#include <thread>

#include "net/socket.hpp"
#include "net/serversocket.hpp"

const char REQUEST[] = "Praise be the opperpython";
const char SERVER_RESPONSE[] = "Server ack";

const int SERVER_PORT = 4242;

void server_routine() {
    echidna::net::ServerSocket server;
    std::cout << "Server binding" << std::endl;
    server.bind("localhost", SERVER_PORT);

    std::cout << "Server accepting" << std::endl;
    echidna::net::Socket client = server.accept();

    char buffer[1024];
    std::cout << "Server receiving request" << std::endl;
    size_t result = client.recv(buffer, 1024);
    std::cout << "Server received " << result << " bytes from client" << std::endl;
    std::cout.write(buffer, result);

    std::cout << std::endl;

    client.sendFully(SERVER_RESPONSE, sizeof(SERVER_RESPONSE));
}

int main() {
    std::thread server_thread(server_routine);

    std::cout << "Press any key to start connecting" << std::endl;
    std::cin.get();

    echidna::net::Socket socket;
    std::cout << "Client connecting" << std::endl;
    socket.connect("localhost", SERVER_PORT);

    std::cout << "Client sending request" << std::endl;
    socket.sendFully(REQUEST, sizeof(REQUEST));

    std::cout << "Client receiving response" << std::endl;
    char response[1024];
    size_t result = socket.recv(response, 1024);
    std::cout << "Client received " << result << " bytes from server" << std::endl;
    std::cout.write(response, result);

    std::cout << std::endl;

    std::cout << "Joining server" << std::endl;
    server_thread.join();

    return 0;
}