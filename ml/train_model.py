import numpy as np
import pandas as pd
from sklearn.linear_model import LogisticRegression
from sklearn.preprocessing import StandardScaler

np.random.seed(42)
n_samples = 3000

age = np.random.randint(21, 65, n_samples)
income = np.random.normal(450000, 150000, n_samples).clip(100000, 1500000)
balance = np.random.normal(600000, 350000, n_samples).clip(0, 4000000)
loan_amount = np.random.normal(1200000, 700000, n_samples).clip(200000, 6000000)
loan_term = np.random.choice([12, 24, 36, 48, 60], n_samples)

monthly_burden = loan_amount / loan_term
risk_factor = (income * 0.45 + balance * 0.25) - (monthly_burden * 3.5)
target = (risk_factor > np.median(risk_factor)).astype(int)

df = pd.DataFrame({
    'age': age,
    'income': income,
    'balance': balance,
    'loan_amount': loan_amount,
    'loan_term': loan_term,
    'target': target
})

X = df[['age', 'income', 'balance', 'loan_amount', 'loan_term']]
y = df['target']

scaler = StandardScaler()
X_scaled = scaler.fit_transform(X)

model = LogisticRegression()
model.fit(X_scaled, y)

print("weights = {" + ", ".join([f"{w:.6f}" for w in model.coef_[0]]) + "};")
print(f"intercept = {model.intercept_[0]:.6f};")
print("scalerMean = {" + ", ".join([f"{m:.6f}" for m in scaler.mean_]) + "};")
print("scalerScale = {" + ", ".join([f"{s:.6f}" for s in scaler.scale_]) + "};")
