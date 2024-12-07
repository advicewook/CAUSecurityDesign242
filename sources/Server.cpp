#include "Server.h"
#include <boost/asio.hpp>
#include <iostream>
#include "openfhe.h"

#include <thread>
#include <chrono>

#include <mysql.h>

#include "mysql_driver.h" 
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>

using namespace std;

#ifdef _DEBUG
#pragma comment(lib, "mysqlcppconn.lib")
#else
#pragma comment(lib, "MySQL\\Release\\mysqlcppconn.lib")
#endif

using namespace boost::asio;
using ip::tcp;
using namespace lbcrypto;
using namespace std;

void sendPublicKey(tcp::socket& socket, const PublicKey<DCRTPoly>& publicKey) {
    std::stringstream ss;
    Serial::Serialize(publicKey, ss, SerType::BINARY);
    std::string publicKeyString = ss.str();
    boost::asio::write(socket, boost::asio::buffer(publicKeyString));
}


//데이터를 넣으면서 analyzedData도 갱신
void handleDepartmentRequest(tcp::socket& socket, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& keyPair, DBManager& DBManager) {
    // Encrypted data from client
    std::vector<char> buf(4096);
    size_t len = socket.read_some(boost::asio::buffer(buf));
    std::string encryptedData(buf.data(), len);

    // Deserialize the data
    std::stringstream cipherStream(encryptedData);
    std::string department, label;
    cipherStream >> department >> label;

    // Deserialize the new ciphertext *newCiphertext == Data from the client
    Ciphertext<DCRTPoly> newCiphertext;
    Serial::Deserialize(newCiphertext, cipherStream, SerType::BINARY);

    // Get the current ciphertext from the database *currentCipherText == Data from the database
    Ciphertext<DCRTPoly> currentCiphertext = DBManager.getCurrentCiphertextFromDepartment(department, label);

    // Add the two ciphertexts
    Ciphertext<DCRTPoly> updatedCiphertext = DBManager.addCiphertexts(currentCiphertext, newCiphertext, cc);

    // Save the updated ciphertext back to the database
    if (DBManager.saveEncryptedDataToDepartment(department, label, updatedCiphertext)) {
        std::cout << "Encrypted result successfully updated in the department table." << std::endl;
    } else {
        std::cerr << "Failed to update encrypted result in the department table." << std::endl;
    }
}

void handleInvestorRequest(tcp::socket& socket, FinancialAnalyzer& FinancialAnalyzer) {
    // Read the message from client (department + " " + label)
    std::vector<char> buf(4096);
    size_t len = socket.read_some(boost::asio::buffer(buf));
    std::string message(buf.data(), len);

    // Process the message (department and label)
    std::cout << "Investor request: " << message << std::endl;

    std::istringstream iss(message);
    std::string department, label;
    iss >> department >> label;
    //if label is empty, return previous data
    if(label == "") {
        if(department == "all" || department == "All") {
            //Using DBManager, get all InvestInfo from DB and return to client
            std::string response = FinancialAnalyzer.displayPreviousData();
        } else {
            //Using DBManager, get InvestInfo from DB and return to client
            std::string response = FinancialAnalyzer.displaypreviousData(department);
        }
    }
    else{
    //Using DBManager, get InvestInfo from DB and return to client
    std::string response = FinancialAnalyzer.getInvestInfo(message);
    }
    // Send response back to client
    std::string response = "Processed investor request for: " + message;
    boost::asio::write(socket, boost::asio::buffer(response));
}


