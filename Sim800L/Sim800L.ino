#include <SoftwareSerial.h>
// Create a SoftwareSerial object to communicate with the SIM800L module
SoftwareSerial mySerial(3, 2);  // SIM800L Tx & Rx connected to Arduino pins #3 & #2


void setup() {
  // Initialize serial communication with Arduino and the Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  // Initialize serial communication with Arduino and the SIM800L module
  mySerial.begin(9600);
  Serial.println("Initializing...");
  delay(1000);
  mySerial.println("AT");  // Handshake test, should return "OK" on success
  updateSerial();
  mySerial.println("ATD+919344040002;");  // Change ZZ with the country code and xxxxxxxxxxx with the phone number to dial
  updateSerial();
  delay(20000);             // Wait for 20 seconds...
  mySerial.println("ATH");  // Hang up the call
  updateSerial();
}

void loop() {
}

void updateSerial() {
  delay(500);
  while (Serial.available()) {
    mySerial.write(Serial.read());  // Forward data from Serial to Software Serial Port
  }
  while (mySerial.available()) {
    Serial.write(mySerial.read());  // Forward data from Software Serial to Serial Port
  }
}