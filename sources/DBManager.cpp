#include "DBManager.h"

DBManager::DBManager(const std::string& server, const std::string& user, const std::string& password, const std::string& database)
    : server_(server), user_(user), password_(password), database_(database) {
    driver_ = sql::mysql::get_driver_instance();
    conn_ = std::unique_ptr<sql::Connection>(driver_->connect(server_, user_, password_));
    conn_->setSchema(database_);
}

DBManager::~DBManager() {
    conn_->close();
}

void DBManager::createTables() {
    std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
    stmt->execute("CREATE TABLE IF NOT EXISTS department(departmentName varchar(20) not null,"
                "revenue bigint null,"
                "CostofGoodsSold bigint null,"
                "OperatingExpenses bigint null,"
                "NonOperatingExpenses bigint null,"
                "CurrentAssets bigint null,"
                "NonCurrentAssets bigint null,"
                "CurrentLiability bigint null,"
                "NonCurrentLiability bigint null,"
                "CapitalStock bigint null,"
                "RetainedEarning bigint null);");

    stmt->execute("CREATE TABLE IF NOT EXISTS analyzedData(departmentName varchar(20) not null,"
                "GrossProfit bigint null,"
                "OperatingProfit bigint null,"
                "NetProfit bigint null,"
                "TotalAssets bigint null,"
                "TotalLiabilities bigint null,"
                "TotalEquity bigint null);");

    stmt->execute("CREATE TABLE IF NOT EXISTS departmentKey(departmentName varchar(20) not null,"
                "publicKey bigint null);");

    stmt->execute("CREATE TABLE IF NOT EXISTS financialStatements(departmentName varchar(20) not null,"
                "year int null,"
                "quater int null,"
                "revenue int null,"
                "CostofGoodsSold int null,"
                "OperatingExpenses int null,"
                "NonOperatingExpenses int null,"
                "CurrentAssets int null,"
                "NonCurrentAssets int null,"
                "CurrentLiability int null,"
                "NonCurrentLiability int null,"
                "CapitalStock int null,"
                "RetainedEarning int null,"
                "GrossProfit int null,"
                "OperatingProfit int null,"
                "NetProfit int null,"
                "TotalAssets int null,"
                "TotalLiabilities int null,"
                "TotalEquity int null);");
}


Ciphertext<DCRTPoly> DBManager::getCurrentCiphertextFromDepartment(const std::string& department, const std::string& label) {
    Ciphertext<DCRTPoly> ciphertext;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(conn_->prepareStatement("SELECT " + label + " FROM department WHERE departmentName = ?"));
        pstmt->setString(1, department);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next()) {
            std::string encryptedData = res->getString(label);
            std::stringstream ss(encryptedData);
            Serial::Deserialize(ciphertext, ss, SerType::BINARY);
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return ciphertext;
}

bool DBManager::saveEncryptedDataToDepartment(const std::string& department, const std::string& label, const Ciphertext<DCRTPoly>& ciphertext) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(conn_->prepareStatement("UPDATE department SET " + label + " = ? WHERE departmentName = ?"));
        std::stringstream ss;
        Serial::Serialize(ciphertext, ss, SerType::BINARY);
        pstmt->setString(1, ss.str());
        pstmt->setString(2, department);
        pstmt->executeUpdate();
        return true;
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
        return false;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
}

Ciphertext<DCRTPoly> DBManager::addCiphertexts(Ciphertext<DCRTPoly>& ct1, Ciphertext<DCRTPoly>& ct2, CryptoContext<DCRTPoly>& cc) {
    return cc->EvalAdd(ct1, ct2);
}


