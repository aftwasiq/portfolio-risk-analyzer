/*
    stock.h
    Header file for stock class
*/

#ifndef STOCK_H
#define STOCK_H

#include <iostream>
#include <vector>

class stock {
public:
    std::string ticker;
    std::vector<double> prices;
    std::vector<double> returns;

    void loadPrices(const std::string& filepath);
    void computeDailyReturns();
    double computeMeanReturn();
    double computeVolatility();
};

#endif
