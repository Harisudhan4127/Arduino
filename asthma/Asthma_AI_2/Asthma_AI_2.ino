#define USE_ARDUINO_INTERRUPTS false
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <DHT.h>
#include <TinyGPS++.h>
#include <PulseSensorPlayground.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 4
#define DHTTYPE DHT11
#define MQ135_PIN 34

// LEDs & Buzzer
#define RED 25
#define YELLOW 26
#define GREEN 27
#define BUZZER 33

// GPS using HardwareSerial 2
#define RXD2 16
#define TXD2 17
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;

// Pulse Sensor
#define PULSE_SENSOR_PIN 36  // ADC1 channel 0 (VP)
#define LED_PIN 2            // Onboard LED
#define THRESHOLD 550
PulseSensorPlayground pulseSensor;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi credentials
const char* ssid = "Sri";
const char* password = "Sri_Home_Bas_Raji";

WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);

/* Sensor variables */
float temperature = 0;
float humidity = 0;
int air_value = 0;
int bpm = 0;

String air_quality = "Normal";
String asthma_status = "SAFE";
String issue_note = "Sensors OK";
int ai_risk_percent = 0;
float latitude = 0;
float longitude = 0;

/* Filter buffers */
#define FILTER_SIZE 5
float tempBuffer[FILTER_SIZE];
float humBuffer[FILTER_SIZE];
int airBuffer[FILTER_SIZE];
int filterIndex = 0;

/* Task handle */
TaskHandle_t sensorTask;

/* ---------- FILTER FUNCTIONS ---------- */
float averageFloat(float *arr) { float sum = 0; for (int i = 0; i < FILTER_SIZE; i++) sum += arr[i]; return sum / FILTER_SIZE; }
int averageInt(int *arr) { int sum = 0; for (int i = 0; i < FILTER_SIZE; i++) sum += arr[i]; return sum / FILTER_SIZE; }

