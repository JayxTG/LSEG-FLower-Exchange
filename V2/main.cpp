#include "order_matcher.h"
#include <chrono>
#include <iostream>
#include <fstream>  // Add this include for ofstream

int main() {

    // Clear output file
    std::ofstream outFile("execution_report.csv", std::ios::trunc);
    outFile.close();

    // Ask for the CSV file name without the extension
    std::string fileName;
    std::cout << "Enter the CSV file name (with .csv extension): ";
    std::cin >> fileName;



    // Start timing
    auto start = std::chrono::high_resolution_clock::now();

    // Read orders
    std::vector<CSVRow> orders = utils::readCSV(fileName);
    OrderMatcher matcher;

    // Process orders
    for (auto& order : orders) {
        if (!utils::validateOrder(order)) {
            utils::writeToCSV(order);
            continue;
        }
        matcher.processOrder(order);
    }

    // Match orders
    matcher.matchOrders();

    // Calculate execution time
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    utils::writeExecutionTime(duration.count());

    std::cout << "Processing complete. Check execution_report.csv for results." << std::endl;
    return 0;
}