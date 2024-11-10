#include "order_matcher.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// Utility functions implementation
void utils::trim(std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start != std::string::npos && end != std::string::npos) {
        s = s.substr(start, end - start + 1);
    }
    else {
        s.clear();
    }
}

std::vector<CSVRow> utils::readCSV(const std::string& filename) {
    std::vector<CSVRow> data;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return data;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        CSVRow row;
        
        // Changed from "ord" to "order"
        row.ord = "order" + std::to_string(data.size() + 1);
        std::getline(ss, row.column1, ',');
        std::getline(ss, row.column2, ',');
        std::getline(ss, row.column3, ',');
        row.newColumn = "New";
        std::getline(ss, row.column4, ',');
        std::getline(ss, row.column5, ',');
        
        utils::trim(row.column3);
        data.push_back(row);
    }
    
    file.close();
    return data;
}

void utils::writeToCSV(const CSVRow& row) {
    std::ofstream file("execution_report.csv", std::ios::app);
    if (file.is_open()) {
        file << row.ord << "," << row.column1 << "," << row.column2 << ","
             << row.column3 << "," << row.newColumn << "," << row.column4 << ","
             << row.column5 << std::endl;
        file.close();
    }
}


bool utils::validateOrder(CSVRow& row) {
    if (row.column1.empty() || row.column2.empty() || row.column3.empty() ||
        row.column4.empty() || row.column5.empty()) {
        row.newColumn = "Reject";
        return false;
    }

    utils::trim(row.column2);
    if (VALID_INSTRUMENTS.find(row.column2) == VALID_INSTRUMENTS.end()) {
        row.newColumn = "Reject";
        return false;
    }

    try {
        int side = std::stoi(row.column3);
        int price = std::stoi(row.column4);
        int quantity = std::stoi(row.column5);

        if (side < 1 || side > 2 || price % PRICE_MULTIPLE != 0 || 
            price < MIN_PRICE || price > MAX_PRICE || quantity <= 0) {
            row.newColumn = "Reject";
            return false;
        }
    }
    catch (...) {
        row.newColumn = "Reject";
        return false;
    }

    return true;
}

void utils::writeExecutionTime(long long microseconds) {
    std::ofstream file("execution_report.csv", std::ios::app);
    if (file.is_open()) {
        file << "\nExecution Time (microseconds): " << microseconds << std::endl;
        file.close();
    }
}

// OrderMatcher implementation
OrderMatcher::OrderMatcher() {}

void OrderMatcher::processOrder(const CSVRow& row) {
    if (row.column3 == "1") {
        buyTable.push_back(row);
    }
    else if (row.column3 == "2") {
        sellTable.push_back(row);
    }
}

void OrderMatcher::matchOrders() {
    sortTables();
    
    for (auto buyIt = buyTable.begin(); buyIt != buyTable.end();) {
        if (buyIt->newColumn != "New" && buyIt->newColumn != "PFill") {
            ++buyIt;
            continue;
        }

        for (auto sellIt = sellTable.begin(); sellIt != sellTable.end();) {
            if (sellIt->newColumn != "New" && sellIt->newColumn != "PFill") {
                ++sellIt;
                continue;
            }

            if (buyIt->column2 != sellIt->column2) {
                ++sellIt;
                continue;
            }

            int buyQty = std::stoi(buyIt->column5);
            int sellQty = std::stoi(sellIt->column5);
            int buyPrice = std::stoi(buyIt->column4);
            int sellPrice = std::stoi(sellIt->column4);

            if (sellQty <= buyQty) {
                if (sellPrice > buyPrice) {
                    // Buy order gets filled at buy price
                    buyIt->newColumn = "Fill";
                    
                    CSVRow newSellRow = *sellIt;
                    newSellRow.newColumn = "PFill";
                    newSellRow.column4 = buyIt->column4;
                    newSellRow.column5 = buyIt->column5;
                    
                    sellIt->column4 = std::to_string(sellPrice - buyPrice);
                    sellIt->newColumn = "PFill";
                    
                    utils::writeToCSV(newSellRow);
                    utils::writeToCSV(*buyIt);
                    ++sellIt;
                }
                else if (sellPrice == buyPrice) {
                    // Both orders get filled at the same price
                    buyIt->newColumn = "Fill";
                    sellIt->newColumn = "Fill";
                    sellIt->column5 = buyIt->column5;
                    
                    utils::writeToCSV(*sellIt);
                    utils::writeToCSV(*buyIt);
                    ++sellIt;
                }
                else {
                    // Sell order gets filled at sell price
                    sellIt->newColumn = "Fill";
                    sellIt->column5 = buyIt->column5;
                    
                    CSVRow newBuyRow = *buyIt;
                    newBuyRow.newColumn = "PFill";
                    newBuyRow.column4 = sellIt->column4;
                    newBuyRow.column5 = buyIt->column5;
                    
                    buyIt->column4 = std::to_string(buyPrice - sellPrice);
                    buyIt->newColumn = "PFill";
                    
                    utils::writeToCSV(*sellIt);
                    utils::writeToCSV(newBuyRow);
                    ++sellIt;
                }
            }
            else {
                if (buyIt->newColumn == "New") {
                    utils::writeToCSV(*buyIt);
                }
                break;
            }
        }

        if (buyIt->newColumn == "New" || buyIt->newColumn == "PFill") {
            ++buyIt;
        }
    }

    cleanupFilledOrders();
}

void OrderMatcher::cleanupFilledOrders() {
    buyTable.erase(
        std::remove_if(buyTable.begin(), buyTable.end(),
            [](const CSVRow& row) { return row.newColumn == "Fill"; }),
        buyTable.end());

    sellTable.erase(
        std::remove_if(sellTable.begin(), sellTable.end(),
            [](const CSVRow& row) { return row.newColumn == "Fill"; }),
        sellTable.end());
}

void OrderMatcher::sortTables() {
    // Sort buy table in descending order based on quantity
    std::sort(buyTable.begin(), buyTable.end(),
        [](const CSVRow& a, const CSVRow& b) {
            return std::stoi(a.column5) > std::stoi(b.column5);
        });

    // Sort sell table in ascending order based on quantity
    std::sort(sellTable.begin(), sellTable.end(),
        [](const CSVRow& a, const CSVRow& b) {
            return std::stoi(a.column5) < std::stoi(b.column5);
        });
}