#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

int Piezo = A0;
int ledoutput = 13;
int THRESHOLD = 500;  // Adjust this as needed

void setup() {
  pinMode(ledoutput, OUTPUT);
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Piezo Electric");
  delay(2000);
  lcd.clear();
}

void loop() {
  int sensorValue = analogRead(Piezo);
  float voltage = (sensorValue / 1023.0) * 5.0;           // in Volts
  float current_microamps = (voltage / 1000000.0) * 1e6;  // μA if 1 MΩ resistor

  if (sensorValue >= THRESHOLD) {
    digitalWrite(ledoutput, HIGH);

    Serial.print("Analog Value = ");
    Serial.println(sensorValue);
    Serial.print("Volt = ");
    Serial.print(voltage * 1000);  // Convert to mV
    Serial.println(" mV");

    Serial.print("Current = ");
    Serial.print(current_microamps);
    Serial.println(" uA");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Volt: ");
    lcd.print(voltage * 1000, 1);  // Display in mV
    lcd.print("mV");

    lcd.setCursor(0, 1);
    lcd.print("Curr: ");
    lcd.print(current_microamps, 1);
    lcd.print("uA");

    delay(500);
  } else {
    digitalWrite(ledoutput, LOW);

    Serial.print("Analog Value = ");
    Serial.println(sensorValue);


    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Piezo Electric");
  }

  delay(250);
}
