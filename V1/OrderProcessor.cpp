#include "OrderProcessor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <algorithm>

OrderProcessor::OrderProcessor() {
    // Initialization if required
}

void OrderProcessor::processOrders(const std::string& orderFile, const std::string& execReportFile) {
    std::vector<CSVRow> rows;
    readCSV(orderFile, rows);
    
    std::vector<Order> execReports;

    for (const auto& row : rows) {
        Order order;
        order.clientOrderID = row[0];
        order.instrument = row[1];
        order.side = std::stoi(row[2]);
        order.price = std::stod(row[3]);
        order.quantity = std::stoi(row[4]);
        order.orderID = generateOrderID();
        order.transactionTime = getCurrentTimestamp();

        if (validateOrder(order)) {
            if (order.side == 1) { // Buy
                matchOrders(sellOrders, order, execReports);  // Match against sell orders
            } else if (order.side == 2) { // Sell
                matchOrders(buyOrders, order, execReports);  // Match against buy orders
            }
        } else {
            order.status = 1; // Rejected
            order.reason = "Validation failed";  // Add reason for rejection
            execReports.push_back(order);
        }
    }
    writeCSV(execReportFile, execReports);
}

bool OrderProcessor::validateOrder(const Order& order) {
    if (order.instrument != "Rose" && order.instrument != "Lavender" && 
        order.instrument != "Lotus" && order.instrument != "Tulip" && 
        order.instrument != "Orchid") {
        return false;
    }
    if (order.side != 1 && order.side != 2) {
        return false;
    }
    if (order.price <= 0.0) {
        return false;
    }
    if (order.quantity < 10 || order.quantity > 1000 || order.quantity % 10 != 0) {
        return false;
    }
    return true;
}

std::string OrderProcessor::generateOrderID() {
    static int counter = 1;
    return "ORD" + std::to_string(counter++);
}

std::string OrderProcessor::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y%m%d-%H%M%S");
    
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    ss << "." << std::setfill('0') << std::setw(3) << milliseconds.count();
    return ss.str();
}

void OrderProcessor::readCSV(const std::string& fileName, std::vector<CSVRow>& rows) {
    std::ifstream file(fileName);
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        CSVRow row;
        std::string item;
        while (std::getline(ss, item, ',')) {
            row.push_back(item);
        }
        rows.push_back(row);
    }
}

void OrderProcessor::writeCSV(const std::string& fileName, const std::vector<Order>& execReports) {
    std::ofstream file(fileName);
    
    // Write header for the execution report
    file << "Order ID,Client Order,Instrument,Side,Exec Status,Quantity,Price\n";
    
    for (const auto& order : execReports) {
        file << order.orderID << "," << order.clientOrderID << "," << order.instrument << "," 
             << order.side << "," << (order.status == 1 ? "Reject" : (order.status == 2 ? "Fill" : "PFill")) << "," 
             << order.quantity << "," << order.price << "\n";
    }
    
    // Indicate the end of data
    file << "..........................................................\n";
    // Placeholder for execution time (will be filled in main after processing)
    file << "Execution Time (microseconds): N/A\n";

    file.close();
}

void OrderProcessor::matchOrders(std::unordered_map<std::string, std::vector<Order>>& orderBook, Order newOrder, std::vector<Order>& execReports) {
    std::vector<Order>& matchingOrders = orderBook[newOrder.instrument];
    
    auto it = matchingOrders.begin();
    while (it != matchingOrders.end() && newOrder.quantity > 0) {
        Order& bookOrder = *it;
        
        // Check if the price matches (aggressive order executed at the passive order's price)
        bool priceMatch = (newOrder.side == 1 && newOrder.price >= bookOrder.price) ||  // Buy order, price >= Sell price
                          (newOrder.side == 2 && newOrder.price <= bookOrder.price);    // Sell order, price <= Buy price
        
        if (priceMatch) {
            if (newOrder.quantity >= bookOrder.quantity) {  // Full fill
                newOrder.quantity -= bookOrder.quantity;
                newOrder.status = (newOrder.quantity == 0) ? 2 : 3;  // Fill or Partial Fill
                execReports.push_back(newOrder);
                
                // Book order fully executed, remove it
                it = matchingOrders.erase(it);
            } else {  // Partial fill
                bookOrder.quantity -= newOrder.quantity;
                newOrder.quantity = 0;
                newOrder.status = 2;  // Fully filled
                execReports.push_back(newOrder);
                return;
            }
        } else {
            ++it;
        }
    }
    
    if (newOrder.quantity > 0) {
        newOrder.status = 0;  // New order added to book
        addToOrderBook(orderBook, newOrder);
    }
}

void OrderProcessor::addToOrderBook(std::unordered_map<std::string, std::vector<Order>>& orderBook, const Order& order) {
    orderBook[order.instrument].push_back(order);
    
    // Sort order book: Buy side sorted descending by price, Sell side sorted ascending by price
    if (order.side == 1) {  // Buy side
        std::sort(orderBook[order.instrument].begin(), orderBook[order.instrument].end(), [](const Order& a, const Order& b) {
            return a.price > b.price;
        });
    } else if (order.side == 2) {  // Sell side
        std::sort(orderBook[order.instrument].begin(), orderBook[order.instrument].end(), [](const Order& a, const Order& b) {
            return a.price < b.price;
        });
    }
}
