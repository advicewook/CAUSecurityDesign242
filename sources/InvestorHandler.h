#ifndef INVESTOR_HANDLER_H
#define INVESTOR_HANDLER_H

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <iostream>

using namespace boost::asio;
using ip::tcp;
using namespace std;

class InvestorHandler {
public:
    void run();

private:
std::vector<std::string> departments = {"Company", "HR", "Finance", "IT", "Marketing"}; // Registered departments
std::vector<std::string> validLabels = {"Revenue", "CostofGoodsSold", "OperatingExpenses", "NonOperatingExpenses", 
                                        "CurrentAssets", "NonCurrentAssets", "CurrentLiability", "NonCurrentLiability", 
                                        "CapitalStock", "RetainedEarning"};


void connectToServerUnEncrypted();  // Declare the connetToServerUnEncrypted function

void getInvestInformation(tcp::socket& socket, std::string& department, std::string& label);

void getInfoFromServer(tcp::socket& socket, std::string& message);

bool validateLabel(const std::string& label);

bool parseInput(const std::string& input, std::string& department, std::string& label);

void requestPreviousData(tcp::socket& socket, std::string& department);

void displayFinancialStatements(const std::string& data);

void displayFinancialStatements(const std::string& data);

void sendToServer(tcp::socket& socket, std::string& message);  // Declare the sendToServer function

};

#endif; // INVESTOR_HANDLER_H