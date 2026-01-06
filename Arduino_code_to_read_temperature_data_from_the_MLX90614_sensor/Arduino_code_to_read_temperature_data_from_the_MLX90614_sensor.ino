#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <NewPing.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
#define TRIG_PIN 9
#define ECHO_PIN 10
#define BUZZER_PIN 6
#define MAX_DISTANCE 100
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

void setup() {
    Serial.begin(9600);
    mlx.begin();
    pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
    delay(500);
    int distance = sonar.ping_cm();
    
    if (distance > 0 && distance <= 10) {
        delay(2000); // Allow time for stable reading
        float temp = mlx.readObjectTempC();
        Serial.print("Temperature: ");
        Serial.print(temp);
        Serial.println(" °C");
        
        if (temp > 30.0) {
            tone(BUZZER_PIN, 1000, 500); // High temperature alert
        } else {
            tone(BUZZER_PIN, 500, 500);  // Low temperature alert
        }
    }
}