#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <iostream>
#include <memory>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>
#include <string>
#include <sstream>
#include "openfhe.h"  // OpenFHE 관련 헤더


using namespace std;
using namespace lbcrypto;

class DBManager {
public:
    DBManager(const std::string& server, const std::string& user, const std::string& password, const std::string& database);
    ~DBManager();

    void createTables();
    Ciphertext<DCRTPoly> getCurrentCiphertextFromDepartment(const std::string& department, const std::string& label);
    bool saveEncryptedDataToDepartment(const std::string& department, const std::string& label, const Ciphertext<DCRTPoly>& ciphertext);
    Ciphertext<DCRTPoly> addCiphertexts(Ciphertext<DCRTPoly>& ct1, Ciphertext<DCRTPoly>& ct2, CryptoContext<DCRTPoly>& cc);


    std::vector<std::vector<Ciphertext<DCRTPoly>>> DBManager::fetchDepartmentData();
    bool DBManager::insertAnalyzedData(const std::string& departmentName, const std::vector<Ciphertext<DCRTPoly>>& encryptedData);

    void DBManager::decryptAndInsertFinancialStatements(CryptoContext<DCRTPoly>& cc, PrivateKey<DCRTPoly>& secretKey, int year, int quarter);

    double calculateGrowthRate(const std::string& department, const std::string& label);

    void printFinancialStatements();
    void printFinancialStatementsByDepartment(const std::string& departmentName);

    std::string getFinancialStatementsByDepartment(const std::string& departmentName);
    std::string getFinancialStatements();



private:
    sql::mysql::MySQL_Driver* driver_;
    std::unique_ptr<sql::Connection> conn_;
    std::string server_;
    std::string user_;
    std::string password_;
    std::string database_;


};

#endif // DBMANAGER_H