#define USE_ARDUINO_INTERRUPTS true

#include <PulseSensorPlayground.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>
#include <TinyGPS++.h>

// -------- WIFI --------
const char* ssid = "ESP32_Asthma_Device";
const char* password = "12345678";

WebServer server(80);

// -------- GPS --------
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

static const int RXPin = 16;
static const int TXPin = 17;
static const uint32_t GPSBaud = 9600;

String latitude = "Searching";
String longitude = "Searching";

// -------- LCD --------
LiquidCrystal_I2C lcd(0x27,16,2);

// -------- Pulse Sensor --------
const int PULSE_SENSOR_PIN = 34;
const int LED_PIN = 2;
const int THRESHOLD = 550;

PulseSensorPlayground pulseSensor;

// -------- MQ135 --------
const int MQ135_SENSOR_PIN = 35;

// -------- DHT11 --------
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// -------- OUTPUT --------
int buzzer = 13;

// -------- VARIABLES --------
int bpm = 0;
float temperature = 0;
float humidity = 0;
String air_quality;
String asthma_status = "SAFE";

// -------- AIR QUALITY --------
String air_quality_data(int value)
{
if (value < 50) return "Excellent";
else if (value < 100) return "Good";
else if (value < 150) return "Moderate";
else if (value < 200) return "Poor";
else return "Danger";
}

// -------- WEB PAGE --------
void handleRoot()
{

String page = "<html><head>";
page += "<title>Asthma Monitoring Device</title>";
page += "<meta http-equiv='refresh' content='3'>";
page += "</head><body>";

page += "<h2>Asthma Monitoring System</h2>";

page += "<p><b>Heart Rate:</b> " + String(bpm) + " BPM</p>";
page += "<p><b>Temperature:</b> " + String(temperature) + " C</p>";
page += "<p><b>Humidity:</b> " + String(humidity) + " %</p>";
page += "<p><b>Air Quality:</b> " + air_quality + "</p>";

if(asthma_status=="ASTHMA ALERT!")
page += "<h3 style='color:red'>ASTHMA ALERT!</h3>";
else
page += "<h3 style='color:green'>SAFE</h3>";

page += "<h3>Device Location</h3>";
page += "<p>Latitude: " + latitude + "</p>";
page += "<p>Longitude: " + longitude + "</p>";

page += "<a href='https://maps.google.com/?q=" + latitude + "," + longitude + "' target='_blank'>";
page += "Open in Google Maps</a>";

page += "</body></html>";

server.send(200,"text/html",page);

}

// -------- SETUP --------
void setup()
{

Serial.begin(115200);

pinMode(buzzer,OUTPUT);

// LCD
lcd.init();
lcd.backlight();

lcd.setCursor(0,0);
lcd.print("Asthma Monitor");
lcd.setCursor(0,1);
lcd.print("Initializing");
delay(2000);
lcd.clear();

// DHT
dht.begin();

// Pulse Sensor
pulseSensor.analogInput(PULSE_SENSOR_PIN);
pulseSensor.blinkOnPulse(LED_PIN);
pulseSensor.setThreshold(THRESHOLD);

if (pulseSensor.begin())
Serial.println("Pulse Sensor Ready");

// GPS
gpsSerial.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
Serial.println("GPS Started");

// WIFI
WiFi.softAP(ssid,password);

Serial.println("WiFi Started");
Serial.println(WiFi.softAPIP());

// WEB SERVER
server.on("/",handleRoot);
server.begin();

}

// -------- LOOP --------
void loop()
{

server.handleClient();

// HEART RATE
bpm = pulseSensor.getBeatsPerMinute();

if (pulseSensor.sawStartOfBeat())
{
Serial.print("BPM: ");
Serial.println(bpm);
}

// AIR QUALITY
int sensor_value = analogRead(MQ135_SENSOR_PIN);
air_quality = air_quality_data(sensor_value);

// DHT11
float h = dht.readHumidity();
float t = dht.readTemperature();

if(!isnan(h) && !isnan(t))
{
humidity = h;
temperature = t;
}

// GPS
while (gpsSerial.available() > 0)
{
gps.encode(gpsSerial.read());

if (gps.location.isUpdated())
{
latitude = String(gps.location.lat(),6);
longitude = String(gps.location.lng(),6);

Serial.print("Lat: ");
Serial.println(latitude);

Serial.print("Lon: ");
Serial.println(longitude);
}
}

// ASTHMA ALERT
if(sensor_value > 200 || humidity > 80 || temperature > 35 || bpm > 120 || bpm < 50)
{
asthma_status="ASTHMA ALERT!";
digitalWrite(buzzer,HIGH);
}
else
{
asthma_status="SAFE";
digitalWrite(buzzer,LOW);
}

// LCD PAGE 1
lcd.clear();
lcd.setCursor(0,0);
lcd.print("Heart Rate:");
lcd.setCursor(0,1);
lcd.print(bpm);
lcd.print(" BPM");

delay(2000);

// LCD PAGE 2
lcd.clear();
lcd.setCursor(0,0);
lcd.print("Temp:");
lcd.print(temperature);
lcd.print("C");

lcd.setCursor(0,1);
lcd.print("Hum:");
lcd.print(humidity);
lcd.print("%");

delay(2000);

// LCD PAGE 3
lcd.clear();
lcd.setCursor(0,0);
lcd.print("Air Quality:");
lcd.setCursor(0,1);
lcd.print(air_quality);

delay(2000);

// LCD PAGE 4 (GPS)
lcd.clear();

lcd.setCursor(0,0);
lcd.print("Lat:");
lcd.print(latitude.substring(0,7));

lcd.setCursor(0,1);
lcd.print("Lon:");
lcd.print(longitude.substring(0,7));

delay(3000);

}
