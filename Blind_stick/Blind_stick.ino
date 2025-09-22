#define trigPin 9
#define echoPin 10
#define motor 13
#define buzzer 11

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(motor, OUTPUT);
  pinMode(buzzer, OUTPUT);
  Serial.begin(115200); 
}

void loop() {
  long duration, distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < 35) {
    digitalWrite(motor, HIGH);
    digitalWrite(buzzer, HIGH);
  } else {
    digitalWrite(motor, LOW);
    digitalWrite(buzzer, LOW);
  }

  delay(500);
}