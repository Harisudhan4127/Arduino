#include <Adafruit_MLX90614.h>

#define AOUT A0          // Analog output from alcohol sensor
#define DOUT 8           // Digital output from alcohol sensor
#define BUZZER_PIN 7     // Define buzzer pin
#define LED_PIN 13       // Built-in LED pin

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

int limit;
int value;

void setup() {
  Serial.begin(9600);
  mlx.begin();

  pinMode(DOUT, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  float temp = mlx.readObjectTempC();
  value = analogRead(AOUT);
  limit = digitalRead(DOUT);

  if ((temp >= 38.0) || (value > 560)) {
    tone(BUZZER_PIN, 1000, 500);  
    digitalWrite(LED_PIN, HIGH);
  } else {
    noTone(BUZZER_PIN);        
    digitalWrite(LED_PIN, LOW);
  }

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" °C");
  
  Serial.print("Alcohol value: ");
  Serial.println(value);
  
  Serial.print("Digital limit: ");
  Serial.println(limit);
  
  delay(500);
}