void handleClient(tcp::socket socket, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& keyPair, FinancialAnalyzer& financialAnalyzer, DBManager& DBManager) {
    bool keySent = false; // 각 클라이언트 스레드 내에서 키 전송 여부 추적

    try {
        while (true) {
            // Read request type from client
            std::vector<char> buf(4096);
            size_t len = socket.read_some(boost::asio::buffer(buf));
            std::string request(buf.data(), len);

            if (request == "DEPARTMENT") {
                // 공용 키를 클라이언트에 전송할지 여부 확인
                if (!keySent) {
                    // Send public key to client for department data handling
                    sendPublicKey(socket, keyPair.publicKey);
                    keySent = true; // 키가 전송되었음을 표시
                    continue; // 첫 요청에서는 키만 전송하고 다음 루프로 넘어감
                }
                // 키가 전송된 후에는 부서 요청을 처리
                handleDepartmentRequest(socket, cc, keyPair, DBManager);
            } else if (request == "INVESTOR") {
                handleInvestorRequest(socket, financialAnalyzer);
            } else {
                std::cerr << "Unknown request type: " << request << std::endl;
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Exception in handleClient: " << e.what() << std::endl;
    }
}

void startServer(FinancialAnalyzer& FinancialAnalyzer, DBManager& DBManager) {
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

            std::thread([&socket, &cc, &keyPair, &FinancialAnalyzer, &DBManager]() {
                handleClient(std::move(socket), cc, keyPair, FinancialAnalyzer, DBManager);
            }).detach();

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


void sendEncryptedData(tcp::socket& socket, Ciphertext<DCRTPoly>& ciphertext) {
    std::stringstream ss;
    Serial::Serialize(ciphertext, ss, SerType::BINARY);
    std::string serializedCiphertext = ss.str();
    boost::asio::write(socket, boost::asio::buffer(serializedCiphertext));
}

Ciphertext<DCRTPoly> receiveEncryptedData(tcp::socket& socket, CryptoContext<DCRTPoly>& cc) {
    std::vector<char> buf(4096);
    size_t len = socket.read_some(boost::asio::buffer(buf));
    std::string ciphertextStr(buf.data(), len);
    
    std::stringstream ss(ciphertextStr);
    Ciphertext<DCRTPoly> ciphertext;
    Serial::Deserialize(ciphertext, ss, SerType::BINARY);
    return ciphertext;
}

// void decryptAndPrint(Ciphertext<DCRTPoly>& ciphertext, const PrivateKey<DCRTPoly>& secretKey, CryptoContext<DCRTPoly>& cc) {
//     // 복호화
//     Plaintext plaintext = decryptCiphertext(ciphertext, secretKey, cc);
    
//     // 평문에서 숫자 추출
//     vector<double> result;
//     plaintext->GetRealPackedValue(result);
    
//     // 결과 출력
//     cout << "복호화된 숫자: " << result[0] << endl;
// }

/////////////////////DB////////////////////////

void serverSQL(){
    
    // Start the server in a separate thread
    std::thread serverThread(startServer);

    // Wait for the server to start
    std::this_thread::sleep_for(std::chrono::seconds(1));

    sql::mysql::MySQL_Driver* driver;


    try {
        // sql driver 가져오기
        driver = sql::mysql::get_driver_instance();

        // sql server 연결
        const string server = "tcp://127.0.0.1:3306";
        const string name = "root";
        const string password = "dlaalsgur5162!";
        std::unique_ptr<sql::Connection> conn(driver->connect(server, name, password));
        
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        
        //database 없으면 생성
        stmt->execute("create database if not exists securityproj");

        //db 연결
        conn->setSchema("securityproj");


        // 만약 sql 에 table이 없다면 생성

        // encrypted input data
        stmt->execute("create table if not exists department(departmentName varchar(20) not null,\
                                                            revenue bigint null,\
                                                            CostofGoodsSold bigint null,\
                                                            OperatingExpenses bigint null,\
                                                            NonOperatingExpenses bigint null,\
                                                            CurrentAssets bigint null,\
                                                            NonCurrentAssets bigint null,\
                                                            CurrentLiability bigint null,\
                                                            NonCurrentLiability bigint null,\
                                                            CapitalStock bigint null,\
                                                            RetainedEarning bigint null\
                                                            )");


        //encrypted calculated data
        stmt->execute("create table if not exists analyzedData(departmentName varchar(20) not null,\
                                                            GrossProfit bigint null,\
                                                            OperatingProfit bigint null,\
                                                            NetProfit bigint null,\
                                                            TotalAssets bigint null, \
                                                            TotalLiabilities bigint null,\
                                                            TotalEquity bigint null\
                                                            )");
                                                        
        // each dapartment's public key
        stmt->execute("create table if not exists departmentKey(departmentName varchar(20) not null,\
                                                            publicKey bigint null\
                                                            )");

        // final financial table (decrypted)
        stmt->execute("create table if not exists financialStatements(departmentName varchar(20) not null,\
                                                            year int null,\
                                                            quater int null,\
                                                            revenue int null,\
                                                            CostofGoodsSold int null,\
                                                            OperatingExpenses int null,\
                                                            NonOperatingExpenses int null,\
                                                            CurrentAssets int null,\
                                                            NonCurrentAssets int null,\
                                                            CurrentLiability int null,\
                                                            NonCurrentLiability int null,\
                                                            CapitalStock int null,\
                                                            RetainedEarning int null,\
                                                            GrossProfit int null,\
                                                            OperatingProfit int null,\
                                                            NetProfit int null,\
                                                            TotalAssets int null, \
                                                            TotalLiabilities int null,\
                                                            TotalEquity int null\
                                                            )");


        // 각 table의 값 전부 읽어오기

        std::unique_ptr<sql::ResultSet> department(stmt->executeQuery("SELECT * FROM department"));
        std::unique_ptr<sql::ResultSet> deparmentKey(stmt->executeQuery("SELECT * FROM departmentkey"));
        std::unique_ptr<sql::ResultSet> analyzedData(stmt->executeQuery("SELECT * FROM analyzeddata"));
        std::unique_ptr<sql::ResultSet> finanacialStatement(stmt->executeQuery("SELECT * FROM analyzeddata"));

        // 출력
        while (department && department->next()) {
            std::cout << department->getString(1) << " " << department->getString(2) << std::endl;
        }

    }
    catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQL state: " << e.getSQLState() << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
    }

    // Wait for the server thread to finish
    serverThread.join();

}

// Ciphertext<DCRTPoly> getCurrentCiphertextFromDepartment(const std::string& department, const std::string& label) {
//     sql::mysql::MySQL_Driver* driver;
//     Ciphertext<DCRTPoly> ciphertext;

//     try {
//         driver = sql::mysql::get_driver_instance();
//         std::unique_ptr<sql::Connection> conn(driver->connect("tcp://127.0.0.1:3306", "root", "dlaalsgur5162!"));
//         conn->setSchema("securityproj");

//         // Prepare the SQL statement to get the current value
//         std::string query = "SELECT " + label + " FROM department WHERE departmentName = ?";
//         std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(query));
//         pstmt->setString(1, department);

//         std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
//         if (res->next()) {
//             // Assuming the label corresponds to the first column
//             std::string encryptedData = res->getString(label);
//             std::stringstream ss(encryptedData);
//             Serial::Deserialize(ciphertext, ss, SerType::BINARY);
//         }
//     } catch (sql::SQLException& e) {
//         std::cerr << "SQLException: " << e.what() << std::endl;
//     } catch (std::exception& e) {
//         std::cerr << "Exception: " << e.what() << std::endl;
//     }

//     return ciphertext;
// }

// bool saveEncryptedDataToDepartment(const std::string& department, const std::string& label, const Ciphertext<DCRTPoly>& ciphertext) {
//     sql::mysql::MySQL_Driver* driver;

//     try {
//         driver = sql::mysql::get_driver_instance();
//         std::unique_ptr<sql::Connection> conn(driver->connect("tcp://127.0.0.1:3306", "root", "dlaalsgur5162!"));
//         conn->setSchema("securityproj");

//         // Prepare the SQL statement to update the specified label in the department table
//         std::string query = "UPDATE department SET " + label + " = ? WHERE departmentName = ?";
//         std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(query));
//         std::stringstream ss;
//         Serial::Serialize(ciphertext, ss, SerType::BINARY);
//         pstmt->setString(1, ss.str());
//         pstmt->setString(2, department);
//         pstmt->executeUpdate();

//         return true;
//     } catch (sql::SQLException& e) {
//         std::cerr << "SQLException: " << e.what() << std::endl;
//         return false;
//     } catch (std::exception& e) {
//         std::cerr << "Exception: " << e.what() << std::endl;
//         return false;
//     }
// }


// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Ciphertext<DCRTPoly> addCiphertexts(Ciphertext<DCRTPoly>& ct1, Ciphertext<DCRTPoly>& ct2, CryptoContext<DCRTPoly>& cc) {
//     return cc->EvalAdd(ct1, ct2);
// }

Plaintext encodeData(const std::vector<double>& data, CryptoContext<DCRTPoly>& cc) {
    Plaintext plaintext;
    plaintext = cc->MakeCKKSPackedPlaintext(data);
    return plaintext;
}

Ciphertext<DCRTPoly> encryptPlaintext(Plaintext& plaintext, PublicKey<DCRTPoly>& publicKey, CryptoContext<DCRTPoly>& cc) {
    return cc->Encrypt(publicKey, plaintext);
}

Plaintext decryptCiphertext(Ciphertext<DCRTPoly>& ciphertext, const PrivateKey<DCRTPoly>& secretKey, CryptoContext<DCRTPoly>& cc) {

    Plaintext decrypted_ptx;
    cc-> Decrypt(secretKey, ciphertext, &decrypted_ptx);
    //decrypted_text->SetLength(4);

    std::vector<int64_t> decrypted_msg = decrypted_ptx->GetPackedValue();

    std::cout << "decrypted msg: " << decrypted_msg << std::endl;

    return decrypted_ptx;
    }

Ciphertext<DCRTPoly> multiplyCiphertexts(Ciphertext<DCRTPoly>& ct1, Ciphertext<DCRTPoly>& ct2, CryptoContext<DCRTPoly>& cc) {
    return cc->EvalMult(ct1, ct2);
}

Ciphertext<DCRTPoly> multiplyCiphertextByScalar(Ciphertext<DCRTPoly>& ct, double scalar, CryptoContext<DCRTPoly>& cc) {
    return cc->EvalMult(ct, scalar);
}

// Ciphertext<DCRTPoly> rotateCiphertext(Ciphertext<DCRTPoly>& ct, int steps, CryptoContext<DCRTPoly>& cc) {
//     return cc->Rotate(ct, steps);
// }

Ciphertext<DCRTPoly> bootstrapCiphertext(Ciphertext<DCRTPoly>& ct, CryptoContext<DCRTPoly>& cc) {
    return cc->EvalBootstrap(ct);
}

int main(){

    serverSQL();


    DBManager dbManager = DBManager("tcp://127.0.0.1:3306", "root", "dlaalsgur5162!", "securityproj");
    FinancialAnalyzer financialAnalyzer = FinancialAnalyzer(dbManager);


    startServer(financialAnalyzer, dbManager);

    return 0;
}
