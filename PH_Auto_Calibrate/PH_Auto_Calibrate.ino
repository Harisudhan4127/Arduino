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

// Calibration (default fallback)
float slope = -7.24;
float intercept = 26.7;

// Voltage reference (measure for accuracy)
float vRef = 3.3;

// Calibration temp values
float calVolt7 = 0;
float calVolt4 = 0;
bool cal7_done = false;
bool cal4_done = false;

// Web theme
bool webDark = true;

// Time
int hours=0, minutes=0, seconds=0;
bool isPM=false;
unsigned long lastClock=0;

// Cache
float lastPH=-1;
float lastVolt=-1;

unsigned long sensorTimer=0;
const int sensorInterval=800;

// ================= SENSOR =================
float readVoltage(){
  long total=0;
  for(int i=0;i<30;i++){
    total+=analogRead(SensorPin);
    delayMicroseconds(300);
  }
  float avg=total/30.0;
  return (avg * vRef) / 4095.0;
}

float readPH(float v){
  return slope*v + intercept;
}

// smoothing
float smoothPH(float newPH){
  static float prevPH = 7.0;
  float smoothed = (prevPH * 0.7) + (newPH * 0.3);
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
  Serial.println("Calibration Saved!");
}

void calculateCalibration(){
  if(cal7_done && cal4_done){
    slope = (7.0 - 4.0) / (calVolt7 - calVolt4);
    intercept = 7.0 - slope * calVolt7;

    saveCalibration();

    Serial.println("New Calibration:");
    Serial.print("Slope: "); Serial.println(slope);
    Serial.print("Intercept: "); Serial.println(intercept);
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

  tft.drawLine(10,36,150,36,ST77XX_WHITE);
  tft.drawLine(10,110,150,110,ST77XX_WHITE);
}

void centerText(String text,int y,int size,uint16_t color){
  tft.setTextSize(size);
  tft.setTextColor(color);
  int16_t x1,y1; uint16_t w,h;
  tft.getTextBounds(text,0,0,&x1,&y1,&w,&h);
  tft.setCursor((160-w)/2,y);
  tft.print(text);
}

// ================= CLOCK =================
void updateClock(){
  if(millis()-lastClock>=1000){
    lastClock=millis();
    seconds++;
    if(seconds>=60){seconds=0;minutes++;}
    if(minutes>=60){minutes=0;hours++;}
    if(hours>=24)hours=0;

    int displayHour=hours%12;
    if(displayHour==0) displayHour=12;
    isPM=hours>=12;

    char buf[15];
    sprintf(buf,"%02d:%02d:%02d %s",
            displayHour,minutes,seconds,
            isPM?"PM":"AM");

    tft.fillRect(5,4,120,12,tft.color565(20,30,50));
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(5,6);
    tft.print(buf);
  }
}

// ================= WIFI =================
void drawWiFi(){
  int stations = WiFi.softAPgetStationNum();
  int bars = stations>0 ? 4 : 1;

  tft.fillRect(110,4,45,12,tft.color565(20,30,50));

  int x=120,y=8;
  for(int i=0;i<4;i++){
    if(i<bars)
      tft.fillRect(x+i*6,y-i*2,4,i*2+4,ST77XX_GREEN);
    else
      tft.drawRect(x+i*6,y-i*2,4,i*2+4,ST77XX_WHITE);
  }
}

// ================= DISPLAY =================
void updateDisplay(float volt,float pH){

  if(abs(pH-lastPH)<0.01) return;

  lastPH=pH;
  lastVolt=volt;

  tft.fillRect(0,40,160,65,ST77XX_BLACK);

  String status=phStatus(pH);

  uint16_t statusColor =
      status=="Very Acidic" ? ST77XX_RED :
      status=="Acidic" ? tft.color565(255,140,0) :
      status=="Neutral" ? ST77XX_GREEN :
      status=="Alkaline" ? ST77XX_CYAN :
      ST77XX_MAGENTA;

  centerText(String(pH,2),50,4,ST77XX_WHITE);

  tft.fillRect(20,85,120,18,statusColor);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor(45,90);
  tft.print(status);

  tft.fillRect(0,112,160,30,ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(15,118);
  tft.print("Voltage: ");
  tft.print(volt,2);
  tft.print(" V");

  tft.setCursor(15,130);
  tft.print("System: Stable");
}

// ================= WEB =================
void handleRoot(){

  String page="<html><head>";
  page+="<meta name='viewport' content='width=device-width, initial-scale=1'>";
  page+="<meta http-equiv='refresh' content='3'>";

  page+="<style>body{font-family:Arial;text-align:center;";
  page+=webDark?"background:#0f172a;color:white;}":"background:#f3f4f6;color:black;}";
  page+=".card{background:"+(webDark?String("#1e293b"):String("white"))+";";
  page+="padding:25px;margin:20px;border-radius:15px;";
  page+="box-shadow:0 4px 12px rgba(0,0,0,0.2);}button{padding:10px 18px;";
  page+="margin:8px;border-radius:8px;border:none;font-size:14px;cursor:pointer;}";
  page+="</style></head><body>";

  page+="<div class='card'><h2>SURAS Dashboard</h2>";
  page+="<h1>"+String(lastPH,2)+"</h1>";
  page+="<h3>"+phStatus(lastPH)+"</h3>";
  page+="<p>Voltage: "+String(lastVolt,2)+" V</p>";

  page+="<button onclick=\"location.href='/cal7'\">Calibrate pH 7</button>";
  page+="<button onclick=\"location.href='/cal4'\">Calibrate pH 4</button>";
  page+="<button onclick=\"location.href='/toggleWeb'\">Toggle Theme</button>";

  page+="</div></body></html>";

  server.send(200,"text/html",page);
}

// Calibration routes
void handleCal7(){
  calVolt7 = readVoltage();
  cal7_done = true;
  calculateCalibration();
  server.send(200,"text/plain","Calibrated pH 7");
}

void handleCal4(){
  calVolt4 = readVoltage();
  cal4_done = true;
  calculateCalibration();
  server.send(200,"text/plain","Calibrated pH 4");
}

void toggleWeb(){
  webDark=!webDark;
  server.sendHeader("Location","/");
  server.send(303);
}

// ================= SETUP =================
void setup(){

  Serial.begin(115200);

  // Preferences
  prefs.begin("phcal", false);

  slope = prefs.getFloat("slope", -7.24);
  intercept = prefs.getFloat("intercept", 26.7);

  Serial.println("Loaded Calibration:");
  Serial.println(slope);
  Serial.println(intercept);

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
  server.on("/toggleWeb",toggleWeb);

  server.begin();
}

// ================= LOOP =================
void loop(){

  server.handleClient();
  updateClock();
  drawWiFi();

  if(millis()-sensorTimer>=sensorInterval){
    sensorTimer=millis();

    float volt = readVoltage();
    float rawPH = readPH(volt);
    float pH = smoothPH(rawPH);

    updateDisplay(volt,pH);
  }
}