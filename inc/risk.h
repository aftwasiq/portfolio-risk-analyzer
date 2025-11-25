/*
    risk.h
    Header file for risk.h implementations
*/
#ifndef RISK_H
#define RISK_H

class risk {
public:
    double computeSharpeRatio(double mean_return, double volatility, double annual_risk);
    double computeVaR(double mean_return, double volatility);
};

#endif
