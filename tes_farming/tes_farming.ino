// Made by Harisudhan
// www.youtube.com/@harisudhantechnology - visit for more projects

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHTTYPE DHT11        // Type of DHT sensor
#define alarm 4              // Alarm output pin
#define ledPin 11            // LED indicator
#define DHTPIN 9             // DHT11 data pin
#define relay1 13            // Relay for light
#define pumpRelay 12         // Relay for water pump

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);
String voice;

void setup() {
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 1);
  lcd.print("Smart Farming");
  delay(2000);
  lcd.clear();

  pinMode(relay1, OUTPUT);
  pinMode(alarm, OUTPUT);
  pinMode(DHTPIN, INPUT);
  pinMode(pumpRelay, OUTPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(relay1, HIGH);   // Light OFF
  digitalWrite(pumpRelay, HIGH); // Pump OFF
  digitalWrite(ledPin, HIGH);
  digitalWrite(alarm, LOW);
  
  Serial.begin(9600);
}

void loop() {
  Moisture();
  DTH();

  while (Serial.available()) {
    delay(100);
    char c = Serial.read();
    if (c == '#') {
      break;
    }
    voice += c;
  }

  if (voice.length() > 0) {
    if (voice == "turn on light") {
      digitalWrite(relay1, LOW);     // Light ON
      Serial.print("LIGHT ON\n");
    } else if (voice == "turn off light") {
      digitalWrite(relay1, HIGH);    // Light OFF
      Serial.print("LIGHT OFF\n");
    }
    voice = "";
  }
}

void DTH() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("\nHumidity: ");
  Serial.print(h);
  Serial.println(" %");

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C");

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("H: ");
  lcd.print(h);
  lcd.print("% T: ");
  lcd.print(t);
  lcd.print("C");

  if (t < 24.0 || t > 35.5) {
    for (int i = 0; i < 3; i++) {
      Serial.println("Weather is bad");
      digitalWrite(alarm, HIGH);
      delay(250);
      digitalWrite(alarm, LOW);
    }
  }

  delay(1000);
}

void Moisture() {
  int level = analogRead(A1);

  lcd.clear();
  lcd.setCursor(0, 0);

  if (level < 175) {
    lcd.print("Moisture: LEVEL 5");
    Serial.println("Moisture: LEVEL 5");
    digitalWrite(pumpRelay, HIGH);
  } else if (level < 350) {
    lcd.print("Moisture: LEVEL 4");
    Serial.println("Moisture: LEVEL 4");
    digitalWrite(pumpRelay, LOW);
  } else if (level < 525) {
    lcd.print("Moisture: LEVEL 3");
    Serial.println("Moisture: LEVEL 3");
    digitalWrite(pumpRelay, LOW);
  } else if (level < 700) {
    lcd.print("Moisture: LEVEL 2");
    Serial.println("Moisture: LEVEL 2");
    digitalWrite(pumpRelay, HIGH);
  } else if (level < 876) {
    lcd.print("Moisture: LEVEL 1");
    Serial.println("Moisture: LEVEL 1");
    digitalWrite(pumpRelay, HIGH);
  }

  delay(500);
}