double DBManager::calculateGrowthRate(const std::string& department, const std::string& label) {
    long long currentValue = 0;
    long long previousValue = 0;

    try {
        // Step 1: Fetch the latest year and quarter data from financialStatements
        std::unique_ptr<sql::PreparedStatement> pstmt(conn_->prepareStatement(
            "SELECT year, quater, " + label + " FROM financialStatements "
            "WHERE departmentName = ? "
            "ORDER BY year DESC, quater DESC LIMIT 1"));
        pstmt->setString(1, department);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res->next()) {
            currentValue = res->getInt64(label); // 현재 값
        }

        // Step 2: Fetch the previous year and quarter data from financialStatements
        std::unique_ptr<sql::PreparedStatement> prevPstmt(conn_->prepareStatement(
            "SELECT " + label + " FROM financialStatements "
            "WHERE departmentName = ? AND (year, quater) < (SELECT year, quater FROM financialStatements "
            "WHERE departmentName = ? ORDER BY year DESC, quater DESC LIMIT 1) "
            "ORDER BY year DESC, quater DESC LIMIT 1"));
        prevPstmt->setString(1, department);
        prevPstmt->setString(2, department);
        std::unique_ptr<sql::ResultSet> prevRes(prevPstmt->executeQuery());

        if (prevRes->next()) {
            previousValue = prevRes->getInt64(label); // 이전 값
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    // Step 3: Calculate growth rate
    if (previousValue == 0) {
        return (currentValue > 0) ? 100.0 : 0.0; // 이전 값이 0일 경우, 현재 값이 양수면 100% 성장으로 간주
    }
    return ((static_cast<double>(currentValue) - previousValue) / previousValue) * 100.0;
}


// 부서 데이터 가져오기 - 사용할 부분 확인하기! 어디서 analyzedData를 갱신할 것인가.. 
std::vector<std::vector<Ciphertext<DCRTPoly>>> DBManager::fetchDepartmentData() {
    std::vector<std::vector<Ciphertext<DCRTPoly>>> encryptedData;

    try {
        std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT departmentName, revenue, CostofGoodsSold, OperatingExpenses, "
                                                                "NonOperatingExpenses, CurrentAssets, NonCurrentAssets, "
                                                                "CurrentLiability, NonCurrentLiability, CapitalStock, "
                                                                "RetainedEarning FROM department"));

        while (res->next()) {
            std::vector<Ciphertext<DCRTPoly>> row;
            std::string departmentName = res->getString("departmentName");
            // row.push_back(departmentName);  // 부서 이름은 문자열로 저장

            // 암호화된 데이터 가져오기
            for (const auto& field : {"revenue", "CostofGoodsSold", "OperatingExpenses", "NonOperatingExpenses",
                                    "CurrentAssets", "NonCurrentAssets", "CurrentLiability", 
                                    "NonCurrentLiability", "CapitalStock", "RetainedEarning"}) {
                std::string encryptedData = res->getString(field);
                std::stringstream ss(encryptedData);
                Ciphertext<DCRTPoly> ciphertext;
                Serial::Deserialize(ciphertext, ss, SerType::BINARY);
                row.push_back(ciphertext);
            }

            encryptedData.push_back(row);
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return encryptedData;
}

// 분석된 데이터 삽입 - 사용할 부분 확인하기!
bool DBManager::insertAnalyzedData(const std::string& departmentName, const std::vector<Ciphertext<DCRTPoly>>& encryptedData) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(conn_->prepareStatement("INSERT INTO analyzedData (departmentName, GrossProfit, "
                                                                            "OperatingProfit, NetProfit, TotalAssets, TotalLiabilities, "
                                                                            "TotalEquity) VALUES (?, ?, ?, ?, ?, ?, ?)"));
        pstmt->setString(1, departmentName);

        // 암호화된 데이터 각각을 업데이트
        for (size_t i = 0; i < encryptedData.size(); ++i) {
            std::stringstream ss;
            Serial::Serialize(encryptedData[i], ss, SerType::BINARY);
            pstmt->setString(i + 2, ss.str()); // 2부터 시작
        }

        pstmt->executeUpdate();
        return true;
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
        return false;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
}

