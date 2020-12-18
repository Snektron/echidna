#include "client/client.hpp"
#include "client/renderqueue.hpp"

int main() {
    echidna::client::RenderQueue queue;
    echidna::client::Client client("localhost", 80, queue);

    client.start();

    client.join();

    return 0;
}