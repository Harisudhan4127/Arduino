#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MLX90614.h>
#include <Fonts/TomThumb.h>

// Pins
#define MQ3_AOUT A0
#define BUZZER_PIN 7
#define LED_PIN 13

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Sensors
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(9600);
  mlx.begin();

  pinMode(MQ3_AOUT, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // OLED initialize
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED initialization failed"));
    for (;;)
      ;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.println("System Initializing..");
  display.display();
  delay(1500);
}


void loop() {
  int alcoholValue = analogRead(MQ3_AOUT);
  float temperature = mlx.readObjectTempC();

  bool alcoholDetected = (alcoholValue > 150);
  bool abnormalTemp = (temperature >= 38.0); // || temperature <= 33.5

  String statusMsg;

  if (alcoholDetected && abnormalTemp) {
    statusMsg = "Alcohol + Fever!";
    tone(BUZZER_PIN, 1000);
    digitalWrite(LED_PIN, HIGH);
  } else if (alcoholDetected) {
    statusMsg = "Alcohol Detected!";
    tone(BUZZER_PIN, 800);
    digitalWrite(LED_PIN, HIGH);
  } else if (abnormalTemp) {
    statusMsg = "Fever Detected!";
    tone(BUZZER_PIN, 600);
    digitalWrite(LED_PIN, HIGH);
  } else {
    statusMsg = "Normal Condition";
    noTone(BUZZER_PIN);
    digitalWrite(LED_PIN, LOW);
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C  |  Alcohol: ");
  Serial.println(alcoholValue);

  display.clearDisplay();
  display.setFont(&TomThumb);  // Tiny readable font
  display.setTextColor(SSD1306_WHITE);

  // Header
  display.setCursor(10, 14);
  display.println("Alcohol & Temp Monitor");
  display.drawLine(0, 20, 127, 20, SSD1306_WHITE);  // Top divider line

  // Sensor readings
  display.setCursor(0, 30);
  display.print("Temp: ");
  display.print(temperature, 1);
  display.println(" C");

  display.setCursor(0, 42);
  display.print("Alcohol: ");
  display.println(alcoholValue);

  // Status
  display.setCursor(0, 54);
  display.print("Status: ");
  display.println(statusMsg);

  // Bottom border line
  display.drawLine(0, 63, 127, 63, SSD1306_WHITE);  // bottom edge line (y = 63 is the OLED’s last row)

  display.display();


  delay(1000);
}
