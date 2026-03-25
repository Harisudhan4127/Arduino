#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "MAX30105.h"
#include "heartRate.h"

// --- WiFi Credentials ---
const char* ssid = "Sri";
const char* pass = "Sri_Home_Bas_Raji";

// --- Hardware Setup ---
#define RED_LED 25
#define YEL_LED 26
#define GRN_LED 27

MAX30105 particleSensor;
LiquidCrystal_I2C lcd(0x27, 16, 2);
WebServer server(80);

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg = 0;
unsigned long lastSerialUpdate = 0;

void handleData() {
  // Python Dashboard calls this via http://192.168.1.7/data
  String json = "{\"bpm\": " + String(beatAvg) + "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  pinMode(RED_LED, OUTPUT);
  pinMode(YEL_LED, OUTPUT);
  pinMode(GRN_LED, OUTPUT);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.print("MDS: Connecting");

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  server.on("/data", handleData);
  server.begin();

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 not found.");
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A); 
  particleSensor.setPulseAmplitudeGreen(0);

  lcd.clear();
  lcd.print("IP: ");
  lcd.print(WiFi.localIP());
  Serial.println("\nReady. IP: " + WiFi.localIP().toString());
}

void loop() {
  server.handleClient(); 

  long irValue = particleSensor.getIR();

  // FINGER NOT DETECTED
  if (irValue < 50000) {
    beatAvg = 0;
    digitalWrite(YEL_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    digitalWrite(GRN_LED, LOW);

    lcd.setCursor(0, 1);
    lcd.print("Place Finger... ");
    
    // Send "Zero" to Serial every 500ms so Python knows we are still alive
    if (millis() - lastSerialUpdate > 500) {
       Serial.println("{\"bpm\": 0}");
       lastSerialUpdate = millis();
    }
  } 
  // FINGER DETECTED
  else {
    digitalWrite(YEL_LED, LOW);

    if (checkForBeat(irValue) == true) {
      long delta = millis() - lastBeat;
      lastBeat = millis();
      beatsPerMinute = 60 / (delta / 1000.0);

      if (beatsPerMinute < 255 && beatsPerMinute > 20) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;

        beatAvg = 0;
        for (byte x = 0; x < RATE_SIZE; x++) beatAvg += rates[x];
        beatAvg /= RATE_SIZE;

        // Visual Feedback
        if (beatAvg > 100) {
          digitalWrite(RED_LED, HIGH);
          digitalWrite(GRN_LED, LOW);
        } else {
          digitalWrite(RED_LED, LOW);
          digitalWrite(GRN_LED, HIGH);
        }

        lcd.setCursor(0, 1);
        lcd.print("BPM: ");
        lcd.print(beatAvg);
        lcd.print(" (Live)   ");

        // Send JSON to Serial for the USB Python Dashboard
        Serial.println("{\"bpm\": " + String(beatAvg) + "}");
      }
    }
  }
}