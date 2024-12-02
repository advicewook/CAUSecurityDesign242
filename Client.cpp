#include "Client.h"
#include <boost/asio.hpp>
#include <iostream>
#include "openfhe.h"

using namespace boost::asio;
using ip::tcp;
using namespace std;
using namespace lbcrypto;



void startClient() {
    try {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "12345");
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        // receive pk from server
        std::vector<char> buf(4096);
        size_t len = socket.read_some(boost::asio::buffer(buf));
        std::string publicKeyStr(buf.data(), len);

        std::stringstream ss(publicKeyStr);
        PublicKey<DCRTPoly> publicKey;
        Serial::Deserialize(publicKey, ss, SerType::BINARY);

        uint32_t multDepth = 31;
        uint32_t scaleModSize = 59;
        uint32_t firstModSize = 60;
        uint32_t batchSize = 4096;

        SecurityLevel securityLevel = HEStd_128_classic;

        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(multDepth);
        parameters.SetScalingModSize(scaleModSize);
        parameters.SetFirstModSize(firstModSize);
        parameters.SetBatchSize(batchSize);
        parameters.SetSecurityLevel(securityLevel);

        CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(LEVELEDSHE);
        cc->Enable(KEYSWITCH);
        cc->Enable(ADVANCEDSHE);
        cc->Enable(FHE);

        //calculate encryption 
        Plaintext plaintext = cc->MakeCKKSPackedPlaintext({1.0, 2.0, 3.0, 4.0});
        auto ciphertext = cc->Encrypt(publicKey, plaintext);


        //send encrypted data to server
        std::stringstream cipherStream;
        Serial::Serialize(ciphertext, cipherStream, SerType::BINARY);
        std::string encryptedData = cipherStream.str();

        boost::asio::write(socket, boost::asio::buffer(encryptedData));

        // std::cout << "Connected to server!" << std::endl;

        // std::string message = "Hello, Server!";
        // write(socket, buffer(message));
        // std::cout << "Sent to server: " << message << std::endl;

        // char response[1024] = {0};
        // size_t length = socket.read_some(buffer(response));
        // std::cout << "Received from server: " << std::string(response, length) << std::endl;

    } catch (std::exception& e) {
        std::cerr << "Client Error: " << e.what() << std::endl;
    }
}



int main() {
    private PublicKey<DCRTPoly> publicKey;



    startClient();
    return 0;
}