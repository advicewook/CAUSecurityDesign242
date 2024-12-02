#include "Server.h"
#include <boost/asio.hpp>
#include <iostream>
#include "openfhe.h"

using namespace boost::asio;
using ip::tcp;
using namespace lbcrypto;


void handleClient(tcp::socket& socket, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& keyPair){
    try{
        //transfer pk to client
        std::stringstream ss;
        Serial::Serialize(keyPair.publicKey, ss, SerType::BINARY);
        std::string publicKeyString = ss.str();

        boost::asio::write(socket, boost::asio::buffer(publicKeyString));

        //Encrypted data from client
        std::vector<char> buf(4096);
        size_t len =socket.read_some(boost::asio::buffer(buf));
        std::string encryptedData(buf.data(), len);

        std::stringstream cipherStream(encryptedData);
        Ciphertext<DCRTPoly> ciphertext;
        Serial::Deserialize(ciphertext, cipherStream, SerType::BINARY);

        //Data decryption
        Plaintext plaintext;
        cc->Decrypt(keyPair.secretKey, ciphertext, &plaintext);

        std::cout << "Decrypted data: " << plaintext << std::endl;
    }
        catch (std::exception& e) {
        std::cerr << "Exception in handle_client: " << e.what() << std::endl;
    }
}

void startServer() {
    try {
        io_service ioService;
        tcp::acceptor acceptor(ioService, tcp::endpoint(tcp::v4(), 12345));

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

        auto keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);

        std::cout << "Server is running on port 12345..." << std::endl;


        while (true) {
            tcp::socket socket(ioService);
            acceptor.accept(socket);

            std::thread(handleClient, std::ref(socket), std::ref(cc), std::ref(keyPair)).detach();

            // char data[1024] = {0};
            // size_t length = socket.read_some(buffer(data));
            // std::cout << "Received from client: " << std::string(data, length) << std::endl;

            // std::string response = "Data received: " + std::string(data, length);
            // write(socket, buffer(response));
        }
    } catch (std::exception& e) {
        std::cerr << "Server Error: " << e.what() << std::endl;
    }
}


int main(){
    startServer();

    return 0;
}
