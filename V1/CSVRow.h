#ifndef CSVROW_H
#define CSVROW_H

#include <vector>
#include <string>

class CSVRow : public std::vector<std::string> {
public:
    CSVRow() = default;
};

#endif
