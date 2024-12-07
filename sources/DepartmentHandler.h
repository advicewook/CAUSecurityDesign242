#ifndef DEPARTMENT_HANDLER_H
#define DEPARTMENT_HANDLER_H

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <iostream>
#include "openfhe.h"


using namespace boost::asio;
using ip::tcp;
using namespace std;
using namespace lbcrypto;

class DepartmentHandler {
public: 

DepartmentHandler(const std::string& departmentName);  // Constructor to initialize the department name
void run();

    
private:
std::string department;
std::vector<std::string> departments = {"HR", "Finance", "IT", "Marketing"}; // Registered departments
bool exitRequested = false;



void connectToServerEncrypted(tcp::socket& socket, const std::string& ip, const std::string& port);  // Declare the connetToServerUnEncrypted function
void connectToServerEncrypted();  // Declare the connetToServerUnEncrypted function

void inputInternalData(tcp::socket& socket, PublicKey<DCRTPoly>& publicKey, CryptoContext<DCRTPoly>& cc);  // Declare the inputInternalData function

bool validateLabel(const std::string& label);
bool parseInput(const std::string& input, std::string& label, std::string& data);
std::vector<std::string> validLabels = {"Revenue", "CostofGoodsSold", "OperatingExpenses", "NonOperatingExpenses", 
                                        "CurrentAssets", "NonCurrentAssets", "CurrentLiability", "NonCurrentLiability", 
                                        "CapitalStock", "RetainedEarning"};


void sendToServer(tcp::socket& socket, const std::string& message);  // Declare the sendToServer function

void sendEncryptedData(tcp::socket& socket, Ciphertext<DCRTPoly>& ciphertext);  // Declare the sendEncryptedData function

void sendEncryptedLabelNData(const std::string& label, int64_t data, tcp::socket& socket, PublicKey<DCRTPoly>& publicKey, CryptoContext<DCRTPoly>& cc);

Plaintext encodeData(const std::vector<double>& data, CryptoContext<DCRTPoly>& cc);  // Declare the encodeData function


Ciphertext<DCRTPoly> encryptPlaintext(Plaintext& plaintext, PublicKey<DCRTPoly>& publicKey, CryptoContext<DCRTPoly>& cc);  // Declare the encryptPlaintext function

Ciphertext<DCRTPoly> encryptNumber(int64_t number, PublicKey<DCRTPoly>& publicKey, CryptoContext<DCRTPoly>& cc);

Plaintext decryptCiphertext(Ciphertext<DCRTPoly>& ciphertext, const PrivateKey<DCRTPoly>& secretKey, CryptoContext<DCRTPoly>& cc);  // Declare the decryptCiphertext function

std::vector<double> decryptCiphertextVector(Ciphertext<DCRTPoly>& ciphertext, const PrivateKey<DCRTPoly>& secretKey, CryptoContext<DCRTPoly>& cc);  // Declare the decryptCiphertext 
        
};

#endif // DEPARTMENT_HANDLER_H
