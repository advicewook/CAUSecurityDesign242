#ifndef FINANCIAL_ANALYZER_H
#define FINANCIAL_ANALYZER_H

#include "DatabaseManager.h"
#include <vector>

class FinancialAnalyzer {
public:
    FinancialAnalyzer(DatabaseManager& dbManager);
    void analyze();
    void compareWithPreviousData(const std::string& departmentName, int year, int quarter);
    void compareWithPreviousData(const std::string& departmentName, int year, int quarter, const std::vector<std::string>& metrics);


private:
    DatabaseManager& dbManager;

    long long calculateGrossProfit(long long revenue, long long costOfGoodsSold);
    long long calculateOperatingProfit(long long grossProfit, long long operatingExpenses);
    long long calculateNetProfit(long long operatingProfit, long long nonOperatingExpenses);
    long long calculateTotalAssets(long long currentAssets, long long nonCurrentAssets);
    long long calculateTotalLiabilities(long long currentLiabilities, long long nonCurrentLiabilities);
    long long calculateTotalEquity(long long totalAssets, long long totalLiabilities);
    double calculatePercentageChange(long long previous, long long current);
    void displayPreviousData(const std::string& departmentName);

};

#endif // FINANCIAL_ANALYZER_H
