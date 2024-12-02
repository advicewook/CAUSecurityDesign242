#ifndef SERVER_H
#define SERVER_H

void startServer();  // Declare the startServer function

void handleClient(tcp::socket& socket, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& keyPair);  // Declare the handleClient function

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



#endif
