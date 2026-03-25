#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MLX90614.h>
#include <Fonts/TomThumb.h>

// ---------------- PIN DEFINITIONS ----------------
#define MQ3_AOUT A0
#define BUZZER_PIN 7
#define RED_LED 13
#define YELLOW_LED 12
#define GREEN_LED 11

// ---------------- OLED SETTINGS ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- SENSOR ----------------
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// ---------------- AI PARAMETERS ----------------
#define SAMPLE_SIZE 10

// Temperature levels (×10 to avoid float comparison)
#define TEMP_MILD      375   // 37.5°C
#define TEMP_HIGH      380   // 38.0°C
#define TEMP_CRITICAL  395   // 39.5°C

// Alcohol thresholds
#define ALC_MED   120
#define ALC_HIGH  150

int tempBuffer[SAMPLE_SIZE] = {0};
int alcoholBuffer[SAMPLE_SIZE] = {0};

long tempSum = 0;
long alcoholSum = 0;

int indexAI = 0;

// Timing control
unsigned long previousMillis = 0;
const long interval = 500;  // update every 500ms

// ------------------------------------------------

void setup() {
  Serial.begin(9600);
  mlx.begin();

  pinMode(MQ3_AOUT, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED init failed"));
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 25);
  display.println(F("Initializing..."));
  display.display();
  delay(1500);
}

// ------------------------------------------------

void loop() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis < interval) return;
  previousMillis = currentMillis;

  // -------- SENSOR READINGS --------
  float temperature = mlx.readObjectTempC();
  int alcoholValue = analogRead(MQ3_AOUT);

  int tempInt = temperature * 10;  // convert to integer ×10

  // -------- MOVING AVERAGE FILTER --------
  tempSum -= tempBuffer[indexAI];
  alcoholSum -= alcoholBuffer[indexAI];

  tempBuffer[indexAI] = tempInt;
  alcoholBuffer[indexAI] = alcoholValue;

  tempSum += tempInt;
  alcoholSum += alcoholValue;

  indexAI++;
  if (indexAI >= SAMPLE_SIZE) indexAI = 0;

  int avgTemp = tempSum / SAMPLE_SIZE;
  int avgAlcohol = alcoholSum / SAMPLE_SIZE;

  // -------- AWTRA AI RISK SCORING --------
  int riskScore = 0;
  const char* tempLevel;

  // Temperature Classification
  if (avgTemp >= TEMP_CRITICAL) {
    riskScore += 60;
    tempLevel = "Critical";
  }
  else if (avgTemp >= TEMP_HIGH) {
    riskScore += 50;
    tempLevel = "High Fever";
  }
  else if (avgTemp >= TEMP_MILD) {
    riskScore += 30;
    tempLevel = "Mild Fever";
  }
  else {
    tempLevel = "Normal";
  }

  // Alcohol Classification
  if (avgAlcohol > ALC_HIGH) {
    riskScore += 50;
  }
  else if (avgAlcohol > ALC_MED) {
    riskScore += 30;
  }

  const char* statusMsg;

  if (riskScore >= 80) {
    statusMsg = "HIGH RISK";
    tone(BUZZER_PIN, 1200);
    digitalWrite(RED_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);

  } else if (riskScore >= 40) {
    statusMsg = "MEDIUM RISK";
    tone(BUZZER_PIN, 800);
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);

  } else {
    statusMsg = "SAFE";
    noTone(BUZZER_PIN);
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
  }

  // -------- SERIAL OUTPUT --------
  Serial.print(F("Temp: "));
  Serial.print(avgTemp / 10.0);
  Serial.print(F(" C | Alcohol: "));
  Serial.print(avgAlcohol);
  Serial.print(F(" | Risk Score: "));
  Serial.println(riskScore);

  // -------- OLED DISPLAY --------
  display.clearDisplay();
  display.setFont(&TomThumb);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(5, 14);
  display.println(F("Wristband Monitor"));
  display.drawLine(0, 20, 127, 20, SSD1306_WHITE);

  display.setCursor(0, 30);
  display.print(F("Temp: "));
  display.print(avgTemp / 10.0);
  display.print(F("C "));
  display.println(tempLevel);

  display.setCursor(0, 42);
  display.print(F("Alcohol: "));
  display.println(avgAlcohol);

  display.setCursor(0, 54);
  display.print(F("Status: "));
  display.println(statusMsg);

  display.display();
}
