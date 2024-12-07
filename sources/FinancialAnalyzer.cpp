#include "FinancialAnalyzer.h"

FinancialAnalyzer::FinancialAnalyzer(DBManager& dbManager) : dbManager(dbManager) {}

// void FinancialAnalyzer::analyze() {
//     auto data = dbManager.fetchDepartmentData();
    
//     for (const auto& row : data) {
//         std::string departmentName = row[0];
//         long long revenue = row[1];
//         long long costOfGoodsSold = row[2];
//         long long operatingExpenses = row[3];
//         long long nonOperatingExpenses = row[4];
//         long long currentAssets = row[5];
//         long long nonCurrentAssets = row[6];
//         long long currentLiabilities = row[7];
//         long long nonCurrentLiabilities = row[8];
//         long long capitalStock = row[9];
//         long long retainedEarning = row[10];

//         long long grossProfit = calculateGrossProfit(revenue, costOfGoodsSold);
//         long long operatingProfit = calculateOperatingProfit(grossProfit, operatingExpenses);
//         long long netProfit = calculateNetProfit(operatingProfit, nonOperatingExpenses);
//         long long totalAssets = calculateTotalAssets(currentAssets, nonCurrentAssets);
//         long long totalLiabilities = calculateTotalLiabilities(currentLiabilities, nonCurrentLiabilities);
//         long long totalEquity = calculateTotalEquity(totalAssets, totalLiabilities);

//         dbManager.insertAnalyzedData(departmentName, grossProfit, operatingProfit, netProfit, totalAssets, totalLiabilities, totalEquity);
//     }
// }

std::string FinancialAnalyzer::getInvestInfo(const std::string& message) {
    std::string departmentName, label;

    // Parse the message (message format: departmentName label)
    std::istringstream iss(message);
    iss >> departmentName >> label;

    double percentage = dbManager.calculateGrowthRate(departmentName, label);
    
    // Get the investment recommendation based on the growth percentage
    std::string recommendation = getInvestRecommendation(percentage);

    return recommendation; // Return the investment recommendation
}

std::string FinancialAnalyzer::getInvestRecommendation(double percentage) {
    if (percentage < 0) {
        return "Not Recommended for Investment"; // Negative growth rate
    } else if (percentage >= 0 && percentage < 10) {
        return "Avoid Investment"; // Low growth
    } else if (percentage >= 10 && percentage < 20) {
        return "Recommended for Investment"; // Moderate growth
    } else {
        return "Strongly Recommended for Investment"; // High growth
    }
}


// void FinancialAnalyzer::processDepartmentData(DBManager& dbManager, CryptoContext<DCRTPoly>& cc, PublicKey<DCRTPoly>& publicKey) {
//     auto encryptedData = dbManager.fetchDepartmentData();

//     for (const auto& row : encryptedData) {
//         std::string departmentName = *std::static_pointer_cast<std::string>(row[0]); // 부서 이름
        
//         const Ciphertext<DCRTPoly>& grossProfit = dbManager.addCiphertexts(row[1], row[2], cc); // 매출 총이익
//         const Ciphertext<DCRTPoly>& operatingProfit = dbManager.addCiphertexts(grossProfit, row[3], cc); // 운영 수익
//         const Ciphertext<DCRTPoly>& netProfit = dbManager.addCiphertexts(operatingProfit, row[4], cc); // 순이익
//         const Ciphertext<DCRTPoly>& totalAssets = dbManager.addCiphertexts(row[5], row[6], cc); // 총자산
//         const Ciphertext<DCRTPoly>& totalLiabilities = dbManager.addCiphertexts(row[7], row[8], cc); // 총부채
//         Ciphertext<DCRTPoly>& totalEquity = dbManager.addCiphertexts(totalAssets, totalLiabilities, cc); // 총자본

//         // 분석된 데이터 삽입
//         dbManager.insertAnalyzedData(departmentName, {grossProfit, operatingProfit, netProfit, totalAssets, totalLiabilities, totalEquity});
//     }
// }



// void FinancialAnalyzer::compareWithPreviousData(const std::string& departmentName, int year, int quarter) {
//     auto currentData = dbManager.fetchDepartmentData();
//     auto previousData = dbManager.fetchPreviousData(departmentName, year, quarter);

//     if (previousData.empty()) {
//         std::cerr << "No previous data found for department: " << departmentName << ", year: " << year << ", quarter: " << quarter << std::endl;
//         return;
//     }

//     for (const auto& row : currentData) {
//         if (row[0] == departmentName) {
//             long long currentRevenue = row[1];
//             long long previousRevenue = previousData[0][1];
//             double revenueChange = calculatePercentageChange(previousRevenue, currentRevenue);

//             // Repeat the process for other financial metrics...

//             std::cout << "Comparison for department: " << departmentName << ", year: " << year << ", quarter: " << quarter << std::endl;
//             std::cout << "Revenue change: " << revenueChange << "%" << std::endl;
//             // Print other comparisons...
//         }
//     }
// }

// void FinancialAnalyzer::compareWithPreviousData(const std::string& departmentName, int year, int quarter, const std::vector<std::string>& metrics) {
//     auto currentData = dbManager.fetchDepartmentData();
//     auto previousData = dbManager.fetchPreviousData(departmentName, year, quarter);

//     if (previousData.empty()) {
//         std::cerr << "No previous data found for department: " << departmentName << ", year: " << year << ", quarter: " << quarter << std::endl;
//         return;
//     }

//     for (const auto& row : currentData) {
//         if (row[0] == departmentName) {
//             std::cout << "Comparison for department: " << departmentName << ", year: " << year << ", quarter: " << quarter << std::endl;

//             for (const auto& metric : metrics) {
//                 if (metric == "revenue") {
//                     long long currentRevenue = row[1];
//                     long long previousRevenue = previousData[0][1];
//                     double revenueChange = calculatePercentageChange(previousRevenue, currentRevenue);
//                     std::cout << "Revenue change: " << revenueChange << "%" << std::endl;
//                 }
//                 // Repeat for other metrics as necessary
//                 else if (metric == "CostofGoodsSold") {
//                     long long currentCostofGoodsSold = row[2];
//                     long long previousCostofGoodsSold = previousData[0][2];
//                     double costOfGoodsSoldChange = calculatePercentageChange(previousCostofGoodsSold, currentCostofGoodsSold);
//                     std::cout << "Cost of Goods Sold change: " << costOfGoodsSoldChange << "%" << std::endl;
//                 }
//                 // Add other metrics as needed
//             }
//         }
//     }
// }


// TODO: change from cout to boost::asio::write
std::string FinancialAnalyzer::displayPreviousData() {
    return dbManager.getFinancialStatements();
}

// TODO: change from cout to boost::asio::write
std::string FinancialAnalyzer::displaypreviousData(const std::string& departmentName) {
    return dbManager.getFinancialStatementsByDepartment(departmentName);
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
