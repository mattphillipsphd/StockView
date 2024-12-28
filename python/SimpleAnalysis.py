#!/usr/bin/env python3
import sys
import pandas as pd
import numpy as np
from datetime import datetime

def analyze_stock_data(file_path, ticker):
    # Read the CSV file
    df = pd.read_csv(file_path)
    
    # Convert timestamp to datetime
    df['date'] = pd.to_datetime(df['timestamp'], unit='s')
    df = df.set_index('date')
    
    # Calculate basic statistics
    latest_price = df['price'].iloc[-1]
    avg_price = df['price'].mean()
    std_price = df['price'].std()
    
    # Calculate daily returns
    df['returns'] = df['price'].pct_change()
    
    # Calculate volatility (annualized)
    volatility = df['returns'].std() * np.sqrt(252)
    
    # Calculate moving averages
    df['SMA_20'] = df['price'].rolling(window=20).mean()
    df['SMA_50'] = df['price'].rolling(window=50).mean()
    
    # Determine trend
    current_sma20 = df['SMA_20'].iloc[-1]
    current_sma50 = df['SMA_50'].iloc[-1]
    trend = "UPWARD" if current_sma20 > current_sma50 else "DOWNWARD"
    
    # Calculate Relative Strength Index (RSI)
    delta = df['returns']
    gain = (delta.where(delta > 0, 0)).rolling(window=14).mean()
    loss = (-delta.where(delta < 0, 0)).rolling(window=14).mean()
    rs = gain / loss
    rsi = 100 - (100 / (1 + rs))
    current_rsi = rsi.iloc[-1]
    
    # Format results
    analysis = f"""Stock Analysis for {ticker}:
------------------------
Latest Price: ${latest_price:.2f}
Average Price: ${avg_price:.2f}
Standard Deviation: ${std_price:.2f}
Volatility (Annualized): {volatility*100:.1f}%
Current Trend: {trend}
RSI (14-day): {current_rsi:.1f}

Technical Signals:
------------------------
RSI Signal: {"OVERSOLD" if current_rsi < 30 else "OVERBOUGHT" if current_rsi > 70 else "NEUTRAL"}
Trend Signal: {"BULLISH" if trend == "UPWARD" else "BEARISH"}
"""
    
    return analysis

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python stock_analyzer.py <data_file_path> <ticker>")
        sys.exit(1)
        
    file_path = sys.argv[1]
    ticker = sys.argv[2]
    
    try:
        analysis = analyze_stock_data(file_path, ticker)
        print(analysis)
    except Exception as e:
        print(f"Error analyzing stock data: {str(e)}")
        sys.exit(1)