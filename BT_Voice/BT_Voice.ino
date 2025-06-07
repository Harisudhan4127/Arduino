// Made by Harisudhan
//www.youtube.com/@harisudhantechnology viste for more projects

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHTTYPE DHT11  // Type of DHT sensor
#define relay1 13      //LIGHT
#define relay2 12      // FAN
#define DHTPIN 11      // Pin connected to DHT11 data wire

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

String voice;

void setup() {
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 1);
  lcd.print("Smart Home");
  delay(2000);
  lcd.clear();

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(DHTPIN, INPUT);

  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  Serial.begin(9600);
}

void loop() {
  while (Serial.available()) {
    delay(100);
    char c = Serial.read();
    if (c == '#') {
      break;
    }
    voice += c;
  }
  if (voice.length() > 0) {
    if (voice == "turn on light") {  //Voice Command
      digitalWrite(relay1, LOW);     //light ON
      Serial.print("LIGHT ON\n");

    } else if (voice == "turn off light") {  //Voice Command
      digitalWrite(relay1, HIGH);            //light OFF
      Serial.print("LIGHT OFF\n");
    } else if (voice == "turn off fan") {  //Voice Command
      digitalWrite(relay2, LOW);          //Fan ON
      Serial.print("FAN OFF\n");
    } else if (voice == "turn off fan") {  //Voice Command
      digitalWrite(relay2, HIGH);          //Fan OFF
      Serial.print("FAN OFF\n");
    } else {
      DTH();
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

  if (t < 24.00 || t > 35.5) {
    for (int i = 0; i < 3; i++) {
      Serial.println("Weather is bad");
      delay(250);
      Serial.println("Weather is bad");
    }
  }
  delay(2000);
}
