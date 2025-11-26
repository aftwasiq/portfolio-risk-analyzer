from flask import Flask, render_template, request, jsonify, send_file, redirect, url_for, session
from werkzeug.security import generate_password_hash, check_password_hash
import sqlite3
from google import genai
import os
import subprocess
import yfinance as yf
import pandas as pd
import numpy as np

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = 'uploads'
os.makedirs(app.config['UPLOAD_FOLDER'], exist_ok=True)

app.secret_key = "abcd1234"

DB_PATH = "users.db"

def init_db():
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute("""
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL
        )
    """)
    c.execute("""
        CREATE TABLE IF NOT EXISTS history (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER,
            filename TEXT,
            total_return REAL,
            volatility REAL,
            upload_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(user_id) REFERENCES users(id)
        )
    """)
    conn.commit()
    conn.close()

init_db()

RISK_ENGINE_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'build', 'risk_engine'))

@app.route('/')
def index():
    if 'user_id' not in session:
        return redirect(url_for('login_page'))
    return render_template('index.html')

@app.route('/login')
def login_page():
    if 'user_id' in session:
        return redirect(url_for('index'))
    return render_template('login.html')

@app.route('/signup_page')
def signup_page():
    if 'user_id' in session:
        return redirect(url_for('index'))
    return render_template('signup.html')

@app.route('/signup', methods=['POST'])
def signup():
    username = request.form['username']
    password = request.form['password']

    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()

    try:
        c.execute("INSERT INTO users (username, password_hash) VALUES (?, ?)",
                  (username, generate_password_hash(password)))
        conn.commit()
        
        c.execute("SELECT id FROM users WHERE username=?", (username,))
        user = c.fetchone()
        session['user_id'] = user[0]
        session['username'] = username
        
        conn.close()
        return redirect(url_for('index'))
    except sqlite3.IntegrityError:
        conn.close()
        return "Username already taken", 400

@app.route('/login_submit', methods=['POST'])
def login_submit():
    username = request.form['username']
    password = request.form['password']

    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute("SELECT id, password_hash FROM users WHERE username=?", (username,))
    user = c.fetchone()
    conn.close()

    if user and check_password_hash(user[1], password):
        session['user_id'] = user[0]
        session['username'] = username
        return redirect(url_for('index'))

    return "Invalid login", 401

@app.route('/logout')
def logout():
    session.clear()
    return redirect(url_for('login_page'))

import time

def fetch_historical_data(tickers):
    """Fetch historical prices for multiple tickers and return a single merged DataFrame"""
    all_data = []
    
    ticker_corrections = {
        'FB': 'META',   
        'GOOG': 'GOOGL',   
    }

    tickers_with_sp500 = tickers + ["^GSPC"]

    for ticker in tickers_with_sp500:
        try:
            original_ticker = ticker
            corrected_ticker = ticker_corrections.get(ticker, ticker)
            
            if corrected_ticker != original_ticker:
                print(f"Note: Converting {original_ticker} -> {corrected_ticker}")
            
            time.sleep(1)  # Wait 1 second between requests
            
            stock = yf.Ticker(corrected_ticker)
            hist = stock.history(period="1y", auto_adjust=True)
            
            if hist.empty:
                print(f"Warning: No data for {ticker}")
                continue
                
            hist = hist.reset_index()[["Date", "Close"]]
            hist["Ticker"] = "S&P 500" if ticker == "^GSPC" else original_ticker
            
            print(f"{original_ticker}: {hist['Date'].min()} to {hist['Date'].max()}")
            
            all_data.append(hist)
        except Exception as e:
            print(f"Error fetching {ticker}: {e}")
            continue

    if not all_data:
        return None

    merged_df = pd.concat(all_data, ignore_index=True)
    merged_df = merged_df[["Ticker", "Date", "Close"]]
    return merged_df

def save_merged_csv(df):
    csv_path = os.path.join(app.config['UPLOAD_FOLDER'], "merged_prices.csv")
    df.to_csv(csv_path, index=False)
    return csv_path


def parse_cli_to_json(cli_output: str):
    """Parse C++ CLI output into JSON"""
    lines = cli_output.splitlines()
    data = {"stocks": [], "portfolio": {}}
    current_stock = None

    for line in lines:
        line = line.strip()
        if line.startswith("---STOCK_START---"):
            current_stock = {}
        elif line.startswith("---STOCK_END---"):
            data["stocks"].append(current_stock)
            current_stock = None
        elif line.startswith("---PORTFOLIO_START---") or line.startswith("---PORTFOLIO_END---"):
            continue
        else:
            try:
                key, val = line.split(":", 1)
                key = key.strip()
                val = val.strip()
                if current_stock is not None:
                    current_stock[key] = float(val) if key not in ["Ticker"] else val
                else:
                    data["portfolio"][key] = float(val)
            except Exception:
                continue
    return data


def calculate_sp500_metrics(merged_csv_path):
    """Calculate S&P 500 return and volatility from the merged CSV"""
    try:
        df = pd.read_csv(merged_csv_path)
        sp500_data = df[df['Ticker'] == 'S&P 500'].copy()
        
        if sp500_data.empty:
            return None
        
        sp500_data = sp500_data.sort_values('Date')
        sp500_data['Returns'] = sp500_data['Close'].pct_change()
        
        mean_return = sp500_data['Returns'].mean() * 252 * 100
        volatility = sp500_data['Returns'].std() * np.sqrt(252) * 100
        
        total_return = ((sp500_data['Close'].iloc[-1] / sp500_data['Close'].iloc[0]) - 1) * 100
        
        return {
            'TotalReturn': total_return,
            'MeanReturn': mean_return,
            'Volatility': volatility
        }
    except Exception as e:
        print(f"Error calculating S&P 500 metrics: {e}")
        return None

