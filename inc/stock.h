#ifndef STOCK_H
#define STOCK_H

#include <string>
#include <vector>

class stock {
public:
    std::string ticker;
    std::vector<double> prices;
    std::vector<double> returns;

    stock() = default;
    stock(const std::string& t) : ticker(t) {}

    std::string trim(const std::string& s);
    bool loadPricesFromCSV(const std::string& filepath, const std::string& ticker_name);
    void computeDailyReturns();
    double computeMeanReturn() const;
    double computeVolatility() const;
};

#endif

