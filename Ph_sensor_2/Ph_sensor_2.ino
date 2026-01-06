#define SensorPin A0

float slope = -5.7;
float intercept = 27.8;   // calibrated for YOUR module

int buf[10], temp;
unsigned long avgValue;

void setup() {
  Serial.begin(9600);
  Serial.println("pH Meter (Software Calibrated)");
}

void loop() {

  for (int i = 0; i < 10; i++) {
    buf[i] = analogRead(SensorPin);
    delay(10);
  }

  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buf[i] > buf[j]) {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }

  avgValue = 0;
  for (int i = 2; i < 8; i++)
    avgValue += buf[i];

  float voltage = avgValue * 5.0 / (1024.0 * 6);
  float pH = slope * voltage + intercept;

  Serial.print("Voltage: ");
  Serial.print(voltage, 3);
  Serial.print(" V | pH: ");
  Serial.println(pH, 2);

  delay(1000);
}
