#include "../inc/stock.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <unordered_map>
#include <algorithm>

std::string stock::trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    auto end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

bool safe_stod(const std::string& s, double& out) {
    try {
        out = std::stod(s);
        return true;
    } catch (...) {
        return false;
    }
}

bool stock::loadPricesFromCSV(const std::string& filepath, const std::string& ticker_name) {
    prices.clear();

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "cannot open file: " << filepath << std::endl;
        return false;
    }

    std::string header;
    if (!std::getline(file, header)) {
        std::cerr << ".csv empty" << std::endl;
        return false;
    }

    std::unordered_map<std::string,int> col_map;
    std::stringstream hs(header);
    std::string h;
    int idx = 0;

    while (std::getline(hs,h,',')) {
        std::string clean = trim(h);
        if (clean == "Date" || clean == "Trade Date" || clean == "Timestamp") {
            col_map["date"] = idx;
        }

        if (clean == "Close" || clean == "Close Price" || clean == "Market Value" || clean == "Price") {
            col_map["close"] = idx;
        }

        if (clean == "Ticker" || clean == "Symbol" || clean == "Instrument" || clean == "Asset") {
            col_map["ticker"] = idx;
        }

        idx++;
    }

    if (!col_map.count("date") || !col_map.count("close") || !col_map.count("ticker")) {
        std::cerr << ".csv missing required columns" << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file,line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::vector<std::string> fields;
        std::string item;
        
        while (std::getline(ss,item,',')) {
            fields.push_back(trim(item));
        }

        if (fields.size() <= std::max({col_map["date"], col_map["close"], col_map["ticker"]})) {
            continue;
        }
        
        if (fields[col_map["ticker"]] != ticker_name) {
            continue;
        }

        double close;
        if (!safe_stod(fields[col_map["close"]], close)) {
            continue;
        }
        prices.push_back(close);
    }

    file.close();
    return !prices.empty();
}

void stock::computeDailyReturns() {
    returns.clear();
    if (prices.size() < 2) {
        return;
    }
    for (size_t i=1; i<prices.size(); i++) {
        returns.push_back((prices[i]-prices[i-1])/prices[i-1]*100.0);
    }
}

double stock::computeMeanReturn() const {
    double sum=0;
    if (returns.empty()) {
        return 0.0;
    }

    for (auto r : returns) {
        sum += r;
    }
    
    return sum/returns.size();
}

double stock::computeVolatility() const {
    double mean = computeMeanReturn();
    double var = 0.0;
    
    if (returns.empty()) {
        return 0.0;
    }

    for (auto r : returns) {
        var += (r-mean)*(r-mean);
    }

    return std::sqrt(var/returns.size());
}

