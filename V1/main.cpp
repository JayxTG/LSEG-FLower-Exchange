#include "OrderProcessor.h"
#include <iostream>
#include <chrono>
#include <fstream> // for output file operations

int main() {
    OrderProcessor processor;
    std::string inputFileName;
    
    // Ask the user for the CSV file name
    std::cout << "Enter the name of the CSV file to process (e.g., orders.csv): ";
    std::getline(std::cin, inputFileName);

    auto start = std::chrono::high_resolution_clock::now();

    // Process orders from the user-provided file name
    processor.processOrders(inputFileName, "execution_report.csv");

    auto end = std::chrono::high_resolution_clock::now();
    // Use duration_cast to convert the elapsed time to microseconds
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    // Open the execution report to append the execution time
    std::ofstream execReportFile("execution_report.csv", std::ios_base::app);
    if (execReportFile.is_open()) {
        execReportFile << "Execution Time (microseconds): " << elapsed << "\n";
        execReportFile.close();
    } else {
        std::cerr << "Error opening execution_report.csv to write execution time." << std::endl;
    }

    std::cout << "Execution time: " << elapsed << " microseconds\n";

    return 0;
}
