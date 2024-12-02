#include "Client.h"
#include <boost/asio.hpp>
#include <iostream>

using namespace boost::asio;
using ip::tcp;
using namespace std;

void startClient() {
    try {
        io_service ioService;
        tcp::socket socket(ioService);
        socket.connect(tcp::endpoint(ip::address::from_string("127.0.0.1"), 12345));
        std::cout << "Connected to server!" << std::endl;

        std::string message = "Hello, Server!";
        write(socket, buffer(message));
        std::cout << "Sent to server: " << message << std::endl;

        char response[1024] = {0};
        size_t length = socket.read_some(buffer(response));
        std::cout << "Received from server: " << std::string(response, length) << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Client Error: " << e.what() << std::endl;
    }
}
