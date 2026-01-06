// Made by Harisudhan
//www.youtube.com/@harisudhantechnology viste for more projects

#include <Wire.h>
const int red = 13;
const int yellow = 12;
const int green = 11;
const int buzzer = 10;
const int pump = 9;

const int sensor = 5;  // signal pin of sensor to digital pin 5.
int state = LOW;



void setup() {
  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);  // Yellow led
  pinMode(green, OUTPUT);   // Green led
  pinMode(pump, OUTPUT);    // Pump
  pinMode(buzzer, OUTPUT);  // buzzer
  pinMode(sensor, INPUT);   // PIR motion sensor is determined is an input here.
  Serial.begin(9600);
}

void loop() {
  Moisture();
  Motion();
}

void Motion(){ // code that loops forever
 int state = digitalRead(sensor);  // read sensor pin
  if(state){ // if motion detected
    digitalWrite(buzzer,HIGH);  // turn LED on
    Serial.println("HIGH");
  }
  else{  // else, no motion detected
    digitalWrite(buzzer,LOW);  // turn LED off
    Serial.println("LOW");

  }
}

void Moisture() {
  int level = analogRead(A0);  // Updated to use A0 for clarity
  // Serial.println("Analog value:");
  // Serial.println(level);


  if (level < 300) {  //Bad
    Serial.print("Moisture: LEVEL 1");
    digitalWrite(red, HIGH);
    digitalWrite(yellow, LOW);
    digitalWrite(green, LOW);
    digitalWrite(pump, LOW);  // Pump OFF
  } else if (level < 500) {   // GOOD
    Serial.print("Moisture: LEVEL 2");
    digitalWrite(red, LOW);
    digitalWrite(yellow, HIGH);
    digitalWrite(green, LOW);
    digitalWrite(pump, HIGH);  // Pump OFF
  } else if (level > 500) {    // Very Bad
    Serial.print("Moisture: LEVEL 3");
    digitalWrite(red, LOW);
    digitalWrite(yellow, LOW);
    digitalWrite(green, HIGH);
    digitalWrite(pump, HIGH);  // Pump OFF
  }
  Serial.print("\n");
  delay(250);
}
