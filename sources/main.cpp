#pragma comment (lib, "libmysql.lib")



#include <stdlib.h>
#include <iostream>
#include "Server.h"
#include "Client.h"

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



int main() {
    // Start the server in a separate thread
    std::thread serverThread(startServer);

    // Wait for the server to start
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Start the client
    startClient();

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
    

    return 0;
}
    

