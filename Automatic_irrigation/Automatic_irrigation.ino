// Made by Harisudhan
//www.youtube.com/@harisudhantechnology viste for more projects
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 
LiquidCrystal_I2C lcd(0x27,  16, 2);
int outputValue = 0;
int sensorValue = 0;


void setup()
{
  Serial.begin(9600); // Set the serial monitor baudrate to 9600
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3, 1);
  lcd.print("Harisudhan"); // LCD printed name (hari haran)
  pinMode(A1, INPUT);
  pinMode(2, OUTPUT); // Output Relay and Pump
  
}

void loop()
{
  // Variable to store ADC value ( 0 to 1023 )
  int level;
  // analogRead function returns the integer 10 bit integer (0 to 1023)
  level = analogRead(0);
  // Print text in serial monitor
  Serial.println("Analog value:");
  // Print output voltage in serial monitor
  Serial.println(level);
  digitalWrite(9, LOW);
  // output of sensor varies from 0 to 4.28 Volts,
  // It's equivalent ADC value is 0 to 877 ( (4.28/5)*1024 = 877 )

  // Splitting 877 into 5 level => 175, 350, 525, 700, 877
  // Based on the ADC output, LED indicates the level (1 to 5).

  if (level < 175) {
    // LEVEL 5 LED
    lcd.setCursor(5, 0);
    lcd.print("LEVEL 1    ");
    digitalWrite(8, HIGH);
    digitalWrite(13, HIGH);
  }
  else if (level < 350) {
    // LEVEL 4 LED
    lcd.setCursor(5, 0);
    lcd.print("LEVEL 2    ");
    digitalWrite(8, HIGH);
    digitalWrite(13, HIGH);
    digitalWrite(12, HIGH);
  }
  else if (level < 525) {
    // LEVEL 3 LED
    lcd.setCursor(5, 0);
    lcd.print("LEVEL 3    ");
    digitalWrite(8, HIGH);
    digitalWrite(13, HIGH);
    digitalWrite(12, HIGH);
    digitalWrite(11, HIGH);
  }
  else if (level < 700) {
    // LEVEL 2 LED
    lcd.setCursor(5, 0);
    lcd.print("LEVEL 4    ");
    digitalWrite(8, LOW);
    digitalWrite(13, HIGH);
    digitalWrite(12, HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(10, HIGH);
  }
  else if (level < 876) {
    // LEVEL 1 LED
    lcd.setCursor(5, 0);
    lcd.print("LEVEL 5    ");
    digitalWrite(8, LOW);
    digitalWrite(13, HIGH);
    digitalWrite(12, HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(10, HIGH);
    digitalWrite(9, HIGH);
  }
  
  sensorValue = analogRead(A1);
  outputValue = map(sensorValue, 0, 1023, 0, 255);
  delay(100); // Wait for 100 millisecond(s)
}