int sensorPin = A0;
int sensor_1 = A1;
int ledPin = 13;
int x = 0;
int y = 0;

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // read the value from the sensor:
  x = analogRead(sensorPin);
  y = analogRead(sensor_1);
  Serial.println(x);
  delay(500);
  Serial.println(y);
}
