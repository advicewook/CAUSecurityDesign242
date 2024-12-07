#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <iostream>
#include "openfhe.h"

#include "DBManager.h"
#include "FinancialAnalyzer.h"

using namespace boost::asio;
using ip::tcp;
using namespace std;
using namespace lbcrypto;

void startServer(FinancialAnalyzer& FinancialAnalyzer, DBManager& DBManager );  // Declare the startServer function

void handleClient(tcp::socket& socket, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& keyPair, FinancialAnalyzer& financialAnalyzer, DBManager& DBManager);  // Declare the handleClient function

// Function to encode data into plaintext
Plaintext encodeData(const std::vector<double>& data, CryptoContext<DCRTPoly>& cc);

// Function to encrypt plaintext
Ciphertext<DCRTPoly> encryptPlaintext(Plaintext& plaintext, PublicKey<DCRTPoly>& publicKey, CryptoContext<DCRTPoly>& cc);

// Function to decrypt ciphertext
Plaintext decryptCiphertext(Ciphertext<DCRTPoly>& ciphertext, const PrivateKey<DCRTPoly>& secretKey, CryptoContext<DCRTPoly>& cc);

// Function to add two ciphertexts
Ciphertext<DCRTPoly> addCiphertexts(Ciphertext<DCRTPoly>& ct1, Ciphertext<DCRTPoly>& ct2, CryptoContext<DCRTPoly>& cc);

// Function to multiply two ciphertexts
Ciphertext<DCRTPoly> multiplyCiphertexts(Ciphertext<DCRTPoly>& ct1, Ciphertext<DCRTPoly>& ct2, CryptoContext<DCRTPoly>& cc);

// Function to perform scalar multiplication on ciphertext
Ciphertext<DCRTPoly> multiplyCiphertextByScalar(Ciphertext<DCRTPoly>& ct, double scalar, CryptoContext<DCRTPoly>& cc);

// Function to perform homomorphic rotation
Ciphertext<DCRTPoly> rotateCiphertext(Ciphertext<DCRTPoly>& ct, int steps, CryptoContext<DCRTPoly>& cc);

// Function to perform bootstrapping
Ciphertext<DCRTPoly> bootstrapCiphertext(Ciphertext<DCRTPoly>& ct, CryptoContext<DCRTPoly>& cc);

// Function to send encrypted data to the client
void sendEncryptedData(tcp::socket& socket, Ciphertext<DCRTPoly>& ciphertext);

// Function to receive encrypted data from the client
Ciphertext<DCRTPoly> receiveEncryptedData(tcp::socket& socket, CryptoContext<DCRTPoly>& cc);

// Function to decrypt ciphertext and print the result
void decryptAndPrint(Ciphertext<DCRTPoly>& ciphertext, const PrivateKey<DCRTPoly>& secretKey, CryptoContext<DCRTPoly>& cc);

void provideInvestmentData(tcp::socket& socket);


void serverSQL();

void sendPublicKey(tcp::socket& socket, PublicKey<DCRTPoly>& publicKey);

void handleDepartmentRequest(tcp::socket& socket, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& keyPair, DBManager& DBManager);

void handleInvestorRequest(tcp::socket& socket, FinancialAnalyzer& financialAnalyzer);

bool saveEncryptedDataToDatabase(const std::string& encryptedData);

#endif
