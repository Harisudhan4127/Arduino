#include <mlx90615.h>
#include <Wire.h>
MLX90615 mlx = MLX90615();
const int AOUT = 0;
const int DOUT = 8;
const int ledPin = 13;

int limit;
int value;

void setup() {

  mlx.begin();

  pinMode(DOUT, INPUT);
  pinMode(ledPin, OUTPUT);

  Serial.begin(115200);
}

void loop() {

  Serial.print("Ambient = ");
  Serial.print(mlx.get_ambient_temp());
  Serial.print(" *C\	Object = ");
  Serial.print(mlx.get_object_temp());
  Serial.println(" *C");
  
  value = analogRead(AOUT);
  limit = digitalRead(DOUT);
  Serial.print("Alcohol value:  ");
  Serial.println(value);
  Serial.print("Limit:  ");
  Serial.print(limit);
  delay(500);
}