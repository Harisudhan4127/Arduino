#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

SoftwareSerial btSerial(10, 11); // HC-05 TX, RX

LiquidCrystal_I2C lcd(0x27, 20, 4);

const int input = A0;

int X, Y;
float TIME = 0;
float FREQUENCY = 0;
float WATER = 0;
float LS = 0;
float TOTAL = 0;

float waterSum = 0;
int readingCount = 0;

unsigned long lastMinuteTime = 0;
unsigned long lastSaveTime = 0;
unsigned long lastCurrentTime = 0;

const unsigned long saveInterval = 10000;
const unsigned long minuteInterval = 60000;
const unsigned long currentInterval = 3000; // Every 3 seconds send "current flow"

String voice = "";
String data = "";

void EEPROMWriteFloat(int address, float value) {
  byte *data = (byte *)(void *)&value;
  for (int i = 0; i < sizeof(float); i++) {
    EEPROM.update(address + i, data[i]);
  }
}

float EEPROMReadFloat(int address) {
  float value = 0.0;
  byte *data = (byte *)(void *)&value;
  for (int i = 0; i < sizeof(float); i++) {
    data[i] = EEPROM.read(address + i);
  }
  return value;
}

void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Water Flow Meter");
  lcd.setCursor(0, 1);
  lcd.print("Loading Total...");
  delay(2000);

  pinMode(input, INPUT);
  TOTAL = EEPROMReadFloat(0);

  lastMinuteTime = millis();
  lastCurrentTime = millis();
}

void loop() {
  X = pulseIn(input, HIGH);
  Y = pulseIn(input, LOW);
  TIME = X + Y;

  if (TIME > 0) {
    FREQUENCY = 1000000.0 / TIME;
    WATER = FREQUENCY / 7.5;
    LS = WATER / 60.0;
    TOTAL += LS;

    waterSum += WATER;
    readingCount++;
  }

  // Save to EEPROM every 10s
  if (millis() - lastSaveTime >= saveInterval) {
    EEPROMWriteFloat(0, TOTAL);
    lastSaveTime = millis();
  }

  // Send CURRENT water flow every 3s
  if (millis() - lastCurrentTime >= currentInterval) {
    lcd.setCursor(0, 2);
    lcd.print("Current: ");
    lcd.print(WATER, 2);
    lcd.print(" L/M  ");

    btSerial.print("Current: ");
    btSerial.print(WATER, 2);
    btSerial.println(" L/M");

    lastCurrentTime = millis();
  }

  // Update display + Bluetooth every 1 minute
  if (millis() - lastMinuteTime >= minuteInterval) {
    float avgWater = (readingCount > 0) ? (waterSum / readingCount) : 0;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AVG: ");
    lcd.print(avgWater, 2);
    lcd.print(" L/M");

    lcd.setCursor(0, 1);
    lcd.print("TOTAL: ");
    lcd.print(TOTAL, 2);
    lcd.print(" L");

    lcd.setCursor(0, 2);
    lcd.print("Current: ");
    lcd.print(WATER, 2);
    lcd.print(" L/M");

    lcd.setCursor(0, 3);
    lcd.print("Updated @ 1 min");

    // Send Bluetooth full data
    data = "AVG: " + String(avgWater, 2) + " L/M | TOTAL: " + String(TOTAL, 2) + " L";
    btSerial.println(data);
    Serial.println(data);

    waterSum = 0;
    readingCount = 0;
    lastMinuteTime = millis();
  }

  BT_control();  // Optional voice control (if used)
}

void BT_control() {
  while (btSerial.available()) {
    delay(20);
    char c = btSerial.read();
    if (c == '#') break;
    voice += c;
  }

  if (voice.length() > 0) {
    if (voice == "turn on light") {
      btSerial.println("Light ON command received");
    } else {
      btSerial.println("Unknown command");
    }
    voice = "";
  }
}
