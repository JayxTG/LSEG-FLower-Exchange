#ifndef ORDERPROCESSOR_H
#define ORDERPROCESSOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include "CSVRow.h"

struct Order {
    std::string clientOrderID;
    std::string instrument;
    int side;
    double price;
    int quantity;
    std::string orderID;
    int status; // 0: New, 1: Rejected, 2: Fill, 3: Partial Fill (Pfill)
    std::string reason;
    std::string transactionTime;
};

class OrderProcessor {
public:
    OrderProcessor();
    
    void processOrders(const std::string& orderFile, const std::string& execReportFile);
    bool validateOrder(const Order& order);
    std::string generateOrderID();  // Generate a unique Order ID
    std::string getCurrentTimestamp(); // Get the current time in required format
    
private:
    void readCSV(const std::string& fileName, std::vector<CSVRow>& rows);
    void writeCSV(const std::string& fileName, const std::vector<Order>& execReports);
    void matchOrders(std::unordered_map<std::string, std::vector<Order>>& orderBook, Order newOrder, std::vector<Order>& execReports);
    void addToOrderBook(std::unordered_map<std::string, std::vector<Order>>& orderBook, const Order& order);
    
    std::unordered_map<std::string, std::vector<Order>> buyOrders;
    std::unordered_map<std::string, std::vector<Order>> sellOrders;
};

#endif
