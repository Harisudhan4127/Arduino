// Made by Harisudhan
// Visit www.youtube.com/@harisudhantechnology for more projects

#include <LowPower.h>  // Install this library via Arduino Library Manager

const int OP_Pin = 12;  // Output pin (buzzer or LED)
const int sensorPin = A0;

void setup() {
  pinMode(OP_Pin, OUTPUT);
  digitalWrite(OP_Pin, LOW);
  Serial.begin(9600);
  Serial.println(F("🌧 Rain Sensor with Low Power Mode Initialized"));
  Serial.println(F("---------------------------------------------"));
}

void loop() {
  Rain_Sensor();
  delay(250);
}

void Rain_Sensor() {
  int level = analogRead(sensorPin);
  Serial.print(F("Analog value: "));
  Serial.println(level);

  if (level > 255) {
    Serial.println(F("💧 Rain detected! Making alert beeps..."));
    for (int i = 0; i < 3; i++) {
      Serial.print(F("Beep sound "));
      Serial.println(i + 1);
      digitalWrite(OP_Pin, HIGH);
      delay(100);
      digitalWrite(OP_Pin, LOW);
      delay(100);
    }
  } else {
    Serial.println(F("☀️ No rain detected. Entering sleep mode..."));
    digitalWrite(OP_Pin, LOW);

    // Sleep for 8 seconds (minimum power usage)
    // ADC and BOD off for maximum power saving
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }

  Serial.println();
  delay(50);
}