// 공개처리 - 복호화 및 재무제표 삽입
void DBManager::decryptAndInsertFinancialStatements(CryptoContext<DCRTPoly>& cc, PrivateKey<DCRTPoly>& secretKey, int year, int quarter) {
    try {
        // Step 1: Fetch data from the department table
        std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
        std::unique_ptr<sql::ResultSet> departmentRes(stmt->executeQuery("SELECT departmentName, revenue, CostofGoodsSold, OperatingExpenses, "
                                                                        "NonOperatingExpenses, CurrentAssets, NonCurrentAssets, "
                                                                        "CurrentLiability, NonCurrentLiability, CapitalStock, "
                                                                        "RetainedEarning FROM department"));

        while (departmentRes->next()) {
            std::string departmentName = departmentRes->getString("departmentName");

            // Step 2: Fetch encrypted data from analyzedData table
            std::unique_ptr<sql::PreparedStatement> pstmt(conn_->prepareStatement("SELECT GrossProfit, OperatingProfit, NetProfit, "
                                                                                "TotalAssets, TotalLiabilities, TotalEquity "
                                                                                "FROM analyzedData WHERE departmentName = ?"));
            pstmt->setString(1, departmentName);
            std::unique_ptr<sql::ResultSet> analyzedRes(pstmt->executeQuery());

            if (analyzedRes->next()) {
                // Step 3: Decrypt the values
                // Assuming the data is stored as encrypted strings in the analyzedData table
                std::string encryptedGrossProfit = analyzedRes->getString("GrossProfit");
                std::string encryptedOperatingProfit = analyzedRes->getString("OperatingProfit");
                std::string encryptedNetProfit = analyzedRes->getString("NetProfit");
                std::string encryptedTotalAssets = analyzedRes->getString("TotalAssets");
                std::string encryptedTotalLiabilities = analyzedRes->getString("TotalLiabilities");
                std::string encryptedTotalEquity = analyzedRes->getString("TotalEquity");

                // Deserialize and decrypt each encrypted value
                Ciphertext<DCRTPoly> grossProfitCipher, operatingProfitCipher, netProfitCipher;
                Ciphertext<DCRTPoly> totalAssetsCipher, totalLiabilitiesCipher, totalEquityCipher;

                // Deserialize the ciphertexts
                {
                    std::stringstream ssGross(encryptedGrossProfit);
                    Serial::Deserialize(grossProfitCipher, ssGross, SerType::BINARY);

                    std::stringstream ssOperating(encryptedOperatingProfit);
                    Serial::Deserialize(operatingProfitCipher, ssOperating, SerType::BINARY);

                    std::stringstream ssNet(encryptedNetProfit);
                    Serial::Deserialize(netProfitCipher, ssNet, SerType::BINARY);

                    std::stringstream ssTotalAssets(encryptedTotalAssets);
                    Serial::Deserialize(totalAssetsCipher, ssTotalAssets, SerType::BINARY);

                    std::stringstream ssTotalLiabilities(encryptedTotalLiabilities);
                    Serial::Deserialize(totalLiabilitiesCipher, ssTotalLiabilities, SerType::BINARY);

                    std::stringstream ssTotalEquity(encryptedTotalEquity);
                    Serial::Deserialize(totalEquityCipher, ssTotalEquity, SerType::BINARY);
                }

                // Decrypting the values
                Plaintext grossProfitPlain, operatingProfitPlain, netProfitPlain, totalAssetsPlain, totalLiabilitiesPlain, totalEquityPlain;

                cc->Decrypt(secretKey, grossProfitCipher, &grossProfitPlain);
                cc->Decrypt(secretKey, operatingProfitCipher, &operatingProfitPlain);
                cc->Decrypt(secretKey, netProfitCipher, &netProfitPlain);
                cc->Decrypt(secretKey, totalAssetsCipher, &totalAssetsPlain);
                cc->Decrypt(secretKey, totalLiabilitiesCipher, &totalLiabilitiesPlain);
                cc->Decrypt(secretKey, totalEquityCipher, &totalEquityPlain);

                // Extract the decrypted values (assuming they are stored as a single value)
                long long grossProfit = grossProfitPlain->GetPackedValue()[0];
                long long operatingProfit = operatingProfitPlain->GetPackedValue()[0];
                long long netProfit = netProfitPlain->GetPackedValue()[0];
                long long totalAssets = totalAssetsPlain->GetPackedValue()[0];
                long long totalLiabilities = totalLiabilitiesPlain->GetPackedValue()[0];
                long long totalEquity = totalEquityPlain->GetPackedValue()[0];

                // Step 4: Insert the decrypted data into financialStatements table
                std::unique_ptr<sql::PreparedStatement> insertStmt(conn_->prepareStatement("INSERT INTO financialStatements "
                                                                                    "(departmentName, year, quater, revenue, CostofGoodsSold, "
                                                                                    "OperatingExpenses, NonOperatingExpenses, CurrentAssets, "
                                                                                    "NonCurrentAssets, CurrentLiability, NonCurrentLiability, "
                                                                                    "CapitalStock, RetainedEarning, GrossProfit, OperatingProfit, "
                                                                                    "NetProfit, TotalAssets, TotalLiabilities, TotalEquity) "
                                                                                    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
                insertStmt->setString(1, departmentName);
                insertStmt->setInt(2, year);
                insertStmt->setInt(3, quarter);
                insertStmt->setInt64(4, departmentRes->getInt64("revenue"));
                insertStmt->setInt64(5, departmentRes->getInt64("CostofGoodsSold"));
                insertStmt->setInt64(6, departmentRes->getInt64("OperatingExpenses"));
                insertStmt->setInt64(7, departmentRes->getInt64("NonOperatingExpenses"));
                insertStmt->setInt64(8, departmentRes->getInt64("CurrentAssets"));
                insertStmt->setInt64(9, departmentRes->getInt64("NonCurrentAssets"));
                insertStmt->setInt64(10, departmentRes->getInt64("CurrentLiability"));
                insertStmt->setInt64(11, departmentRes->getInt64("NonCurrentLiability"));
                insertStmt->setInt64(12, departmentRes->getInt64("CapitalStock"));
                insertStmt->setInt64(13, departmentRes->getInt64("RetainedEarning"));
                insertStmt->setInt64(14, grossProfit);
                insertStmt->setInt64(15, operatingProfit);
                insertStmt->setInt64(16, netProfit);
                insertStmt->setInt64(17, totalAssets);
                insertStmt->setInt64(18, totalLiabilities);
                insertStmt->setInt64(19, totalEquity);

                insertStmt->executeUpdate();
            }
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

void DBManager::printFinancialStatements() {
    try {
        std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM financialStatements"));

        // Print the header
        std::cout << std::left << std::setw(20) << "Department Name"
                << std::setw(10) << "Year"
                << std::setw(10) << "Quarter"
                << std::setw(15) << "Revenue"
                << std::setw(20) << "Cost of Goods Sold"
                << std::setw(20) << "Operating Expenses"
                << std::setw(20) << "Non-Operating Expenses"
                << std::setw(15) << "Current Assets"
                << std::setw(20) << "Non-Current Assets"
                << std::setw(20) << "Current Liability"
                << std::setw(20) << "Non-Current Liability"
                << std::setw(15) << "Capital Stock"
                << std::setw(20) << "Retained Earnings"
                << std::setw(15) << "Gross Profit"
                << std::setw(20) << "Operating Profit"
                << std::setw(15) << "Net Profit"
                << std::setw(15) << "Total Assets"
                << std::setw(20) << "Total Liabilities"
                << std::setw(15) << "Total Equity" << std::endl;

        // Print a separator
        std::cout << std::string(180, '-') << std::endl;

        // Print each row in the result set
        while (res->next()) {
            std::cout << std::left << std::setw(20) << res->getString("departmentName")
                    << std::setw(10) << res->getInt("year")
                    << std::setw(10) << res->getInt("quater")
                    << std::setw(15) << res->getInt("revenue")
                    << std::setw(20) << res->getInt("CostofGoodsSold")
                    << std::setw(20) << res->getInt("OperatingExpenses")
                    << std::setw(20) << res->getInt("NonOperatingExpenses")
                    << std::setw(15) << res->getInt("CurrentAssets")
                    << std::setw(20) << res->getInt("NonCurrentAssets")
                    << std::setw(20) << res->getInt("CurrentLiability")
                    << std::setw(20) << res->getInt("NonCurrentLiability")
                    << std::setw(15) << res->getInt("CapitalStock")
                    << std::setw(20) << res->getInt("RetainedEarning")
                    << std::setw(15) << res->getInt("GrossProfit")
                    << std::setw(20) << res->getInt("OperatingProfit")
                    << std::setw(15) << res->getInt("NetProfit")
                    << std::setw(15) << res->getInt("TotalAssets")
                    << std::setw(20) << res->getInt("TotalLiabilities")
                    << std::setw(15) << res->getInt("TotalEquity") << std::endl;
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

void DBManager::printFinancialStatementsByDepartment(const std::string& departmentName) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(conn_->prepareStatement(
            "SELECT * FROM financialStatements WHERE departmentName = ?"));
        pstmt->setString(1, departmentName);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        // Print the header
        std::cout << std::left << std::setw(20) << "Department Name"
                << std::setw(10) << "Year"
                << std::setw(10) << "Quarter"
                << std::setw(15) << "Revenue"
                << std::setw(20) << "Cost of Goods Sold"
                << std::setw(20) << "Operating Expenses"
                << std::setw(20) << "Non-Operating Expenses"
                << std::setw(15) << "Current Assets"
                << std::setw(20) << "Non-Current Assets"
                << std::setw(20) << "Current Liability"
                << std::setw(20) << "Non-Current Liability"
                << std::setw(15) << "Capital Stock"
                << std::setw(20) << "Retained Earnings"
                << std::setw(15) << "Gross Profit"
                << std::setw(20) << "Operating Profit"
                << std::setw(15) << "Net Profit"
                << std::setw(15) << "Total Assets"
                << std::setw(20) << "Total Liabilities"
                << std::setw(15) << "Total Equity" << std::endl;

        // Print a separator
        std::cout << std::string(180, '-') << std::endl;

        // Check if there are rows for the given department
        if (!res->next()) {
            std::cout << "No records found for department: " << departmentName << std::endl;
            return;
        }

        // Print each row in the result set
        do {
            std::cout << std::left << std::setw(20) << res->getString("departmentName")
                    << std::setw(10) << res->getInt("year")
                    << std::setw(10) << res->getInt("quater")
                    << std::setw(15) << res->getInt("revenue")
                    << std::setw(20) << res->getInt("CostofGoodsSold")
                    << std::setw(20) << res->getInt("OperatingExpenses")
                    << std::setw(20) << res->getInt("NonOperatingExpenses")
                    << std::setw(15) << res->getInt("CurrentAssets")
                    << std::setw(20) << res->getInt("NonCurrentAssets")
                    << std::setw(20) << res->getInt("CurrentLiability")
                    << std::setw(20) << res->getInt("NonCurrentLiability")
                    << std::setw(15) << res->getInt("CapitalStock")
                    << std::setw(20) << res->getInt("RetainedEarning")
                    << std::setw(15) << res->getInt("GrossProfit")
                    << std::setw(20) << res->getInt("OperatingProfit")
                    << std::setw(15) << res->getInt("NetProfit")
                    << std::setw(15) << res->getInt("TotalAssets")
                    << std::setw(20) << res->getInt("TotalLiabilities")
                    << std::setw(15) << res->getInt("TotalEquity") << std::endl;
        } while (res->next());
        
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

std::string DBManager::getFinancialStatements() {
    std::ostringstream output; // Use ostringstream to build the output string

    try {
        std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM financialStatements"));

        // Print the header
        output << std::left << std::setw(20) << "Department Name"
            << std::setw(10) << "Year"
            << std::setw(10) << "Quarter"
            << std::setw(15) << "Revenue"
            << std::setw(20) << "Cost of Goods Sold"
            << std::setw(20) << "Operating Expenses"
            << std::setw(20) << "Non-Operating Expenses"
            << std::setw(15) << "Current Assets"
            << std::setw(20) << "Non-Current Assets"
            << std::setw(20) << "Current Liability"
            << std::setw(20) << "Non-Current Liability"
            << std::setw(15) << "Capital Stock"
            << std::setw(20) << "Retained Earnings"
            << std::setw(15) << "Gross Profit"
            << std::setw(20) << "Operating Profit"
            << std::setw(15) << "Net Profit"
            << std::setw(15) << "Total Assets"
            << std::setw(20) << "Total Liabilities"
            << std::setw(15) << "Total Equity" << std::endl;

        // Print a separator
        output << std::string(180, '-') << std::endl;

        // Print each row in the result set
        while (res->next()) {
            output << std::left << std::setw(20) << res->getString("departmentName")
                << std::setw(10) << res->getInt("year")
                << std::setw(10) << res->getInt("quater")
                << std::setw(15) << res->getInt("revenue")
                << std::setw(20) << res->getInt("CostofGoodsSold")
                << std::setw(20) << res->getInt("OperatingExpenses")
                << std::setw(20) << res->getInt("NonOperatingExpenses")
                << std::setw(15) << res->getInt("CurrentAssets")
                << std::setw(20) << res->getInt("NonCurrentAssets")
                << std::setw(20) << res->getInt("CurrentLiability")
                << std::setw(20) << res->getInt("NonCurrentLiability")
                << std::setw(15) << res->getInt("CapitalStock")
                << std::setw(20) << res->getInt("RetainedEarning")
                << std::setw(15) << res->getInt("GrossProfit")
                << std::setw(20) << res->getInt("OperatingProfit")
                << std::setw(15) << res->getInt("NetProfit")
                << std::setw(15) << res->getInt("TotalAssets")
                << std::setw(20) << res->getInt("TotalLiabilities")
                << std::setw(15) << res->getInt("TotalEquity") << std::endl;
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
        output << "SQLException: " << e.what() << std::endl; // Append error message to output
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        output << "Exception: " << e.what() << std::endl; // Append error message to output
    }

    return output.str(); // Return the constructed string
}



std::string DBManager::getFinancialStatementsByDepartment(const std::string& departmentName) {
    std::ostringstream output; // Use ostringstream to build the output string

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(conn_->prepareStatement(
            "SELECT * FROM financialStatements WHERE departmentName = ?"));
        pstmt->setString(1, departmentName);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        // Print the header
        output << std::left << std::setw(20) << "Department Name"
            << std::setw(10) << "Year"
            << std::setw(10) << "Quarter"
            << std::setw(15) << "Revenue"
            << std::setw(20) << "Cost of Goods Sold"
            << std::setw(20) << "Operating Expenses"
            << std::setw(20) << "Non-Operating Expenses"
            << std::setw(15) << "Current Assets"
            << std::setw(20) << "Non-Current Assets"
            << std::setw(20) << "Current Liability"
            << std::setw(20) << "Non-Current Liability"
            << std::setw(15) << "Capital Stock"
            << std::setw(20) << "Retained Earnings"
            << std::setw(15) << "Gross Profit"
            << std::setw(20) << "Operating Profit"
            << std::setw(15) << "Net Profit"
            << std::setw(15) << "Total Assets"
            << std::setw(20) << "Total Liabilities"
            << std::setw(15) << "Total Equity" << std::endl;

        // Print a separator
        output << std::string(180, '-') << std::endl;

        // Check if there are rows for the given department
        if (!res->next()) {
            output << "No records found for department: " << departmentName << std::endl;
            return output.str(); // Return the output as a string
        }

        // Print each row in the result set
        do {
            output << std::left << std::setw(20) << res->getString("departmentName")
                << std::setw(10) << res->getInt("year")
                << std::setw(10) << res->getInt("quater")
                << std::setw(15) << res->getInt("revenue")
                << std::setw(20) << res->getInt("CostofGoodsSold")
                << std::setw(20) << res->getInt("OperatingExpenses")
                << std::setw(20) << res->getInt("NonOperatingExpenses")
                << std::setw(15) << res->getInt("CurrentAssets")
                << std::setw(20) << res->getInt("NonCurrentAssets")
                << std::setw(20) << res->getInt("CurrentLiability")
                << std::setw(20) << res->getInt("NonCurrentLiability")
                << std::setw(15) << res->getInt("CapitalStock")
                << std::setw(20) << res->getInt("RetainedEarning")
                << std::setw(15) << res->getInt("GrossProfit")
                << std::setw(20) << res->getInt("OperatingProfit")
                << std::setw(15) << res->getInt("NetProfit")
                << std::setw(15) << res->getInt("TotalAssets")
                << std::setw(20) << res->getInt("TotalLiabilities")
                << std::setw(15) << res->getInt("TotalEquity") << std::endl;
        } while (res->next());

    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
        output << "SQLException: " << e.what() << std::endl; // Append error message to output
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        output << "Exception: " << e.what() << std::endl; // Append error message to output
    }

    return output.str(); // Return the constructed string
}
