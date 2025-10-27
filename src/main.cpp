/*
    main.cpp
*/

#include "../inc/stock.h"
#include "../inc/portfolio.h"
#include "../inc/risk.h"
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>

int main(int argc, char **argv) {
    stock test;
    test.ticker = "TEST";
    test.loadPrices("../data/dummy.csv");
    test.computeDailyReturns();

    double mean_return = test.computeMeanReturn();
    double volatility = test.computeVolatility();

    std::cout << "CP317 - Group 6, Backend Testing" << std::endl;
    std::cout << "-----------------------------" << std::endl;
    std::cout << " " << std::endl;

    std::cout << "Ticker:               " << test.ticker << std::endl;
    std::cout << "Data points:          " << test.prices.size() << std::endl; 
    std::cout << "Daily returns:        ";
    
    std::cout << test.returns[0];
    for (size_t i = 0; i < test.returns.size(); i++) {
        std::cout << "%, " << test.returns[i];
    }
    std::cout << " " << std::endl;
    
    risk risks;
    double annual_risk = 0.05; // assuming an annual risk of 5% for testing
    double sharpe_ratio = risks.computeSharpeRatio(mean_return, volatility, annual_risk);
    double var = risks.computeVaR(mean_return, volatility);

    std::cout << "Avg Daily Return:     " << mean_return << "%" << std::endl;
    std::cout << "Volatility:           " << volatility << "%" << std::endl;
    std::cout << "Sharpe Ratio:         " << sharpe_ratio << "%" << std::endl;
    std::cout << "Value at Risk:        " << var << "%" << std::endl;
    std::cout << "" << std::endl; 
    
    std::cout << "Risk Assessment" << std::endl;
    std::cout << "-----------------------------" << std::endl;
}

