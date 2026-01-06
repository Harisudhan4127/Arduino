#define SensorPin A0  // pH meter Analog output to Arduino Analog Input

float calibration_value = 21.34 + 0.7;
unsigned long int avgValue;   // Store the average value
int buf[10], temp;

void setup() {
  pinMode(SensorPin, INPUT);
  Serial.begin(9600);

  Serial.println("================================");
  Serial.println("      Arduino pH Meter");
  Serial.println("================================");
}

void loop() {

  // Take 10 sensor readings
  for (int i = 0; i < 10; i++) {
    buf[i] = analogRead(SensorPin);
    delay(10);
  }

  // Sort readings (noise reduction)
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buf[i] > buf[j]) {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }

  // Average middle 6 values
  avgValue = 0;
  for (int i = 2; i < 8; i++) {
    avgValue += buf[i];
  }

  // Convert ADC value to pH
  float voltage = (float)avgValue * 5.0 / 1024 / 6;
  float phValue = -5.70 * voltage + calibration_value;

  // Display pH value
  Serial.print("pH Value: ");
  Serial.print(phValue, 2);  // 2 decimal places
  Serial.print("  |  ");

  // Display pH condition
  if (phValue < 4)
    Serial.println("Very Acidic");
  else if (phValue < 5)
    Serial.println("Acidic");
  else if (phValue < 7)
    Serial.println("Slightly Acidic");
  else if (phValue < 8)
    Serial.println("Neutral");
  else if (phValue < 10)
    Serial.println("Slightly Alkaline");
  else if (phValue < 11)
    Serial.println("Alkaline");
  else
    Serial.println("Very Alkaline");

  delay(1000);
}
