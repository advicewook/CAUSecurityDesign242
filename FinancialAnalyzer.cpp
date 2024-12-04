#include "FinancialAnalyzer.h"

FinancialAnalyzer::FinancialAnalyzer(DatabaseManager& dbManager) : dbManager(dbManager) {}

void FinancialAnalyzer::analyze() {
    auto data = dbManager.fetchDepartmentData();
    
    for (const auto& row : data) {
        std::string departmentName = row[0];
        long long revenue = row[1];
        long long costOfGoodsSold = row[2];
        long long operatingExpenses = row[3];
        long long nonOperatingExpenses = row[4];
        long long currentAssets = row[5];
        long long nonCurrentAssets = row[6];
        long long currentLiabilities = row[7];
        long long nonCurrentLiabilities = row[8];
        long long capitalStock = row[9];
        long long retainedEarning = row[10];

        long long grossProfit = calculateGrossProfit(revenue, costOfGoodsSold);
        long long operatingProfit = calculateOperatingProfit(grossProfit, operatingExpenses);
        long long netProfit = calculateNetProfit(operatingProfit, nonOperatingExpenses);
        long long totalAssets = calculateTotalAssets(currentAssets, nonCurrentAssets);
        long long totalLiabilities = calculateTotalLiabilities(currentLiabilities, nonCurrentLiabilities);
        long long totalEquity = calculateTotalEquity(totalAssets, totalLiabilities);

        dbManager.insertAnalyzedData(departmentName, grossProfit, operatingProfit, netProfit, totalAssets, totalLiabilities, totalEquity);
    }
}


void FinancialAnalyzer::compareWithPreviousData(const std::string& departmentName, int year, int quarter) {
    auto currentData = dbManager.fetchDepartmentData();
    auto previousData = dbManager.fetchPreviousData(departmentName, year, quarter);

    if (previousData.empty()) {
        std::cerr << "No previous data found for department: " << departmentName << ", year: " << year << ", quarter: " << quarter << std::endl;
        return;
    }

    for (const auto& row : currentData) {
        if (row[0] == departmentName) {
            long long currentRevenue = row[1];
            long long previousRevenue = previousData[0][1];
            double revenueChange = calculatePercentageChange(previousRevenue, currentRevenue);

            // Repeat the process for other financial metrics...

            std::cout << "Comparison for department: " << departmentName << ", year: " << year << ", quarter: " << quarter << std::endl;
            std::cout << "Revenue change: " << revenueChange << "%" << std::endl;
            // Print other comparisons...
        }
    }
}

void FinancialAnalyzer::compareWithPreviousData(const std::string& departmentName, int year, int quarter, const std::vector<std::string>& metrics) {
    auto currentData = dbManager.fetchDepartmentData();
    auto previousData = dbManager.fetchPreviousData(departmentName, year, quarter);

    if (previousData.empty()) {
        std::cerr << "No previous data found for department: " << departmentName << ", year: " << year << ", quarter: " << quarter << std::endl;
        return;
    }

    for (const auto& row : currentData) {
        if (row[0] == departmentName) {
            std::cout << "Comparison for department: " << departmentName << ", year: " << year << ", quarter: " << quarter << std::endl;

            for (const auto& metric : metrics) {
                if (metric == "revenue") {
                    long long currentRevenue = row[1];
                    long long previousRevenue = previousData[0][1];
                    double revenueChange = calculatePercentageChange(previousRevenue, currentRevenue);
                    std::cout << "Revenue change: " << revenueChange << "%" << std::endl;
                }
                // Repeat for other metrics as necessary
                else if (metric == "CostofGoodsSold") {
                    long long currentCostofGoodsSold = row[2];
                    long long previousCostofGoodsSold = previousData[0][2];
                    double costOfGoodsSoldChange = calculatePercentageChange(previousCostofGoodsSold, currentCostofGoodsSold);
                    std::cout << "Cost of Goods Sold change: " << costOfGoodsSoldChange << "%" << std::endl;
                }
                // Add other metrics as needed
            }
        }
    }
}


void FinancialAnalyzer::displayPreviousData(const std::string& departmentName) {
    sql::Connection* conn = dbManager.getConnection();
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
            "SELECT year, quarter, revenue, CostofGoodsSold, OperatingExpenses, NonOperatingExpenses, "
            "CurrentAssets, NonCurrentAssets, CurrentLiability, NonCurrentLiability, CapitalStock, RetainedEarning "
            "FROM previous_data WHERE departmentName = ? ORDER BY year, quarter"));

        pstmt->setString(1, departmentName);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        std::cout << "Previous data for department: " << departmentName << std::endl;
        std::cout << "Year\tQuarter\tRevenue\tCost of Goods Sold\tOperating Expenses\tNon-Operating Expenses\t"
                    "Current Assets\tNon-Current Assets\tCurrent Liability\tNon-Current Liability\t"
                    "Capital Stock\tRetained Earning" << std::endl;

        while (res->next()) {
            std::cout << res->getInt("year") << "\t"
                        << res->getInt("quarter") << "\t"
                        << res->getInt64("revenue") << "\t"
                        << res->getInt64("CostofGoodsSold") << "\t"
                        << res->getInt64("OperatingExpenses") << "\t"
                        << res->getInt64("NonOperatingExpenses") << "\t"
                        << res->getInt64("CurrentAssets") << "\t"
                        << res->getInt64("NonCurrentAssets") << "\t"
                        << res->getInt64("CurrentLiability") << "\t"
                        << res->getInt64("NonCurrentLiability") << "\t"
                        << res->getInt64("CapitalStock") << "\t"
                        << res->getInt64("RetainedEarning") << std::endl;
        }
    } catch (sql::SQLException &e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
    }
}



long long FinancialAnalyzer::calculateGrossProfit(long long revenue, long long costOfGoodsSold) {
    return revenue - costOfGoodsSold;
}

long long FinancialAnalyzer::calculateOperatingProfit(long long grossProfit, long long operatingExpenses) {
    return grossProfit - operatingExpenses;
}

long long FinancialAnalyzer::calculateNetProfit(long long operatingProfit, long long nonOperatingExpenses) {
    return operatingProfit - nonOperatingExpenses;
}

long long FinancialAnalyzer::calculateTotalAssets(long long currentAssets, long long nonCurrentAssets) {
    return currentAssets + nonCurrentAssets;
}

long long FinancialAnalyzer::calculateTotalLiabilities(long long currentLiabilities, long long nonCurrentLiabilities) {
    return currentLiabilities + nonCurrentLiabilities;
}

long long FinancialAnalyzer::calculateTotalEquity(long long totalAssets, long long totalLiabilities) {
    return totalAssets - totalLiabilities;
}
