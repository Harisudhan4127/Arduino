// Made by Harisudhan
// www.youtube.com/@harisudhantechnology

#include <Wire.h>

const int green = 13;
const int yellow = 12;
const int red = 11;
const int buzzer = 10;
const int pump_in = 9;
const int pump_out = 8;
const int sensor = 5;

int motionState = LOW;
int lastMoistureLevel = -1;

unsigned long lastMoistureTime = 0;
unsigned long lastBuzzerTime = 0;

const unsigned long moistureInterval = 500;
const unsigned long buzzerInterval = 500;

void setup() {
  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(pump_in, OUTPUT);
  pinMode(pump_out, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(sensor, INPUT);

  Serial.begin(9600);
}

void loop() {
  handleMotion();
  handleMoisture();
}

// ---------------- MOTION HANDLING ----------------
void handleMotion() {
  int val = digitalRead(sensor);

  if (val == HIGH && motionState == LOW) {
    Serial.println("Motion detected");
    for (int i = 0; i < 4; i++) {
      digitalWrite(buzzer, HIGH);
      delay(50);
      digitalWrite(buzzer, LOW);
      delay(50);
    }
    lastBuzzerTime = millis();
    motionState = HIGH;
  }

  if (motionState == HIGH && millis() - lastBuzzerTime >= buzzerInterval) {
    digitalWrite(buzzer, LOW);
  }

  if (val == LOW && motionState == HIGH) {
    Serial.println("Motion stopped");
    motionState = LOW;
  }
}

// ---------------- MOISTURE HANDLING ----------------
void handleMoisture() {
  if (millis() - lastMoistureTime < moistureInterval) return;
  lastMoistureTime = millis();

  int level = analogRead(A0);
  int currentLevel;

  if (level < 300) currentLevel = 3;
  else if (level < 500) currentLevel = 2;
  else currentLevel = 1;

  if (currentLevel == lastMoistureLevel) return;
  lastMoistureLevel = currentLevel;

  Serial.print("Moisture Level: ");
  Serial.println(currentLevel);

  digitalWrite(red, currentLevel == 3);
  digitalWrite(yellow, currentLevel == 2);
  digitalWrite(green, currentLevel == 1);

  digitalWrite(pump_in, currentLevel >= 2);
  digitalWrite(pump_out, currentLevel == 3);
}