/* ---------- SENSOR TASK (CORE 0) ---------- */
void sensorTaskFunction(void *pvParameters) {
  while (true) {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    int air = analogRead(MQ135_PIN);

    if (isnan(t) || isnan(h)) issue_note = "DHT Sensor Error";
    else { tempBuffer[filterIndex] = t; humBuffer[filterIndex] = h; }
    if (air <= 0) issue_note = "MQ135 Sensor Error";
    airBuffer[filterIndex] = air;

    filterIndex = (filterIndex + 1) % FILTER_SIZE;
    temperature = averageFloat(tempBuffer);
    humidity = averageFloat(humBuffer);
    air_value = averageInt(airBuffer);

    if (air_value < 1200) air_quality = "Good";
    else if (air_value < 2500) air_quality = "Moderate";
    else air_quality = "Poor";

    bpm = pulseSensor.getBeatsPerMinute();
    if (pulseSensor.sawStartOfBeat()) {
      Serial.print("♥ Heartbeat detected! BPM: ");
      Serial.println(bpm);
    }
    if (bpm < 40 || bpm > 180) issue_note = "Pulse sensor unstable";

    ai_risk_percent = 0;
    if (bpm > 100) ai_risk_percent += 30;
    if (temperature > 37) ai_risk_percent += 20;
    if (air_quality == "Poor") ai_risk_percent += 40;
    if (humidity > 80) ai_risk_percent += 10;
    asthma_status = (ai_risk_percent > 60) ? "ALERT" : "SAFE";

    while (gpsSerial.available() > 0) { gps.encode(gpsSerial.read()); }

    // Browser-based location override if available
    if(server.hasArg("lat") && server.hasArg("lon")){
      latitude = server.arg("lat").toFloat();
      longitude = server.arg("lon").toFloat();
    } else if (gps.location.isUpdated()) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
    }

    if (asthma_status == "ALERT") {
      digitalWrite(RED, HIGH); digitalWrite(YELLOW, LOW); digitalWrite(GREEN, LOW); digitalWrite(BUZZER, HIGH);
    } else if (ai_risk_percent > 30) {
      digitalWrite(RED, LOW); digitalWrite(YELLOW, HIGH); digitalWrite(GREEN, LOW); digitalWrite(BUZZER, LOW);
    } else {
      digitalWrite(RED, LOW); digitalWrite(YELLOW, LOW); digitalWrite(GREEN, HIGH); digitalWrite(BUZZER, LOW);
    }

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("HR:"); lcd.print(bpm); lcd.print(" T:"); lcd.print(temperature);
    lcd.setCursor(0,1);
    lcd.print("Air:"); lcd.print(air_quality);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

/* ---------- WEB DATA API ---------- */
void handleData() {
  String json = "{";
  json += "\"bpm\":" + String(bpm) + ",";
  json += "\"temp\":" + String(temperature) + ",";
  json += "\"hum\":" + String(humidity) + ",";
  json += "\"air\":\"" + air_quality + "\",";
  json += "\"risk\":" + String(ai_risk_percent) + ",";
  json += "\"status\":\"" + asthma_status + "\",";
  json += "\"issue\":\"" + issue_note + "\",";
  json += "\"lat\":" + String(latitude, 6) + ",";
  json += "\"lng\":" + String(longitude, 6);
  json += "}";
  server.send(200, "application/json", json);
}

/* ---------- WEB DASHBOARD ---------- */
void handleRoot() {
  String page = R"====(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Asthma Monitor</title>
<style>
body{margin:0;font-family:Arial;background:#0f172a;color:white;}
.header{background:#111827;padding:18px;text-align:center;font-size:26px;font-weight:bold;}
.container{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:20px;padding:20px;}
.card{background:#1e293b;border-radius:12px;padding:20px;text-align:center;}
.title{font-size:18px;opacity:0.8;}
.value{font-size:32px;font-weight:bold;margin-top:10px;}
.alertSafe{background:#16a34a;padding:18px;text-align:center;font-size:22px;margin:20px;border-radius:10px;}
.alertWarn{background:#dc2626;padding:18px;text-align:center;font-size:22px;margin:20px;border-radius:10px;animation:blink 1s infinite;}
@keyframes blink{0%{opacity:1;}50%{opacity:0.4;}100%{opacity:1;}}
.location{background:#1e293b;margin:20px;padding:20px;border-radius:12px;text-align:center;}
.button{background:#2563eb;border:none;padding:12px 20px;color:white;font-size:16px;border-radius:6px;margin-top:10px;}
</style>
</head>
<body>
<div class="header">Asthma Monitoring Dashboard</div>
<div id="alertBox" class="alertSafe">Patient Status: SAFE</div>
<div class="container">
<div class="card"><div class="title">Heart Rate</div><div id="bpm" class="value">--</div></div>
<div class="card"><div class="title">Temperature</div><div id="temp" class="value">--</div></div>
<div class="card"><div class="title">Humidity</div><div id="hum" class="value">--</div></div>
<div class="card"><div class="title">Air Quality</div><div id="air" class="value">--</div></div>
<div class="card"><div class="title">AI Risk Level</div><div id="risk" class="value">--</div></div>
</div>
<div class="location">
<h3>Device Location</h3>
Latitude: <span id="lat">--</span><br>
Longitude: <span id="lng">--</span><br><br>
<a href="#" id="mapsLink" target="_blank"><button class="button">Open in Google Maps</button></a><br><br>
</div>
<script>
function updateData(){
  fetch("/data").then(res=>res.json()).then(data=>{
    document.getElementById("bpm").innerText=data.bpm+" BPM";
    document.getElementById("temp").innerText=data.temp+" C";
    document.getElementById("hum").innerText=data.hum+" %";
    document.getElementById("air").innerText=data.air;
    document.getElementById("risk").innerText=data.risk+" %";
    let alertBox=document.getElementById("alertBox");
    alertBox.className=(data.status=="SAFE")?"alertSafe":"alertWarn";
    alertBox.innerText=(data.status=="SAFE")?"Patient Status: SAFE":"⚠ ASTHMA ALERT - Immediate Attention Required";
    document.getElementById("lat").innerText=data.lat;
    document.getElementById("lng").innerText=data.lng;
    document.getElementById("mapsLink").href="https://maps.google.com/?q="+data.lat+","+data.lng;
  });
  // Automatically request browser location
  if(navigator.geolocation){
    navigator.geolocation.getCurrentPosition(function(position){
      fetch('/data?lat='+position.coords.latitude+'&lon='+position.coords.longitude);
    });
  }
}
setInterval(updateData,2000);
</script>
</body>
</html>
)====";
  server.send(200,"text/html",page);
}



/* ---------- SETUP ---------- */
void setup(){
  Serial.begin(115200);
  dht.begin();
  lcd.init(); lcd.backlight();
  pinMode(RED,OUTPUT); pinMode(YELLOW,OUTPUT); pinMode(GREEN,OUTPUT); pinMode(BUZZER,OUTPUT);
  gpsSerial.begin(9600,SERIAL_8N1,RXD2,TXD2);

  pulseSensor.analogInput(PULSE_SENSOR_PIN);
  pulseSensor.blinkOnPulse(LED_PIN);
  pulseSensor.setThreshold(THRESHOLD);
  if(pulseSensor.begin()){ Serial.println("PulseSensor initialized successfully!"); }

  WiFi.begin(ssid,password);
  lcd.setCursor(0,0); lcd.print("Connecting WiFi");
  while(WiFi.status()!=WL_CONNECTED){ delay(500); Serial.print("."); }
  lcd.clear(); lcd.print("WiFi Connected"); lcd.setCursor(0,1); lcd.print(WiFi.localIP()); delay(3000);

  server.on("/",handleRoot);
  server.on("/data",handleData);
  server.begin();

  xTaskCreatePinnedToCore(sensorTaskFunction,"SensorTask",4096,NULL,1,&sensorTask,0);
}



/* ---------- LOOP ---------- */
void loop(){ server.handleClient(); }
