// Made by Harisudhan
// Visit www.youtube.com/@harisudhantechnology for more projects

#include <LowPower.h>  // Install from Arduino Library Manager

const int buzzerPin = 12;      // Buzzer output
const int sensorPin = A0;      // Rain sensor analog input
const int redLED = 9;          // Heavy rain indicator
const int yellowLED = 8;       // Light/Moderate rain
const int greenLED = 7;        // Dry condition

// Thresholds for moisture levels
const int thresholds[] = {300, 600, 850};
const char *conditions[] = {"☀️ Dry", "🌦 Light Moisture", "🌧 Moderate Rain", "⛈ Heavy Rain"};
const uint8_t beepCounts[] = {0, 1, 2, 3};

unsigned long previousMillis = 0;
const unsigned long interval = 500;  // Read interval in ms
int lastCondition = -1;

void setup() {
  pinMode(buzzerPin, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  digitalWrite(buzzerPin, LOW);
  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, LOW);

  Serial.begin(9600);
  Serial.println(F("🌧 Smart Rain Sensor Initialized"));
  Serial.println(F("-----------------------------------"));
}

void loop() {
  unsigned long currentMillis = millis();

  // Read every 500 ms
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    checkRainLevel();
  }

  // If dry for a long time → sleep for 8 seconds to save battery
  if (lastCondition == 0) {
    Serial.println(F("💤 Dry condition detected — entering sleep mode..."));
    delay(50); // short delay before sleep
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
}

void checkRainLevel() {
  int level = analogRead(sensorPin);
  Serial.print(F("Analog Value: "));
  Serial.println(level);

  // Determine rain level
  uint8_t conditionIndex = 0;
  while (conditionIndex < 3 && level >= thresholds[conditionIndex]) {
    conditionIndex++;
  }

  // Only update if condition changed
  if (conditionIndex != lastCondition) {
    lastCondition = conditionIndex;
    Serial.print(F("Condition: "));
    Serial.println(conditions[conditionIndex]);
    Serial.println(F("-----------------------------------"));

    // LED Indicator logic
    updateLEDs(conditionIndex);

    // Beep for alert (only if raining)
    if (beepCounts[conditionIndex] > 0) {
      Beep(beepCounts[conditionIndex], 120);
    } else {
      digitalWrite(buzzerPin, LOW);
    }
  }
}

void updateLEDs(uint8_t index) {
  // Turn off all LEDs first
  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, LOW);

  switch (index) {
    case 0:
      digitalWrite(greenLED, HIGH);  // Dry
      break;
    case 1:
    case 2:
      digitalWrite(yellowLED, HIGH); // Light/Moderate rain
      break;
    case 3:
      digitalWrite(redLED, HIGH);    // Heavy rain
      break;
  }
}

void Beep(uint8_t times, uint16_t duration) {
  for (uint8_t i = 0; i < times; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(duration);
    digitalWrite(buzzerPin, LOW);
    delay(duration);
  }
}
