#include <WiFi.h>
#include <WebServer.h>

#define SensorPin 34   // ESP32 ADC1 pin

// WiFi credentials
const char* ssid = "pH Sensor";
const char* password = "9159205309";

// Calibration values (your latest working  values)
float slope = -7.24;
float intercept = 26.7;

WebServer server(80);

// Sensor variables
int buf[10], temp;
unsigned long avgValue;

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

String phStatus(float pH) {
  if (pH < 4) return "Very Acidic";
  else if (pH < 6.5) return "Acidic";
  else if (pH <= 7.5) return "Neutral";
  else if (pH < 9) return "Alkaline";
  else return "Strong Alkaline";
}

void handleRoot() {
  float voltage = readVoltage();
  float pH = readPH(voltage);

  String page = "<!DOCTYPE html><html><head>";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  page += "<meta http-equiv='refresh' content='2'>";
  page += "<style>";
  page += "body{font-family:Arial;background:#f4f6f8;text-align:center;}";
  page += ".card{background:#fff;padding:20px;margin:20px;border-radius:15px;";
  page += "box-shadow:0 5px 15px rgba(0,0,0,0.2);} ";
  page += "h1{color:#2c3e50;} h2{color:#16a085;}";
  page += "</style></head><body>";

  page += "<div class='card'>";
  page += "<h1>pH Sensor Dashboard</h1>";
  page += "<h2>Voltage: " + String(voltage, 3) + " V</h2>";
  page += "<h2>pH Value: " + String(pH, 2) + "</h2>";
  page += "<h2>Status: " + phStatus(pH) + "</h2>";
  page += "</div>";

  page += "<p>Device: ESP32 | Live Data</p>";
  page += "</body></html>";

  server.send(200, "text/html", page);
}

void setup() {
  Serial.begin(115200);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  // Start WiFi Access Point
  WiFi.softAP(ssid, password);

  Serial.println("WiFi AP Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
}
