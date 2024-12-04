#include "DatabaseManager.h"
#include <iostream>

DatabaseManager::DatabaseManager(const std::string& db, const std::string& user, const std::string& password)
    : dbName(db), userName(user), userPassword(password), driver(nullptr) {}

DatabaseManager::~DatabaseManager() {
    if (conn) {
        conn->close();
    }
}

void DatabaseManager::connect() {
    driver = get_driver_instance();
    conn = std::unique_ptr<sql::Connection>(driver->connect("tcp://127.0.0.1:3306", userName, userPassword));
    conn->setSchema(dbName);
}

std::vector<std::vector<long long>> DatabaseManager::fetchDepartmentData() {
    std::vector<std::vector<long long>> data;
    try {
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT departmentName, revenue, CostofGoodsSold, OperatingExpenses, NonOperatingExpenses, CurrentAssets, NonCurrentAssets, CurrentLiability, NonCurrentLiability, CapitalStock, RetainedEarning FROM department"));

        while (res->next()) {
            std::vector<long long> row;
            row.push_back(res->getString("departmentName"));
            row.push_back(res->getInt64("revenue"));
            row.push_back(res->getInt64("CostofGoodsSold"));
            row.push_back(res->getInt64("OperatingExpenses"));
            row.push_back(res->getInt64("NonOperatingExpenses"));
            row.push_back(res->getInt64("CurrentAssets"));
            row.push_back(res->getInt64("NonCurrentAssets"));
            row.push_back(res->getInt64("CurrentLiability"));
            row.push_back(res->getInt64("NonCurrentLiability"));
            row.push_back(res->getInt64("CapitalStock"));
            row.push_back(res->getInt64("RetainedEarning"));
            data.push_back(row);
        }
    } catch (sql::SQLException &e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
    }

    return data;
}

std::vector<std::vector<long long>> DatabaseManager::fetchPreviousData(const std::string& departmentName, int year, int quarter) {
    std::vector<std::vector<long long>> data;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement("SELECT revenue, CostofGoodsSold, OperatingExpenses, NonOperatingExpenses, CurrentAssets, NonCurrentAssets, CurrentLiability, NonCurrentLiability, CapitalStock, RetainedEarning FROM previous_data WHERE departmentName = ? AND year = ? AND quarter = ?"));
        pstmt->setString(1, departmentName);
        pstmt->setInt(2, year);
        pstmt->setInt(3, quarter);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) {
            std::vector<long long> row;
            row.push_back(res->getInt64("revenue"));
            row.push_back(res->getInt64("CostofGoodsSold"));
            row.push_back(res->getInt64("OperatingExpenses"));
            row.push_back(res->getInt64("NonOperatingExpenses"));
            row.push_back(res->getInt64("CurrentAssets"));
            row.push_back(res->getInt64("NonCurrentAssets"));
            row.push_back(res->getInt64("CurrentLiability"));
            row.push_back(res->getInt64("NonCurrentLiability"));
            row.push_back(res->getInt64("CapitalStock"));
            row.push_back(res->getInt64("RetainedEarning"));
            data.push_back(row);
        }
    } catch (sql::SQLException &e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
    }

    return data;
}

void DatabaseManager::insertAnalyzedData(const std::string& departmentName, long long grossProfit, long long operatingProfit, 
                                        long long netProfit, long long totalAssets, long long totalLiabilities, long long totalEquity) {
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement("INSERT INTO analyzedData (departmentName, GrossProfit, OperatingProfit, NetProfit, TotalAssets, TotalLiabilities, TotalEquity) VALUES (?, ?, ?, ?, ?, ?, ?)"));
        pstmt->setString(1, departmentName);
        pstmt->setInt64(2, grossProfit);
        pstmt->setInt64(3, operatingProfit);
        pstmt->setInt64(4, netProfit);
        pstmt->setInt64(5, totalAssets);
        pstmt->setInt64(6, totalLiabilities);
        pstmt->setInt64(7, totalEquity);
        pstmt->executeUpdate();
    } catch (sql::SQLException &e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
    }

    
}

sql::Connection* DatabaseManager::getConnection() {
    return conn.get();
}
