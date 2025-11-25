#include "../inc/portfolio.h"
#include <numeric>
#include <iostream>

void portfolio::computeWeights() {
    double total_shares = std::accumulate(shares.begin(), shares.end(), 0.0);
    weights.clear();
    for (double s : shares) weights.push_back(s / total_shares);
}

double portfolio::computePortfolioReturn() const {
    double ret = 0;
    for (size_t i=0;i<stocks.size();i++)
        ret += stocks[i].computeMeanReturn() * weights[i];
    return ret;
}

double portfolio::computePortfolioVolatility() const {
    double vol = 0;
    for (size_t i=0;i<stocks.size();i++)
        vol += stocks[i].computeVolatility() * weights[i];
    return vol;
}

