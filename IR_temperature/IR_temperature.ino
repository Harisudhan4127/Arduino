#include <mlx90615.h>
#include <Wire.h>
MLX90615 mlx = MLX90615();

void setup() {

  //setup MLX IR sensor

  mlx.begin();

  // initialize   serial communication:

  Serial.begin(9600);
}

void loop() {
  Serial.print("Ambient = ");
  Serial.print(mlx.get_ambient_temp());
  Serial.print(" *C\	Object = ");
  Serial.print(mlx.get_object_temp());
  Serial.println(" *C");
}
