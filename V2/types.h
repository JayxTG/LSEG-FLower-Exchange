#pragma once

#include <string>
#include <vector>
#include <set>

struct CSVRow {
    std::string ord;
    std::string column1;
    std::string column2;
    std::string column3;
    std::string newColumn;
    std::string column4;
    std::string column5;
};

const std::set<std::string> VALID_INSTRUMENTS = { "Rose", "Lavender", "Lotus", "Tulip", "Orchid" };
const int MIN_PRICE = 10;
const int MAX_PRICE = 1000;
const int PRICE_MULTIPLE = 10;
