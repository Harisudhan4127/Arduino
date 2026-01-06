#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------- LCD ----------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- IR Sensors ----------
#define IR_LEFT  2
#define IR_RIGHT 3

// ---------- Motor Driver ----------
#define IN1 4
#define IN2 5
#define IN3 6
#define IN4 7
#define ENA 9
#define ENB 10

// ---------- VARIABLES ----------
char mode = 'M';        // Default MANUAL
int motorSpeed = 150;

// =================================================
//                    SETUP
// =================================================
void setup() {

  pinMode(IR_LEFT, INPUT);
  pinMode(IR_RIGHT, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Pathology Lab");
  lcd.setCursor(0,1);
  lcd.print("MANUAL MODE");
  delay(2000);
  showManual();

  Serial.begin(9600);
  Serial.println("Arduino Ready - MANUAL MODE");
}

// =================================================
//                     LOOP
// =================================================
void loop() {

  if (Serial.available()) {
    char cmd = Serial.read();
    executeCommand(cmd);
  }

  if (mode == 'A') {
    lineFollow();
  }
}

// =================================================
//                COMMAND HANDLER
// =================================================
void executeCommand(char cmd) {

  // Ignore newline characters
  if (cmd == '\n' || cmd == '\r') return;

  Serial.print("Received CMD: ");
  Serial.println(cmd);

  // ---------- MODE SWITCH ----------
  if (cmd == 'M') {
    mode = 'M';
    stopRobot();
    showManual();
    Serial.println("MODE SET TO: MANUAL");
    return;
  }

  if (cmd == 'A') {
    mode = 'A';
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("MODE: AUTO");
    Serial.println("MODE SET TO: AUTO");
    delay(500);
    return;
  }

  // ---------- MANUAL CONTROLS ----------
  if (mode == 'M') {

    if (cmd == 'F') {
      forward();
      lcdStatus("Forward");
      Serial.println("ACTION: FORWARD");
    }
    else if (cmd == 'B') {
      backward();
      lcdStatus("Backward");
      Serial.println("ACTION: BACKWARD");
    }
    else if (cmd == 'L') {
      left();
      lcdStatus("Left");
      Serial.println("ACTION: LEFT");
    }
    else if (cmd == 'R') {
      right();
      lcdStatus("Right");
      Serial.println("ACTION: RIGHT");
    }
    else if (cmd == 'S') {
      stopRobot();
      lcdStatus("STOP");
      Serial.println("ACTION: STOP");
    }
  }
}



// =================================================
//                 AUTO LINE FOLLOW
// =================================================
void lineFollow() {

  int L = digitalRead(IR_LEFT);
  int R = digitalRead(IR_RIGHT);

  lcd.setCursor(0,0);
  lcd.print("MODE: AUTO     ");

  if (L == LOW && R == LOW) forward();
  else if (L == HIGH && R == LOW) right();
  else if (L == LOW && R == HIGH) left();
  else stopRobot();
}

// =================================================
//                LCD HELPERS
// =================================================
void showManual() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("MODE: MANUAL");
  lcd.setCursor(0,1);
  lcd.print("Waiting Cmd");
}

void lcdStatus(const char* msg) {
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print(msg);
}

// =================================================
//                MAPPED LOCATIONS
// =================================================
void goCollection() {
  lcd.clear();
  lcd.print("To Collection");
  forward(); delay(2000);
  right();   delay(700);
  stopRobot();
}

void goTesting() {
  lcd.clear();
  lcd.print("To Testing Lab");
  forward(); delay(3000);
  left();    delay(700);
  stopRobot();
}

void goStorage() {
  lcd.clear();
  lcd.print("To Storage");
  forward(); delay(1500);
  stopRobot();
}

// =================================================
//                MOTOR FUNCTIONS
// =================================================
void forward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}
void backward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}
void left() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}
void right() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}
void stopRobot() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}
