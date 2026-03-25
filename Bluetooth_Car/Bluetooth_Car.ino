#include <Servo.h>
Servo myservo;
int pos = 30;

char t;

void setup() {
  pinMode(13, OUTPUT);  //left motors  forward
  pinMode(12, OUTPUT);  //left motors reverse
  pinMode(11, OUTPUT);  //right  motors forward
  pinMode(10, OUTPUT);  //right motors reverse
  myservo.attach(9);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    t = Serial.read();
    Serial.println(t);
  }

  if (t == 'F') {  //move  forward(all motors rotate in forward direction)
    digitalWrite(13, HIGH);
    digitalWrite(11, HIGH);
  }

  else if (t == 'B') {  //move reverse (all  motors rotate in reverse direction)
    digitalWrite(12, HIGH);
    digitalWrite(10, HIGH);
  }

  else if (t == 'L') {  //turn right (left side motors rotate in forward direction,  right side motors doesn't rotate)
    digitalWrite(11, HIGH);
  }

  else if (t == 'R') {  //turn left (right side motors rotate in forward direction, left  side motors doesn't rotate)
    digitalWrite(13, HIGH);
  }

  else if (t == 'W') {  //turn led on or off)
    digitalWrite(13, LOW);
    digitalWrite(12, LOW);
    digitalWrite(11, LOW);
    digitalWrite(10, LOW);
    for (pos = 30; pos <= 150; pos += 1) {  // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos);  // tell servo to go to position in variable 'pos'
      delay(15);           // waits 15 ms for the servo to reach the position
    }
    for (pos = 150; pos >= 30; pos -= 1) {  // goes from 180 degrees to 0 degrees
      myservo.write(pos);                   // tell servo to go to position in variable 'pos'
      delay(15);                            // waits 15 ms for the servo to reach the position
    }
  }
  // else if (t == 'w') {
  //   digitalWrite(9, LOW);
  // }
  else if (t == 'S') {  //STOP (all motors stop)
    digitalWrite(13, LOW);
    digitalWrite(12, LOW);
    digitalWrite(11, LOW);
    digitalWrite(10, LOW);
  }
  delay(100);
}