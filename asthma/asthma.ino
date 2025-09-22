const int MQ135_SENSOR_PIN = 34;
int led = 12;
int sensitivity = 200;

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
}

void loop() {
  int sensor_value = analogRead(MQ135_SENSOR_PIN);
  int air_quality = sensor_value * sensitivity / 4095; // ESP32 ADC is 12-bit (0–4095)

  String air_quality_label = air_quality_data(sensor_value);

  Serial.print("Raw Value: ");
  Serial.print(sensor_value);
  Serial.print("  |  Air Quality: ");
  Serial.println(air_quality_label);

  if (sensor_value < 50) {
    Serial.println("Level 1");
    digitalWrite(led, LOW);
  }
  else if (sensor_value < 100 ) {
    Serial.println("Level 2");
    digitalWrite(led, LOW);
  }
  else if (sensor_value < 150) {
    Serial.println("Level 3");
  }
  else if (sensor_value < 200) {
    Serial.println("Level 4");
    digitalWrite(led, HIGH);
  }
  else {
    Serial.println("Level 5");
    digitalWrite(led, HIGH);
  }
  delay(500);
}

String air_quality_data(int sensor_value) {
  if (sensor_value < 50) {
    return "Excellent";
  } else if (sensor_value < 100) {
    return "Good";
  } else if (sensor_value < 150) {
    return "Moderate";
  } else if (sensor_value < 200) {
    return "Poor";
  } else {
    return "Dangerous";
  }
}
