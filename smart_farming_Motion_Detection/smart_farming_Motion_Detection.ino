// Made by Harisudhan
// www.youtube.com/@harisudhantechnology

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// LEDs
const int green = 13;
const int yellow = 12;
const int red = 11;

// Buzzer
const int buzzer = 10;

// Relay pins
const int pump_in = 7;
const int pump_out = 4;

// Ultrasonic
const int trig_pin = 6;
const int echo_pin = 5;

// Moisture sensor
const int moisture_pin = A0;

bool objectDetected = false;

float timing = 0;
float distance = 0;

void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 1);
  lcd.print("Smart Farming");
  delay(100);
  lcd.clear();

  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(pump_in, OUTPUT);
  pinMode(pump_out, OUTPUT);
  pinMode(trig_pin, OUTPUT);
  pinMode(echo_pin, INPUT);

  digitalWrite(pump_in, LOW);
  digitalWrite(pump_out, LOW);
  Serial.begin(9600);
}

void loop()
{
  handleUltrasonic();
  Moisture();
}

//////////////////////////////////////////////////
// ULTRASONIC OBJECT DETECTION
//////////////////////////////////////////////////

void handleUltrasonic()
{
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);

  timing = pulseIn(echo_pin, HIGH);
  distance = (timing * 0.034) / 2;
  if (distance > 2 && distance < 400)
  {
    //    Serial.print("Distance: ");
    //    Serial.print(distance);
    //    Serial.println(" cm");

    if (distance <= 10 && !objectDetected)
    {
      Serial.println("Object detected!");
      digitalWrite(buzzer, HIGH); // Buzzer ON
      objectDetected = true;
    }
    else
    {
      digitalWrite(buzzer, LOW);    // Buzzer OFF
    }

    if (distance > 15)
    {
      objectDetected = false;
    }
  }

  delay(50);
}
//////////////////////////////////////////////////
// MOISTURE SENSOR + LCD + RELAY CONTROL
//////////////////////////////////////////////////

void Moisture()
{
  int level = analogRead(moisture_pin);

  lcd.clear();

  Serial.println(level);
  if (level < 175)
  {
    lcd.setCursor(0, 0);
    lcd.print("WATER: LEVEL 5");
    Serial.println("Moisture: LEVEL 5");
    digitalWrite(red, HIGH);
    digitalWrite(yellow, LOW);
    digitalWrite(green, LOW);
    digitalWrite(pump_in, LOW);
    digitalWrite(pump_out, HIGH);
  }

  else if (level < 300)
  {
    lcd.setCursor(0, 0);
    lcd.print("WATER: LEVEL 4");
    Serial.println("Moisture: LEVEL 4");
    digitalWrite(red, HIGH);
    digitalWrite(yellow, LOW);
    digitalWrite(green, LOW);
    digitalWrite(pump_in, LOW);
    digitalWrite(pump_out, HIGH);
  }

  else if (level < 500)
  {
    lcd.setCursor(0, 0);
    lcd.print("WATER: LEVEL 3");
    Serial.println("Moisture: LEVEL 3");
    digitalWrite(red, LOW);
    digitalWrite(yellow, HIGH);
    digitalWrite(green, LOW);
    digitalWrite(pump_in, HIGH);
    digitalWrite(pump_out, HIGH);
  }

  else if (level < 600)
  {
    lcd.setCursor(0, 0);
    lcd.print("WATER: LEVEL 2");
    Serial.println("Moisture: LEVEL 2");
    digitalWrite(red, LOW);
    digitalWrite(yellow, HIGH);
    digitalWrite(green, LOW);
    digitalWrite(pump_in, HIGH);
    digitalWrite(pump_out, HIGH);
  }

  else if (level > 650)
  {
    lcd.setCursor(0, 0);
    lcd.print("WATER: LEVEL 1");
    Serial.println("Moisture: LEVEL 1");
    digitalWrite(red, LOW);
    digitalWrite(yellow, LOW);
    digitalWrite(green, HIGH);
    digitalWrite(pump_in, HIGH);
    digitalWrite(pump_out, LOW);
  }

  delay(250);
}
