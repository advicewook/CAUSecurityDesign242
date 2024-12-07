#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <string>
#include <vector>
#include <memory>

class DatabaseManager {
public:
    DatabaseManager(const std::string& db, const std::string& user, const std::string& password);
    ~DatabaseManager();

    void connect();
    std::vector<std::vector<long long>> fetchDepartmentData();
    std::vector<std::vector<long long>> fetchPreviousData(const std::string& departmentName, int year, int quarter);
    void insertAnalyzedData(const std::string& departmentName, long long grossProfit, long long operatingProfit, 
                            long long netProfit, long long totalAssets, long long totalLiabilities, long long totalEquity);
    
    sql::Connection* getConnection();

private:
    std::string dbName;
    std::string userName;
    std::string userPassword;
    sql::Driver* driver;
    std::unique_ptr<sql::Connection> conn;
};

#endif // DATABASE_MANAGER_H
