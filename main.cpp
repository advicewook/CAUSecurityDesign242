#include "Server.h"
#include "Client.h"
#include <thread>
#include <chrono>

int main() {
    // Start the server in a separate thread
    std::thread serverThread(startServer);

    // Wait for the server to start
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Start the client
    startClient();

    // Wait for the server thread to finish
    serverThread.join();

    return 0;
}
