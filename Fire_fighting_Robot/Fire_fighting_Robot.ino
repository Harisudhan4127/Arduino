#include <Servo.h>

Servo myservo;
int pos = 0;
boolean fire = false;

#define Left_S 7     // left sensor
#define Right_S 6   // right sensor
#define Forward_S 5  //forward sensor

#define LM1 13  // left motor
#define LM2 12  // left motor
#define RM1 11  // right motor
#define RM2 10  // right motor
#define pump 8
void setup() {

  pinMode(Left_S, INPUT);
  pinMode(Right_S, INPUT);
  pinMode(Forward_S, INPUT);

  pinMode(LM1, OUTPUT);
  pinMode(LM2, OUTPUT);
  pinMode(RM1, OUTPUT);
  pinMode(RM2, OUTPUT);
  pinMode(pump, OUTPUT);

  myservo.attach(9);

  myservo.write(90);
}
void put_off_fire() {
  delay(500);
  digitalWrite(LM1, HIGH);
  digitalWrite(LM2, HIGH);
  digitalWrite(RM1, HIGH);
  digitalWrite(RM2, HIGH);
  digitalWrite(pump, HIGH);
  delay(500);
  for (pos = 50; pos <= 130; pos += 1) {
    myservo.write(pos);
    delay(10);
  }
  for (pos = 130; pos >= 50; pos -= 1) {
    myservo.write(pos);
    delay(10);
  }
  digitalWrite(pump, LOW);
  myservo.write(90);
  fire = false;
}
void loop() {
  myservo.write(90);                                                                         //Sweep_Servo();
  if (digitalRead(Left_S) == 1 && digitalRead(Right_S) == 1 && digitalRead(Forward_S) == 1)  //If Fire not detected all sensors are zero
  {
    digitalWrite(LM1, HIGH);
    digitalWrite(LM2, HIGH);
    digitalWrite(RM1, HIGH);
    digitalWrite(RM2, HIGH);
  } else if (digitalRead(Forward_S) == 0)  //If Fire is straight ahead
  {
    //Move the robot forward
    digitalWrite(LM1, HIGH);
    digitalWrite(LM2, LOW);
    digitalWrite(RM1, HIGH);
    digitalWrite(RM2, LOW);
    fire = true;
  } else if (digitalRead(Left_S) == 0)  //If Fire is to the left
  {
    //Move the robot left
    digitalWrite(LM1, HIGH);
    digitalWrite(LM2, LOW);
    digitalWrite(RM1, HIGH);
    digitalWrite(RM2, HIGH);
  } else if (digitalRead(Right_S) == 0)  //If Fire is to the right
  {
    //Move the robot right
    digitalWrite(LM1, HIGH);
    digitalWrite(LM2, HIGH);
    digitalWrite(RM1, HIGH);
    digitalWrite(RM2, LOW);
  }
  delay(300);  //Slow down the speed of robot
  while (fire == true) {
    put_off_fire();
  }
}
