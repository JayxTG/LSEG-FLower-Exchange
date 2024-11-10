#pragma once

#include "Types.h"
#include <vector>

class OrderMatcher {
public:
    OrderMatcher();
    void processOrder(const CSVRow& row);
    void matchOrders();
    
private:
    std::vector<CSVRow> buyTable;
    std::vector<CSVRow> sellTable;
    void sortTables();
    void cleanupFilledOrders();
};

// Utility functions
namespace utils {
    void trim(std::string& s);
    std::vector<CSVRow> readCSV(const std::string& filename);
    void writeToCSV(const CSVRow& row);
    bool validateOrder(CSVRow& row);
    void writeExecutionTime(long long microseconds);
}