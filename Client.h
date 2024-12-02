#ifndef CLIENT_H
#define CLIENT_H

#include <boost/asio.hpp>
#include <iostream>
#include "openfhe.h"


using namespace boost::asio;
using ip::tcp;
using namespace std;
using namespace lbcrypto;

void startClient();  // Declare the startClient function

void sendToServer(tcp::socket& socket, const std::string& message);  // Declare the sendToServer function

void sendEncryptedData(tcp::socket& socket, Ciphertext<DCRTPoly>& ciphertext);  // Declare the sendEncryptedData function

Plaintext encodeData(const std::vector<double>& data, CryptoContext<DCRTPoly>& cc);  // Declare the encodeData function


Ciphertext<DCRTPoly> encryptPlaintext(Plaintext& plaintext, PublicKey<DCRTPoly>& publicKey, CryptoContext<DCRTPoly>& cc);  // Declare the encryptPlaintext function

Ciphertext<DCRTPoly> encryptNumber(int64_t number, PublicKey<DCRTPoly>& publicKey, CryptoContext<DCRTPoly>& cc);

Plaintext decryptCiphertext(Ciphertext<DCRTPoly>& ciphertext, const PrivateKey<DCRTPoly>& secretKey, CryptoContext<DCRTPoly>& cc);  // Declare the decryptCiphertext function

std::vector<double> decryptCiphertextVector(Ciphertext<DCRTPoly>& ciphertext, const PrivateKey<DCRTPoly>& secretKey, CryptoContext<DCRTPoly>& cc);  // Declare the decryptCiphertext function



#endif
