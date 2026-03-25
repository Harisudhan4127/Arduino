import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score, classification_report

# 🔹 Load dataset (USE YOUR REAL PATH)
df = pd.read_csv(r"data.csv")

# 🔹 Remove extra spaces in column names
df.columns = df.columns.str.strip()

# 🔹 Convert Yes/No columns manually
yes_no_columns = [
    "Spicy_Food", "Tobacco", "Alcohol",
    "Skip_Meals", "Soft_Drinks",
    "Empty_Stomach_Pain"
]

for col in yes_no_columns:
    df[col] = df[col].map({"Yes": 1, "No": 0})

# 🔹 Convert Gender manually
df["Gender"] = df["Gender"].map({"Male": 1, "Female": 0})

# 🔹 Check if any object column still exists
print("Data Types:\n", df.dtypes)

# 🔹 Split features & target
X = df.drop("Ulcer", axis=1)
y = df["Ulcer"]

# 🔹 Train-test split
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

# 🔹 Random Forest Model
model = RandomForestClassifier(n_estimators=100, random_state=42)
model.fit(X_train, y_train)

# 🔹 Prediction
y_pred = model.predict(X_test)

# 🔹 Evaluation
print("\nAccuracy:", accuracy_score(y_test, y_pred))
print("\nClassification Report:\n", classification_report(y_test, y_pred))