#!/usr/bin/env python3
import sys
import pandas as pd
import numpy as np
from datetime import datetime, timedelta
from scipy import stats

def estimate_ou_parameters(prices, dt):
    """
    Estimate OU parameters using maximum likelihood estimation
    with stability constraints
    """
    log_prices = np.log(prices)
    returns = np.diff(log_prices)
    
    # Calculate basic statistics
    n = len(returns)
    sigma_emp = np.std(returns)
    
    # Estimate theta using auto-correlation with constraints
    lag_1_corr = np.corrcoef(returns[:-1], returns[1:])[0,1]
    theta = min(max(-np.log(abs(lag_1_corr)) / dt, 0.1), 10.0)  # Bound theta
    
    # Estimate long-term mean (mu) as exponentially weighted mean
    mu = np.average(log_prices, weights=np.exp(-theta * np.arange(len(log_prices))[::-1]))
    
    # Estimate volatility with bias correction
    sigma = sigma_emp * np.sqrt(2 * theta / (1 - np.exp(-2 * theta * dt)))
    sigma = min(sigma, 0.5)  # Cap volatility for stability
    
    return theta, mu, sigma

def predict_stock_prices(file_path, ticker, prediction_days=30):
    """
    Predict future stock prices using a stabilized OU process model.
    """
    # Read and prepare data
    df = pd.read_csv(file_path)
    df['datetime'] = pd.to_datetime(df['timestamp'], unit='s')
    df = df.set_index('datetime')
    
    # Calculate time step
    timestamps = pd.to_datetime(df['timestamp'], unit='s')
    dt = (timestamps.iloc[1] - timestamps.iloc[0]).total_seconds()
    
    # Estimate parameters
    theta, mu, sigma = estimate_ou_parameters(df['price'].values, dt)
    
    # Generate predictions
    n_steps = int(prediction_days * 24 * 60 * 60 / dt)
    last_price = df['price'].iloc[-1]
    last_timestamp = df.index[-1]
    
    predictions = []
    current_log_price = np.log(last_price)
    current_timestamp = last_timestamp
    
    # Simple moving average for trend adjustment
    window_size = min(20, len(df))
    trend = np.mean(np.diff(np.log(df['price'].tail(window_size))))
    
    for _ in range(n_steps):
        # Modified OU process with trend adjustment and bounds
        dW = np.random.normal(0, np.sqrt(dt))
        dx = theta * (mu - current_log_price) * dt + sigma * dW + trend * dt
        
        # Apply stability controls
        dx = np.clip(dx, -0.1, 0.1)  # Limit daily moves
        current_log_price += dx
        
        # Convert back to price with bounds
        current_price = np.exp(current_log_price)
        current_price = np.clip(current_price, last_price * 0.5, last_price * 2.0)
        
        current_timestamp += timedelta(seconds=int(dt))
        
        predictions.append({
            'timestamp': int(current_timestamp.timestamp()),
            'price': current_price
        })
    
    # Create prediction DataFrame
    pred_df = pd.DataFrame(predictions)
    
    # Combine original and predicted data
    combined_df = pd.concat([
        df.reset_index()[['timestamp', 'price']],
        pred_df[['timestamp', 'price']]
    ]).reset_index(drop=True)
    
    # Save to file
    output_path = f"{file_path.rsplit('.', 1)[0]}_with_predictions.txt"
    combined_df.to_csv(output_path, index=False, float_format='%.2f')
    
    # Generate analysis summary
    analysis = f"""Stock Price Prediction Analysis for {ticker}:
------------------------
Original Data Points: {len(df)}
Predicted Data Points: {len(pred_df)}
Last Actual Price: ${last_price:.2f}
Final Predicted Price: ${predictions[-1]['price']:.2f}
Prediction Timeframe: {prediction_days} days
Model Parameters:
- Mean Reversion Speed (theta): {theta:.6f}
- Long-term Mean Level: ${np.exp(mu):.2f}
- Volatility (sigma): {sigma:.6f}
- Detected Trend: {trend*100:.2f}% per period

Estimate: Yes
Output saved to: {output_path}
"""
    
    return analysis

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python stock_predictor.py <data_file_path> <ticker>")
        sys.exit(1)
        
    file_path = sys.argv[1]
    ticker = sys.argv[2]
    
    try:
        analysis = predict_stock_prices(file_path, ticker)
        print(analysis)
    except Exception as e:
        print(f"Error predicting stock prices: {str(e)}")
        sys.exit(1)
