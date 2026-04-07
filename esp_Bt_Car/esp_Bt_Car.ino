#include "BluetoothSerial.h"

// Bluetooth
BluetoothSerial SerialBT;
String device_name = "ESP32-Car";

// ───── Motor Pins ─────
int PIN_IN1 = 27;
int PIN_IN2 = 26;
int PIN_IN3 = 25;
int PIN_IN4 = 33;

// ───── Headlamp ─────
int HIGH_BEAM = 12;
int LOW_BEAM  = 14;

// ───── LDR Sensor ─────
int LDR_PIN = 34;

// ───── Control Variables ─────
char cmd;
bool autoMode = true;

bool highState = false;
bool lowState  = false;

// ───── SETUP ─────
void setup() {
  Serial.begin(9600);
  SerialBT.begin(device_name);

  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);

  pinMode(HIGH_BEAM, OUTPUT);
  pinMode(LOW_BEAM, OUTPUT);

  pinMode(LDR_PIN, INPUT);
}

// ───── AUTO HEADLAMP ─────
void autoHeadlamp() {
  int light = digitalRead(LDR_PIN);

  if (light == HIGH) {
    // Night → HIGH beam
    digitalWrite(HIGH_BEAM, HIGH);
    digitalWrite(LOW_BEAM, LOW);
  } else {
    // Day → OFF
    digitalWrite(HIGH_BEAM, LOW);
    digitalWrite(LOW_BEAM, HIGH);
  }
}

// ───── MANUAL HEADLAMP ─────
void manualHeadlamp() {
  digitalWrite(HIGH_BEAM, highState);
  digitalWrite(LOW_BEAM, lowState);
}

// ───── LOOP ─────
void loop() {

  // Read Bluetooth
  if (SerialBT.available()) {
    cmd = SerialBT.read();
    Serial.println(cmd);
  }

  // ───── MOVEMENT ─────
  if (cmd == 'F') {
    digitalWrite(PIN_IN1, HIGH);
    digitalWrite(PIN_IN2, LOW);
    digitalWrite(PIN_IN3, HIGH);
    digitalWrite(PIN_IN4, LOW);
  }
  else if (cmd == 'B') {
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, HIGH);
    digitalWrite(PIN_IN3, LOW);
    digitalWrite(PIN_IN4, HIGH);
  }
  else if (cmd == 'L') {
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, HIGH);
    digitalWrite(PIN_IN3, HIGH);
    digitalWrite(PIN_IN4, LOW);
  }
  else if (cmd == 'R') {
    digitalWrite(PIN_IN1, HIGH);
    digitalWrite(PIN_IN2, LOW);
    digitalWrite(PIN_IN3, LOW);
    digitalWrite(PIN_IN4, HIGH);
  }
  else if (cmd == 'S') {
    stopMotor();
  }

  // ───── MODE CONTROL ─────
  else if (cmd == 'A') {
    autoMode = true;   // AUTO mode
  }
  else if (cmd == 'M') {
    autoMode = false;  // MANUAL mode
  }

  // ───── MANUAL LIGHT CONTROL ─────
  else if (cmd == 'H') {
    highState = true;   // HIGH beam ON
  }
  else if (cmd == 'h') {
    highState = false;  // HIGH beam OFF
  }
  else if (cmd == 'N') {
    lowState = true;    // LOW beam ON
  }
  else if (cmd == 'n') {
    lowState = false;   // LOW beam OFF
  }

  // ───── APPLY MODE ─────
  if (autoMode) {
    autoHeadlamp();
  } else {
    manualHeadlamp();
  }

  delay(50);
}

// ───── STOP FUNCTION ─────
void stopMotor() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
}