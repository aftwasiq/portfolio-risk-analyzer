# Portfolio Risk Analyzer
## CP317 - Term Project
### Created by: Abdullah Wasiq, Zohaib Ali, Maaz Ahmed, Rayyan Ahmed, Parth Bhatt

This project aims to develop a web-based portfolio risk analyzer tool that helps individual investors (specifically amateur) evaluate the risk of their stock investments. The system will accept the user’s portfolio data, like stock symbols, quantities, and purchase dates, then calculate financial risk metrics including volatility, value-at-risk (VaR), sharpe ratios, and diversification indexes. The results will be presented in a user-friendly dashboard to support better investment decisions, help users learn more about what they’re doing while they invest, and ultimately provide a better experience navigating the stock market with safety and effectiveness.

## Project structure

```
risk-analyzer/
├── src/
│   ├── main.cpp          # temporary cli testing
│   ├── stock.cpp         # stock class implementation
│   ├── risk.cpp          # risk metrics implementation
│   └── portfolio.cpp     # portfolio class (planned / optional)
├── inc/
│   ├── stock.h           # stock class declarations
│   ├── risk.h            # risk class declarations
│   └── portfolio.h       # portfolio class declarations
├── data/
│   └── dummy.csv         # trial data
└── README.md             
```

## Checklist

- [x] Load CSV stock data
- [x] Compute daily returns
- [x] Compute risk metrics
- [x] Analyze risk metrics 
- [x] Text-based feedback/interpretation
- [ ] Portfolio class (multi-stock support)
- [ ] Save analysis history
- [ ] Fetch real-time & historical data with API calls
- [ ] User interface (web-app)
