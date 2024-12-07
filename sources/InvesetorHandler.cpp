#include "InvestorHandler.h"


void InvestorHandler::run() {
    connectToServerUnEncrypted(); // connect server and input data in this function

}


void InvestorHandler::connectToServerUnEncrypted() {
    try {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "12345");
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        std::cout << "Connected to server!" << std::endl;

        while(true){
            std::string department, label, input;

            std::cout << "1. Request investment information 2.Show previous data 3. Exit " << std::endl;
            std::cin >> input;

            // Request investment information
            if(input == "1" || input == "Request" || input == "request"){
                std::cout << "Enter the department and label: ";
                std::cin.ignore(); // Clear the newline character from the input buffer
                std::getline(std::cin, input);

                if(!parseInput(input, department, label)){
                    std::cout << "Invalid input format. Please enter in 'department label' format." << std::endl;
                    return;
                }
                
                if(std::find(departments.begin(), departments.end(), label) != departments.end()){
                    std::cout << "Invalid department. Valid departments are: ";
                    for(const auto& validDepartment : departments){
                        std::cout << validDepartment << " ";
                    }
                    std::cout << std::endl;
                    return;
                }

                if(!validateLabel(label)){
                    std::cout << "Invalid label. Valid labels are: ";
                    for(const auto& validLabel : validLabels){
                        std::cout << validLabel << " ";
                    }
                    std::cout << std::endl;
                    return;
                }

                getInvestInformation(socket, department, label);
            }
            // Show previous data(financial statements table)
            else if(input == "2" || input == "Show" || input == "show"){
                std::cout<< "Enter all or department name: ";
                std::cin.ignore();
                std::getline(std::cin, input);

                if(!(std::find(departments.begin(), departments.end(), label) != departments.end()) || input != "all" || input != "All"){
                    std::cout << "Invalid department. Valid departments are: ";
                    for(const auto& validDepartment : departments){
                        std::cout << validDepartment << " ";
                    }
                    std::cout << std::endl;
                    return;
                }
                requestPreviousData(socket, input);
            }
            else if(input == "3" || input == "Exit" || input == "exit"){
                std::cout << "Exiting the program." << std::endl;
                break;
            }
            
            else{
                std::cout << "Invalid choice. Please enter 1, 2, or 3." << std::endl;
            }


        }
    }
    catch (std::exception& e) {
        std::cerr << "Client Error: " << e.what() << std::endl;
    }
}

void InvestorHandler::getInvestInformation(tcp::socket& socket, std::string& department, std::string& label){
    std::string message = department + " " + label;
    getInfoFromServer(socket, message);
}

void InvestorHandler::getInfoFromServer(tcp::socket& socket, std::string& message){
    // Send request type to server
    boost::asio::write(socket, boost::asio::buffer(std::string("INVESTOR")));

    sendToServer(socket, message);

    // Read response from server
    std::vector<char> buf(4096);
    size_t len = socket.read_some(boost::asio::buffer(buf));
    std::string response(buf.data(), len);
    std::cout << "Investment recommendation intensity " << response << std::endl;
}

void InvestorHandler::sendToServer(tcp::socket& socket, std::string& message) {
    boost::asio::write(socket, boost::asio::buffer(message));
}



bool InvestorHandler::validateLabel(const std::string& label) {
    return std::find(validLabels.begin(), validLabels.end(), label) != validLabels.end();
}

bool InvestorHandler::parseInput(const std::string& input, std::string& department, std::string& label) {
    std::istringstream iss(input);
    if (!(iss >> department >> label)) {
        return false;
    }
    return true;
}

void InvestorHandler::requestPreviousData(tcp::socket& socket, std::string& department){
    // Send request type to server
    boost::asio::write(socket, boost::asio::buffer(std::string("INVESTOR")));

    InvestorHandler::sendToServer(socket, department);

    // Read the response from the server
    std::vector<char> buf(4096);
    size_t len = socket.read_some(boost::asio::buffer(buf));
    std::string response(buf.data(), len);

    // Display the financial statements received from the server
    displayFinancialStatements(response);
}

void displayFinancialStatements(const std::string& data) {
    std::istringstream input(data); // Use istringstream to read from the input string
    std::string line;

    // Print each line of the data
    while (std::getline(input, line)) {
        std::cout << line << std::endl; // Output the line to the console
    }
}