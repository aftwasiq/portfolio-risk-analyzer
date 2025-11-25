#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <vector>
#include "stock.h"

class portfolio {
public:
    std::vector<stock> stocks;
    std::vector<double> weights;
    std::vector<double> shares;

    double computePortfolioReturn() const;
    double computePortfolioVolatility() const;
    void computeWeights();
};

#endif

