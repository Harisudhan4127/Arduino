const int sensorPin = A1;
const int output1 = 2;
const int output2 = 3;
const int output3 = 4;
const int output4 = 5;

const int threshold = 250;

void setup() {

  pinMode(sensorPin, INPUT);

  pinMode(output1, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(output3, OUTPUT);
  pinMode(output4, OUTPUT);

  Serial.begin(115200);
}

void loop() {
  int sensorValue = analogRead(sensorPin);
  Serial.println(sensorValue);  

  if (sensorValue < threshold) {
    digitalWrite(output1, LOW);
    digitalWrite(output2, LOW);
    digitalWrite(output3, HIGH);
    digitalWrite(output4, HIGH);
  } else {
    digitalWrite(output1, HIGH);
    digitalWrite(output2, HIGH);
    digitalWrite(output3, HIGH);
    digitalWrite(output4, HIGH);
  }

  // delay(100);
}