@app.route('/upload_csv', methods=['POST'])
def upload_csv():
    try:
        if 'file' not in request.files:
            return "No file uploaded", 400
        file = request.files['file']
        if file.filename == '':
            return "No selected file", 400

        portfolio_path = os.path.join(app.config['UPLOAD_FOLDER'], file.filename)
        file.save(portfolio_path)

        df_portfolio = pd.read_csv(portfolio_path)
        ticker_col = next((c for c in ['Ticker', 'Symbol', 'Instrument', 'Asset'] if c in df_portfolio.columns), None)

        if ticker_col is None:
            return "Portfolio CSV missing ticker column", 400

        tickers = df_portfolio[ticker_col].tolist()

        merged_df = fetch_historical_data(tickers)
        if merged_df is None or merged_df.empty:
            return "Failed to fetch historical data", 500
        merged_csv_path = save_merged_csv(merged_df)

        if not os.path.exists(RISK_ENGINE_PATH):
            return f"C++ risk engine not found at {RISK_ENGINE_PATH}", 500

        result = subprocess.run(
            [RISK_ENGINE_PATH, portfolio_path, merged_csv_path],
            capture_output=True, text=True
        )

        if result.returncode != 0:
            return f"C++ engine failed:\n{result.stderr}", 500

        portfolio_json = parse_cli_to_json(result.stdout)
        
        sp500_metrics = calculate_sp500_metrics(merged_csv_path)
        if sp500_metrics:
            portfolio_json['sp500'] = sp500_metrics

        if 'user_id' in session:
            portfolio = portfolio_json.get('portfolio', {})
            conn = sqlite3.connect(DB_PATH)
            c = conn.cursor()
            c.execute(
                "INSERT INTO history (user_id, filename, total_return, volatility) VALUES (?, ?, ?, ?)",
                (session['user_id'], file.filename,
                 portfolio.get('TotalReturn', 0),
                 portfolio.get('PortfolioVolatility', 0))
            )
            conn.commit()
            conn.close()

        return jsonify(portfolio_json)

    except Exception as e:
        import traceback
        traceback.print_exc()
        return f"Exception occurred:\n{str(e)}", 500


@app.route('/history')
def history():
    if 'user_id' not in session:
        return redirect(url_for('login_page'))

    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute("""
        SELECT filename, total_return, volatility, upload_time
        FROM history
        WHERE user_id=?
        ORDER BY upload_time DESC
    """, (session['user_id'],))
    rows = c.fetchall()
    conn.close()

    return render_template("history.html", history=rows)


client = genai.Client(api_key="AIzaSyBu89WlvvRQFJYkwP23-oRh6bkaR2zeR6s")

def interpret_portfolio(summary):
    prompt = f"""
    interpret this user's stock portfolio in a simple way that a beginner investor can understand,
    be sure to include a precise summary of what everything means, aswell as a layman-based description

    Portfolio Summary:
    - Total Return: {summary['total_return']*100:.2f}%
    - Portfolio Volatility: {summary['volatility']*100:.2f}%

    S&P 500 Comparison:
    - S&P 500 Return: {summary.get('sp500_return', 0)*100:.2f}%
    - S&P 500 Volatility: {summary.get('sp500_volatility', 0)*100:.2f}%

    Stocks:
    {[
        f"{s['ticker']} — Weight: {s['weight']*100:.1f}%, "
        f"Return: {s['mean_return']*100:.2f}%, "
        f"Volatility: {s['volatility']*100:.2f}%"
        for s in summary['stocks']
    ]}

    key points to emphasize on:
    - which stocks are contributing most to risk?
    - which ones look strongest long-term?
    - any diversification warnings
    - a short suggestion for next steps (which ones you should invest in, 
    which ones to refrain from, etc.

    requirements (IMPORTANT!):
    - please keep it only THREE paragraphs long, don't bother formatting
    - each paragraph should be a maximum of 2-3 sentences (whatever takes less time)
    - format all paragraphs in html (there should be space between paragraphs)
    - title each paragraph in bold, and add an emoji before the title
    - first two paragraphs: general portfolio analysis
    - THIRD paragraph MUST have the title "📊 Individual Stocks vs S&P 500"
    - in the third paragraph, compare the user's portfolio performance against the S&P 500
    - provide a clear recommendation: should they continue with individual stock investing 
      or would they be better off just investing in an S&P 500 index fund?
    - base this recommendation on their returns, volatility, and risk-adjusted performance
    - please remember, this is a layman interpretation. it should as beginner friendly as possible
    """

    response = client.models.generate_content(
        model="gemini-2.5-flash", contents=prompt
    )

    return response.text

@app.get("/historical_data")
def historical_data():
    return send_file("../uploads/merged_prices.csv", mimetype="text/csv")

@app.route('/interpret', methods=['POST'])
def interpret():
    try:
        summary = request.json
        analysis = interpret_portfolio(summary)
        return jsonify({"analysis": analysis})
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
