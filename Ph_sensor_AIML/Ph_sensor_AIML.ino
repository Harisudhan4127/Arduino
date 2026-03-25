#include <WiFi.h>
#include <WebServer.h>

#define SensorPin 34   // ESP32 ADC1 pin

// WiFi credentials
const char* ssid = "pH Sensor";
const char* password = "9159205309";

// Calibration values
float slope = -7.24;
float intercept = 26.7;

WebServer server(80);

// Sensor variables
int buf[10], temp;
unsigned long avgValue;

// ------------------ SENSOR READING ------------------

float readVoltage() {
  for (int i = 0; i < 10; i++) {
    buf[i] = analogRead(SensorPin);
    delay(10);
  }

  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buf[i] > buf[j]) {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }

  avgValue = 0;
  for (int i = 2; i < 8; i++)
    avgValue += buf[i];

  return (avgValue * 3.3) / (4095.0 * 6);
}

float readPH(float voltage) {
  return slope * voltage + intercept;
}

// ------------------ AI / ML MODEL ------------------

/*
  Logistic Regression Inspired ML Model
  z = (w × pH) + b
  probability = 1 / (1 + e^-z)
*/

String predictUlcer(float pH) {

  // Example trained weights (replace with your ML result if available)
  float weight = -3.5;     // w
  float bias = 18.0;       // b

  float z = (weight * pH) + bias;
  float probability = 1.0 / (1.0 + exp(-z));

  if (probability > 0.70)
    return "HIGH RISK (AI Detected)";
  else if (probability > 0.40)
    return "MODERATE RISK";
  else
    return "LOW RISK";
}

// ------------------ pH STATUS ------------------

String phStatus(float pH) {
  if (pH < 4) return "Very Acidic";
  else if (pH < 6.5) return "Acidic";
  else if (pH <= 7.5) return "Neutral";
  else if (pH < 9) return "Alkaline";
  else return "Strong Alkaline";
}

// ------------------ WEB DASHBOARD ------------------

void handleRoot() {

  float voltage = readVoltage();
  float pH = readPH(voltage);
  String risk = predictUlcer(pH);

  // Serial Output
  Serial.print("Voltage: ");
  Serial.print(voltage, 3);
  Serial.print(" V | pH: ");
  Serial.print(pH, 2);
  Serial.print(" | AI Risk: ");
  Serial.println(risk);

  String page = "<!DOCTYPE html><html><head>";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  page += "<meta http-equiv='refresh' content='2'>";
  page += "<style>";
  page += "body{font-family:Arial;background:#f4f6f8;text-align:center;}";
  page += ".card{background:#fff;padding:20px;margin:20px;border-radius:15px;";
  page += "box-shadow:0 5px 15px rgba(0,0,0,0.2);} ";
  page += "h1{color:#2c3e50;} h2{color:#16a085;} h3{color:#e74c3c;}";
  page += "</style></head><body>";

  page += "<div class='card'>";
  page += "<h1>AI Saliva Ulcer Detection</h1>";
  page += "<h2>Voltage: " + String(voltage, 3) + " V</h2>";
  page += "<h2>pH Value: " + String(pH, 2) + "</h2>";
  page += "<h2>Status: " + phStatus(pH) + "</h2>";
  page += "<h3>ML Prediction: " + risk + "</h3>";
  page += "</div>";

  page += "<p>ESP32 | Embedded AI Model</p>";
  page += "</body></html>";

  server.send(200, "text/html", page);
}

// ------------------ SETUP ------------------

void setup() {
  Serial.begin(115200);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  WiFi.softAP(ssid, password);

  Serial.println("WiFi AP Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();
}

// ------------------ LOOP ------------------

void loop() {
  server.handleClient();
}