#include <ESP8266WiFi.h>        // ← Changed from WiFi.h
#include <ESP8266WebServer.h>   // ← Changed from WebServer.h
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "MAX30105.h"
#include "heartRate.h"

// --- WiFi Credentials ---
const char* ssid = "OME NETWORK";
const char* pass = "@BASKARAN";

// --- Hardware Setup ---
// ESP8266 GPIO pins (D5=GPIO14, D6=GPIO12, D7=GPIO13)
#define RED_LED 14  // D5 ← Changed from 25
#define YEL_LED 12  // D6 ← Changed from 26
#define GRN_LED 13  // D7 ← Changed from 27

MAX30105 particleSensor;
LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);   // ← Changed from WebServer

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg = 0;
unsigned long lastSerialUpdate = 0;

void handleData() {
  String json = "{\"bpm\": " + String(beatAvg) + "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(9600);
  pinMode(RED_LED, OUTPUT);
  pinMode(YEL_LED, OUTPUT);
  pinMode(GRN_LED, OUTPUT);

  // ESP8266 default I2C: SDA=D2(GPIO4), SCL=D1(GPIO5)
  Wire.begin(4, 5);  // ← Changed from Wire.begin(21, 22)
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
  lcd.print("IP:");
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

        Serial.println("{\"bpm\": " + String(beatAvg) + "}");
      }
    }
  }
}