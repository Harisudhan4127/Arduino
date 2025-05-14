#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTTYPE DHT11
#define DHTPIN 9
DHT dht(DHTPIN, DHTTYPE);

// DECLARE FUNCTION PROTOTYPE
void showCenteredText(String text, uint8_t textSize, int yOffset = 0);

void setup() {
  Serial.begin(9600);
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }

  display.clearDisplay();
  showCenteredText("PIC Prototype 2", 1);
  delay(2000);
  display.clearDisplay();
}

void loop() {
  DTH();
  delay(2000);
}

void showCenteredText(String text, uint8_t textSize, int yOffset) {
  display.clearDisplay();
  display.setTextSize(textSize);
  display.setTextColor(WHITE);

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  int x = (SCREEN_WIDTH - w) / 2;
  int y = (SCREEN_HEIGHT - h) / 2 + yOffset;

  display.setCursor(x, y);
  display.println(text);
  display.display();
}

void DTH() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    showCenteredText("Sensor Error", 1);
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C");

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  String humText = "Humidity: " + String(h) + " %";
  String tempText = "Temp: " + String(t) + " *C";

  int16_t x1, y1;
  uint16_t w1, h1, w2, h2;

  display.getTextBounds(humText, 0, 0, &x1, &y1, &w1, &h1);
  display.getTextBounds(tempText, 0, 0, &x1, &y1, &w2, &h2);

  int spacing = 2;
  int totalHeight = h1 + h2 + spacing;
  int yStart = (SCREEN_HEIGHT - totalHeight) / 2;

  int x1_center = (SCREEN_WIDTH - w1) / 2;
  display.setCursor(x1_center, yStart);
  display.println(humText);

  int x2_center = (SCREEN_WIDTH - w2) / 2;
  display.setCursor(x2_center, yStart + h1 + spacing);
  display.println(tempText);

  display.display();
}
