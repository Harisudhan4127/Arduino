import sys
import subprocess
import pandas as pd
import joblib
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier

# ==============================
# 1️⃣ TRAIN MODEL
# ==============================

print("Training model...")

df = pd.read_csv("Ulcer_Saliva_Dataset_300.csv")
df.columns = df.columns.str.strip()

# Convert categorical columns
df["Gender"] = df["Gender"].map({"Male": 1, "Female": 0})

yes_no_cols = [
    "Spicy_Food","Tobacco","Alcohol",
    "Skip_Meals","Soft_Drinks","Empty_Stomach_Pain"
]

for col in yes_no_cols:
    df[col] = df[col].map({"Yes": 1, "No": 0})

X = df.drop("Ulcer", axis=1)
y = df["Ulcer"]

X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

model = RandomForestClassifier(n_estimators=100, random_state=42)
model.fit(X_train, y_train)

joblib.dump(model, "ulcer_model.pkl")

print("Model saved successfully!")

# ==============================
# 2️⃣ AUTOMATICALLY OPEN GUI
# ==============================

print("Launching GUI...")
subprocess.Popen([sys.executable, "ulcer_gui_modern.py"])