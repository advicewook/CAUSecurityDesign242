#include "Server.h"
#include <boost/asio.hpp>
#include <iostream>

using namespace boost::asio;
using ip::tcp;

void startServer() {
    try {
        io_service ioService;
        tcp::acceptor acceptor(ioService, tcp::endpoint(tcp::v4(), 12345));
        std::cout << "Server is running on port 12345..." << std::endl;

        while (true) {
            tcp::socket socket(ioService);
            acceptor.accept(socket);

            char data[1024] = {0};
            size_t length = socket.read_some(buffer(data));
            std::cout << "Received from client: " << std::string(data, length) << std::endl;

            std::string response = "Data received: " + std::string(data, length);
            write(socket, buffer(response));
        }
    } catch (std::exception& e) {
        std::cerr << "Server Error: " << e.what() << std::endl;
    }
}
