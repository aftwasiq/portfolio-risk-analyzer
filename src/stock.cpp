/*
    stock.cpp
    Loads in csv format data and processes it
*/

#include "../inc/stock.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>

void stock::loadPrices(const std::string& filepath) {
    returns.clear();

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "Could not open file" << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string date, closer;
        std::getline(ss, date, ',');
        std::getline(ss, closer, ',');

        closer.erase(0, closer.find_first_not_of(" \t\r\n"));
        closer.erase(closer.find_last_not_of(" \t\r\n") + 1);

        if (closer.empty()) {
            continue;
        }

        double close_price = std::stod(closer);
        prices.push_back(close_price);
    }

    file.close();
}

void stock::computeDailyReturns() {
    if (prices.size() < 2) {
        return;
    }
    
    returns.clear();
    for(size_t i = 1; i < prices.size(); i++) {
        returns.push_back(((prices[i] - prices[i - 1]) / prices[i - 1]) * 100);
    }
}

double stock::computeMeanReturn() {
    if (returns.empty()) {
       return 0.0;
    }

    double total = 0.0;
    for (size_t i = 0; i < returns.size(); i++) {
        total += returns[i];
    }

    double mean = total / returns.size();
    return mean;
}

double stock::computeVolatility(){
    if (returns.empty()) {
        return 0.0;
    }
    
    double mean = computeMeanReturn();
    double variance = 0.0;

    for (size_t i = 0; i < returns.size(); i++) {
        variance += ((returns[i] - mean) * (returns[i] - mean));
    }
    
    variance /= returns.size();
    double volatility = std::sqrt(variance);    
    return volatility;
}
