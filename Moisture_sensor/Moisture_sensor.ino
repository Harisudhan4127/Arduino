// Made by Harisudhan
//www.youtube.com/@harisudhantechnology viste for more projects

#include <Wire.h>
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 1);
  lcd.print("Smart Farming");
  delay(2000);  // Display the text for 2 seconds
  lcd.clear();
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
  Serial.begin(9600);
}

void loop() {
  Moisture();
}



void Moisture() {
  int level = analogRead(A1);  // Updated to use A0 for clarity
//  Serial.println("Analog value:");
//  Serial.println(level);

  lcd.clear();  // Clear display before printing new moisture level

  if (level < 175) {
    lcd.setCursor(0, 0);
    lcd.print(" WATER: LEVEL 5");
    Serial.print("Moisture: LEVEL 5");
    digitalWrite(12, HIGH);

  } else if (level < 350) {
    lcd.setCursor(0, 0);
    lcd.print(" WATER: LEVEL 4");
    Serial.print("Moisture: LEVEL 4");
    digitalWrite(12, HIGH);
  } else if (level < 525) {
    lcd.setCursor(0, 0);
    lcd.print(" WATER: LEVEL 3");
    Serial.print("Moisture: LEVEL 3");
    digitalWrite(12, HIGH);
  } else if (level < 700) {
    lcd.setCursor(0, 0);
    lcd.print(" WATER: LEVEL 2");
    Serial.print("Moisture: LEVEL 2");
    digitalWrite(12, LOW);
  } else if (level < 876) {
    lcd.setCursor(0, 0);
    lcd.print(" WATER: LEVEL 1");
    Serial.print("Moisture: LEVEL 1");
    digitalWrite(12, LOW);
  }
Serial.print("\n");
  delay(1000);
}
