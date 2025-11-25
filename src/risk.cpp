/*
    risk.cpp
    Calculates risk metrics, such as covariance & correlation matrix, 
    portfolio volatility, simple value at-risk, and the sharpe ratio.
*/

#include "../inc/risk.h"
#include <cmath>

/*
    risk::computeSharpeRatio()
    calculates sharpe ratio based on provided risk metrics
*/

double risk::computeSharpeRatio(double mean_return, double volatility, double annual_risk) {
    double daily_risk_free = pow(1.0 + annual_risk, 1.0/252.0) - 1.0; 
    double sharpe = (mean_return - daily_risk_free) / volatility;

    return sharpe;
}

double risk::computeVaR(double mean_return, double volatility) {
    // daily return - daily volatility * z score
    double z_score = 1.645; // temporary z-score based on a confidence of 95 
    double var = 0.0;

    var = mean_return - (volatility * z_score);
    return var;
}

// still need to add  covariance, correlation matrix, etc
