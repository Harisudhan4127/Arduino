#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define SensorPin 34
#define TFT_CS   5
#define TFT_RST  4
#define TFT_DC   2

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
WebServer server(80);
Preferences prefs;

// WiFi AP
const char* ssidAP = "SURAS_Device";
const char* passAP = "12345678";

// Calibration
float slope = -7.24;
float intercept = 26.7;

// Voltage reference
float vRef = 3.3;

// Calibration temp
float calVolt7 = 0;
float calVolt4 = 0;
bool cal7_done = false;
bool cal4_done = false;

// Time
int hours=0, minutes=0, seconds=0;
bool isPM=false;
unsigned long lastClock=0;

// Cache
float lastPH=-1;
float lastVolt=-1;

unsigned long sensorTimer=0;
const int sensorInterval=200;   // ⚡ FAST UPDATE

// ================= FAST SENSOR =================
float readVoltage(){
  long total=0;
  for(int i=0;i<10;i++){  // ⚡ Reduced samples
    total+=analogRead(SensorPin);
    delayMicroseconds(100);
  }
  float avg=total/10.0;
  return (avg * vRef) / 4095.0;
}

float readPH(float v){
  return slope*v + intercept;
}

// ⚡ FAST SMOOTHING
float smoothPH(float newPH){
  static float prevPH = 7.0;
  float smoothed = (prevPH * 0.4) + (newPH * 0.6);
  prevPH = smoothed;
  return smoothed;
}

String phStatus(float pH){
  if (pH < 4) return "Very Acidic";
  else if (pH < 6.5) return "Acidic";
  else if (pH <= 7.5) return "Neutral";
  else if (pH < 9) return "Alkaline";
  else return "Strong Alkaline";
}

// ================= CALIBRATION =================
void saveCalibration(){
  prefs.putFloat("slope", slope);
  prefs.putFloat("intercept", intercept);
}

void calculateCalibration(){
  if(cal7_done && cal4_done){
    slope = (7.0 - 4.0) / (calVolt7 - calVolt4);
    intercept = 7.0 - slope * calVolt7;
    saveCalibration();
  }
}

// ================= UI =================
void drawBase(){
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRect(0,0,160,18,tft.color565(20,30,50));

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(30,22);
  tft.print("SURAS Diagnostic");
}

void centerText(String text,int y,int size,uint16_t color){
  tft.setTextSize(size);
  tft.setTextColor(color);
  int16_t x1,y1; uint16_t w,h;
  tft.getTextBounds(text,0,0,&x1,&y1,&w,&h);
  tft.setCursor((160-w)/2,y);
  tft.print(text);
}

// ================= DISPLAY =================
void updateDisplay(float volt,float pH){

  if(abs(pH-lastPH)<0.005) return;   // ⚡ faster update

  lastPH=pH;
  lastVolt=volt;

  tft.fillRect(0,40,160,80,ST77XX_BLACK);

  String status=phStatus(pH);

  centerText(String(pH,2),50,4,ST77XX_WHITE);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(40,90);
  tft.print(status);

  tft.setCursor(20,110);
  tft.print("V:");
  tft.print(volt,2);
}

// ================= WEB =================
void handleRoot(){

  String page="<html><head>";
  page+="<meta name='viewport' content='width=device-width, initial-scale=1'>";
  page+="<meta http-equiv='refresh' content='1'>";  // ⚡ fast refresh
  page+="</head><body style='text-align:center;font-family:Arial;'>";

  page+="<h2>SURAS Real-Time pH</h2>";
  page+="<h1>"+String(lastPH,2)+"</h1>";
  page+="<h3>"+phStatus(lastPH)+"</h3>";
  page+="<p>Voltage: "+String(lastVolt,2)+" V</p>";

  page+="<button onclick=\"location.href='/cal7'\">Calibrate pH 7</button>";
  page+="<button onclick=\"location.href='/cal4'\">Calibrate pH 4</button>";

  page+="</body></html>";

  server.send(200,"text/html",page);
}

// Calibration
void handleCal7(){
  calVolt7 = readVoltage();
  cal7_done = true;
  calculateCalibration();
  server.send(200,"text/plain","pH7 done");
}

void handleCal4(){
  calVolt4 = readVoltage();
  cal4_done = true;
  calculateCalibration();
  server.send(200,"text/plain","pH4 done");
}

// ================= SETUP =================
void setup(){

  Serial.begin(115200);

  prefs.begin("phcal", false);
  slope = prefs.getFloat("slope", -7.24);
  intercept = prefs.getFloat("intercept", 26.7);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  drawBase();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssidAP,passAP);

  server.on("/",handleRoot);
  server.on("/cal7", handleCal7);
  server.on("/cal4", handleCal4);

  server.begin();
}

// ================= LOOP =================
void loop(){

  server.handleClient();

  if(millis()-sensorTimer>=sensorInterval){
    sensorTimer=millis();

    float volt = readVoltage();
    float rawPH = readPH(volt);
    float pH = smoothPH(rawPH);

    updateDisplay(volt,pH);
  }
}