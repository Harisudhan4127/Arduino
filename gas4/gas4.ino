int sensorPin=A2;
int sensorData;
int ledPin = 11;
void setup()
{  
  Serial.begin(9600);   
  pinMode(sensorPin,INPUT);
  pinMode(ledPin,OUTPUT);                         
 }
void loop()
{
  sensorData = analogRead(sensorPin);       
  Serial.print("Sensor Data:");
if (sensorData >= 550){
  digitalWrite(ledPin, HIGH);
}
  delay(100);                                   
}
