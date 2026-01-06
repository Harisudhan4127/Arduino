// Made by Harisudhan
//www.youtube.com/@harisudhantechnology viste for more projects

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHTTYPE DHT11   // Type of DHT sensor
#define alarm 4        // Alarm weather
const int ldrPin = A0;  //  LDR Sensor light
#define ledPin 11
#define DHTPIN 9   // Pin connected to DHT11 data wire
#define relay1 13  //LIGHT

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);
String voice;
void setup() {
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 1);
  lcd.print("Smart Farming");
  delay(2000);  // Display the text for 2 seconds
  lcd.clear();

  pinMode(relay1, OUTPUT);  //Set relay1 as an output
  pinMode(alarm, OUTPUT);   // Alarm
  pinMode(DHTPIN, INPUT);   // DHTPIN
  pinMode(12, OUTPUT);      // Output Relay and Pump
  pinMode(ledPin, OUTPUT);

  digitalWrite(relay1, HIGH);  //Switch relay1 off
  digitalWrite(12, HIGH);
  digitalWrite(ledPin, HIGH);
  digitalWrite(alarm, LOW);
  Serial.begin(9600);
}

void loop() {
  // ldr_sensor();
  Moisture();
//  DTH();


  while (Serial.available()) {  //Check if there is an available byte to read
    delay(100);                 //Delay added to make thing stable
    char c = Serial.read();     //Conduct a serial read
    if (c == '#') {
      break;  //Exit the loop when the # is detected after the word
    }
    voice += c;  //Shorthand for voice = voice + c
  }
  if (voice.length() > 0)
  ////////////////////////////////////////////////////////////////////
  {
    if (voice == "turn on light") {  //Voice Command
      digitalWrite(relay1, LOW);     //light Relay 01 ON
      Serial.print("LIGHT ON\n");

    } else if (voice == "turn off light") {  //Voice Command
      digitalWrite(relay1, HIGH);            //light Relay 03 OFF
      Serial.print("LIGHT OFF\n");
    }

    voice = "";  //Reset the variable after initiating
  }
}


// void ldr_sensor() {

//   int ldrValue = analogRead(ldrPin);
//   Serial.println(ldrValue);

//   if (ldrValue > 600) {
//     digitalWrite(ledPin, HIGH);
//     Serial.print("HIGH\n");
//   } else {
//     digitalWrite(ledPin, LOW);
//     Serial.print("LOW\n");
//   }

//   delay(500);
// }

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

  // Clear previous sensor readings from the display
  lcd.clear();

  // Display Humidity and Temperature on the same line

  lcd.setCursor(0, 1);  // Start at the Second line
  lcd.print("H: ");
  lcd.print(h);
  lcd.print("% T: ");
  lcd.print(t);
  lcd.print("C");

  // // Display Humidity
  // lcd.setCursor(0, 0);
  // lcd.print("Humidity: ");
  // lcd.print(h);
  // lcd.print(" %");

  // // Display Temperature
  // lcd.setCursor(0, 1);
  // lcd.print("Temp: ");
  // lcd.print(t);
  // lcd.print(" *C");

  // Check for temperature thresholds
  if (t < 24.00 || t > 35.5) {
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
    digitalWrite(12, LOW);
  } else if (level < 525) {
    lcd.setCursor(0, 0);
    lcd.print(" WATER: LEVEL 3");
    Serial.print("Moisture: LEVEL 3");
    digitalWrite(12, LOW);
  } else if (level < 700) {
    lcd.setCursor(0, 0);
    lcd.print(" WATER: LEVEL 2");
    Serial.print("Moisture: LEVEL 2");
    digitalWrite(12, HIGH);
  } else if (level < 876) {
    lcd.setCursor(0, 0);
    lcd.print(" WATER: LEVEL 1");
    Serial.print("Moisture: LEVEL 1");
    digitalWrite(12, HIGH);
  }
Serial.print("\n");
  delay(1000);
}
