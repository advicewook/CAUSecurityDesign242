#include "DepartmentHandler.h"


DepartmentHandler::DepartmentHandler(const std::string& departmentName) : department(departmentName) {
    if (std::find(departments.begin(), departments.end(), departmentName) == departments.end()) {
        throw std::invalid_argument("Department not registered.");
    }
}

void DepartmentHandler::run() {
    connectToServerEncrypted(); // connect server and input data in this function

    // while (!exitRequested) {
    //     inputInternalData();
    // }
}

void DepartmentHandler::connectToServerEncrypted() {
    try {
            boost::asio::io_context io_context;
            tcp::resolver resolver(io_context);
            tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "12345");
            tcp::socket socket(io_context);
            boost::asio::connect(socket, endpoints);

            // department receive pk from server by sending department request signal
            boost::asio::write(socket, boost::asio::buffer(std::string("DEPARTMENT")));
            std::vector<char> buf(4096);
            size_t len = socket.read_some(boost::asio::buffer(buf));
            std::string publicKeyString(buf.data(), len);

            std::cout << "Connected to server! department: " << department << std::endl;

            // std::vector<char> buf(4096);
            // size_t len = socket.read_some(boost::asio::buffer(buf));
            // std::string publicKeyStr(buf.data(), len);

            std::stringstream ss(publicKeyString);
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

            while (!exitRequested) {
                inputInternalData(socket, publicKey, cc);
            }

    } catch (std::exception& e) {
        std::cerr << "Client Error: " << e.what() << std::endl;
    }
}



void DepartmentHandler::inputInternalData(tcp::socket& socket, PublicKey<DCRTPoly>& publicKey, CryptoContext<DCRTPoly>& cc) {
    std::string input, label, dataStr;

    std::cout << "Enter label and data (e.g., Revenue 5000(Default unit: 1000)) (or type 'exit' to stop): ";
    std::cin.ignore(); // Clear the newline character from the input buffer
    std::getline(std::cin, input);

    if (department == "Exit" || department == "exit") {
        exitRequested = true;
        return;
    }

    if (!parseInput(input, label, dataStr)) {
        std::cout << "Invalid input format. Please enter in 'label data' format." << std::endl;
        return;
    }

    if (!validateLabel(label)) {
        std::cout << "Invalid label. Valid labels are: ";
        for (const auto& validLabel : validLabels) {
            std::cout << validLabel << " ";
        }
        std::cout << std::endl;
        return;
    }


    int64_t data;
    try {
        data = std::stoll(dataStr);
    } catch (const std::invalid_argument& e) {
        std::cerr << "유효하지 않은 인수: " << e.what() << std::endl;
        return;
    } catch (const std::out_of_range& e) {
        std::cerr << "범위를 벗어난 값: " << e.what() << std::endl;
        return;
    }

    sendEncryptedLabelNData(label, data, socket, publicKey, cc);

}

bool DepartmentHandler::validateLabel(const std::string& label) {
    return std::find(validLabels.begin(), validLabels.end(), label) != validLabels.end();
}

bool DepartmentHandler::parseInput(const std::string& input, std::string& label, std::string& data) {
    std::istringstream iss(input);
    if (!(iss >> label >> data)) {
        return false;
    }
    return true;
}

void sendToServer(tcp::socket& socket, const std::string& message) {
    boost::asio::write(socket, boost::asio::buffer(message));
}

void DepartmentHandler::sendEncryptedLabelNData(const std::string& label, int64_t data, tcp::socket& socket, PublicKey<DCRTPoly>& publicKey, CryptoContext<DCRTPoly>& cc) {

     // Send request type to server
    boost::asio::write(socket, boost::asio::buffer(std::string("DEPARTMENT")));

    // Encrypt the number
    auto ciphertext = encryptNumber(data, publicKey, cc);

    // Serialize the department name, label, and ciphertext
    std::stringstream ss;
    ss << department << " " << label << " ";
    Serial::Serialize(ciphertext, ss, SerType::BINARY);
    std::string serializedMessage = ss.str();

    // Send the serialized data to the server
    sendToServer(socket, serializedMessage);
}

Ciphertext<DCRTPoly> encryptNumber(int64_t number, PublicKey<DCRTPoly>& publicKey, CryptoContext<DCRTPoly>& cc) {
    // Encode the number as a plaintext
    std::vector<double> data = { static_cast<double>(number) };
    Plaintext plaintext = encodeData(data, cc);

    // Encrypt the plaintext
    return encryptPlaintext(plaintext, publicKey, cc);
}

Plaintext encodeData(const std::vector<double>& data, CryptoContext<DCRTPoly>& cc) {
    Plaintext plaintext;
    plaintext = cc->MakeCKKSPackedPlaintext(data);
    return plaintext;
}

Ciphertext<DCRTPoly> encryptPlaintext(Plaintext& plaintext, PublicKey<DCRTPoly>& publicKey, CryptoContext<DCRTPoly>& cc) {
    return cc->Encrypt(publicKey, plaintext);
}



