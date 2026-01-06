// Made by Harisudhan
//www.youtube.com/@harisudhantechnology viste for more projects

const int OP_Pin = 12;

void setup() {

  pinMode(OP_Pin, OUTPUT);
  digitalWrite(OP_Pin, HIGH);
  Serial.begin(9600);
}

void loop() {
  Rain_Sensor();
  delay(250);
}

void Rain_Sensor() {
  int level = analogRead(A0);
  Serial.println("Analog value:");
  Serial.println(level);

  if (level > 255) {
    int count = 0;
    for (int i = 0; i < 3; i++) {
      count = i + 1;
      digitalWrite(OP_Pin, HIGH);
      delay(100);
      digitalWrite(OP_Pin, LOW);
      delay(100);
    }
    Serial.println("Beep sound Count : ");
    Serial.print(count);

  } else {
    digitalWrite(OP_Pin, HIGH);
  }
  Serial.print("\n");
  delay(50);
}
