/*
    main.cpp
    C++ CLI portfolio risk engine (outputs text for Python parsing)
*/

#include "../inc/stock.h"
#include "../inc/portfolio.h"
#include "../inc/risk.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: risk_engine <portfolio_csv> <merged_prices_csv>\n";
        return 1;
    }

    std::string portfolio_csv = argv[1];
    std::string prices_csv = argv[2];

    std::unordered_map<std::string, double> ticker_to_shares;
    std::ifstream file(portfolio_csv);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << portfolio_csv << "\n";
        return 1;
    }

    std::string line;
    getline(file, line);
    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string ticker, qty_str;

        if (!getline(ss, ticker, ',')) continue;
        if (!getline(ss, qty_str, ',')) continue;

        double qty = std::stod(qty_str);
        ticker_to_shares[ticker] += qty; 
    }

    std::vector<std::string> tickers;
    std::vector<double> shares;
    for (auto& [ticker, qty] : ticker_to_shares) {
        tickers.push_back(ticker);
        shares.push_back(qty);
    }

    portfolio pf;
    pf.shares = shares;

    for (size_t i = 0; i < tickers.size(); ++i) {
        stock s(tickers[i]);
        if (!s.loadPricesFromCSV(prices_csv, tickers[i])) {
            std::cerr << "[WARNING] No data for " << tickers[i] << "\n";
            continue;
        }
        s.computeDailyReturns();
        pf.stocks.push_back(s);
    }

    pf.computeWeights();

    risk r;
    std::cout << "---PORTFOLIO_START---\n";
    for (size_t i = 0; i < pf.stocks.size(); ++i) {
        const stock& s = pf.stocks[i];
        double mean = s.computeMeanReturn();
        double vol = s.computeVolatility();
        double sharpe = r.computeSharpeRatio(mean, vol, 0.05);
        double var = r.computeVaR(mean, vol);

        std::cout << "---STOCK_START---\n";
        std::cout << "Ticker: " << s.ticker << "\n";
        std::cout << "Shares: " << pf.shares[i] << "\n";
        std::cout << "Weight: " << pf.weights[i] << "\n";
        std::cout << "MeanReturn: " << mean << "\n";
        std::cout << "Volatility: " << vol << "\n";
        std::cout << "Sharpe: " << sharpe << "\n";
        std::cout << "VaR: " << var << "\n";
        std::cout << "---STOCK_END---\n";
    }

    double port_ret = pf.computePortfolioReturn();
    double port_vol = pf.computePortfolioVolatility();
    std::cout << "TotalReturn: " << port_ret << "\n";
    std::cout << "PortfolioVolatility: " << port_vol << "\n";
    std::cout << "---PORTFOLIO_END---\n";

    return 0;
}
