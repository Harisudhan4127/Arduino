#include <Wire.h>
#include "MAX30105.h"
#include <Adafruit_BMP280.h>

MAX30105 particleSensor;
Adafruit_BMP280 bmp;

// Create second I2C for BMP280
TwoWire I2C_BMP = TwoWire(1);

void setup() {
  Serial.begin(115200);

  // First I2C for MAX30105 (default SDA=21, SCL=22)
  Wire.begin(21, 22);
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30105 not found!");
    while (1);
  }

  // Second I2C for BMP280 (SDA=25, SCL=26)
  I2C_BMP.begin(25, 26, 100000);
  if (!bmp.begin(0x76, &I2C_BMP)) { // use second I2C instance
    Serial.println("BMP280 not found!");
    while (1);
  }

  Serial.println("Sensors ready");
}

void loop() {
  long redValue = particleSensor.getRed();
  float temp = bmp.readTemperature();

  Serial.print("Red: "); Serial.print(redValue);
  Serial.print("  Temp: "); Serial.println(temp);
  delay(1000);
}
