#include <Adafruit_MLX90614.h>   // Library for temperature sensor

#define MQ3_AOUT A0      // MQ3 Analog output
#define BUZZER_PIN 7     // Buzzer pin
#define LED_PIN 13       // LED pin

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(9600);
  mlx.begin();                     // Initialize MLX90614
  
  pinMode(MQ3_AOUT, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  Serial.println("Alcohol + Temperature Detection System Initialized...");
  delay(1000);
}

void loop() {
  // --- Read Alcohol Level ---
  int alcoholValue = analogRead(MQ3_AOUT);

  // --- Read Body Temperature ---
  float temperature = mlx.readObjectTempC();

  // --- Display Values ---
  Serial.print("Alcohol Value: ");
  Serial.print(alcoholValue);
  Serial.print(" | Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  // --- Detection Logic ---
  bool alcoholDetected = (alcoholValue > 550);           // adjust threshold
  bool abnormalTemp = (temperature >= 38.0 || temperature <= 35.0); // abnormal range

  if (alcoholDetected && abnormalTemp) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 1000);
    Serial.println("⚠️ ALERT: Drunk person with abnormal temperature detected!");
  } 
  else if (alcoholDetected) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 800);
    Serial.println("⚠️ ALERT: Alcohol detected!");
  } 
  else if (abnormalTemp) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 600);
    Serial.println("⚠️ ALERT: Abnormal temperature detected!");
  } 
  else {
    noTone(BUZZER_PIN);
    digitalWrite(LED_PIN, LOW);
    Serial.println("✅ Normal condition");
  }

  delay(1000);
}
