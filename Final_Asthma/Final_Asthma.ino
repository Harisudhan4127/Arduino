#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <DHT.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"

// ---------------- TFT ----------------
#define TFT_CS 5
#define TFT_RST 4
#define TFT_DC 2
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// ---------------- SENSORS ----------------
#define DHTPIN 13
#define DHTTYPE DHT11
#define MQ135_PIN 34
#define PIN_LM35 36

#define ADC_VREF_mV 3300.0
#define ADC_RESOLUTION 4096.0

DHT dht(DHTPIN, DHTTYPE);
MAX30105 particleSensor;

// ---------------- BUZZER ----------------
#define BUZZER_PIN 15

// ---------------- WIFI ----------------
const char* ssid = "AsthmaMonitor";
const char* password = "12345678";
WebServer server(80);

// ---------------- DATA ----------------
int bpm = 0;
int spo2 = 98;
float temperature = 0;
float humidity = 0;
float bodyTemp = 0;
int airPercent = 0;
String status = "STABLE";

// MAX30105 Buffers
#define BUFFER_LENGTH 100
uint32_t irBuffer[BUFFER_LENGTH];
uint32_t redBuffer[BUFFER_LENGTH];
int32_t spo2Val = 0, heartRateVal = 0;
int8_t validSPO2, validHR;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  dht.begin();

  // TFT
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  drawBase();

  // MAX30105
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    while (1)
      ;
  }
  particleSensor.setup(50, 1, 2, 100, 69, 4096);

  // WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  // Web
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

// ---------------- LOOP ----------------
void loop() {
  server.handleClient();
  drawWiFi();
  readSensors();
  calculateRisk();
  handleAlert();  // ALERT SYSTEM
  updateDisplay();
}

// ---------------- UI BASE ----------------
void drawBase() {
  tft.fillScreen(ST77XX_BLACK);

  tft.fillRect(0, 0, 160, 20, tft.color565(20, 20, 60));

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_CYAN);

  String title = "ASTHMA ALERT";
  int16_t x1, y1;
  uint16_t w, h;

  tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  int x = (160 - w) / 2;

  tft.setCursor(x, 22);
  tft.print(title);

  tft.drawLine(10, 35, 150, 35, ST77XX_WHITE);
  tft.drawLine(10, 110, 150, 110, ST77XX_WHITE);
}

// ---------------- WIFI BAR ----------------
void drawWiFi() {
  // Clear top bar
  tft.fillRect(0, 0, 160, 20, tft.color565(20, 20, 60));

  // ================= WIFI ICON (LEFT) =================
  int stations = WiFi.softAPgetStationNum();
  int bars = stations > 0 ? 4 : 1;

  int x = 5, y = 15;

  for (int i = 0; i < 4; i++) {
    if (i < bars)
      tft.fillRect(x + i * 6, y - i * 3, 4, i * 3, ST77XX_GREEN);
    else
      tft.drawRect(x + i * 6, y - i * 3, 4, i * 3, ST77XX_WHITE);
  }

  // ================= IP ADDRESS (RIGHT ALIGNED) =================
  IPAddress ip = WiFi.softAPIP();
  String ipStr = ip.toString();

  int16_t x1, y1;
  uint16_t w, h;

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  // Get width of IP text
  tft.getTextBounds(ipStr, 0, 0, &x1, &y1, &w, &h);

  // Right align with padding
  int ipX = 160 - w - 5;

  tft.setCursor(ipX, 5);
  tft.print(ipStr);
}

// ---------------- SENSOR ----------------
void readSensors() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  int air = analogRead(MQ135_PIN);
  airPercent = map(air, 0, 4095, 100, 0);

  // LM35 BODY TEMP
  int adcVal = analogRead(PIN_LM35);
  float milliVolt = adcVal * (ADC_VREF_mV / ADC_RESOLUTION);
  bodyTemp = milliVolt / 10.0;

  // MAX30105
  for (byte i = 0; i < BUFFER_LENGTH; i++) {
    while (!particleSensor.available()) particleSensor.check();
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_LENGTH, redBuffer,
                                         &spo2Val, &validSPO2,
                                         &heartRateVal, &validHR);

  if (validHR) bpm = heartRateVal;
  if (validSPO2) spo2 = spo2Val;
}

// ---------------- AI ----------------
void calculateRisk() {
  int risk = 0;

  if (spo2 < 92) risk += 50;
  if (bpm > 110) risk += 30;
  if (airPercent < 75) risk += 30;
  if (humidity > 80) risk += 10;

  // BODY TEMP
  if (bodyTemp > 38.0) risk += 40;
  else if (bodyTemp > 37.5) risk += 20;

  if (risk > 80) status = "CRITICAL";
  else if (risk > 60) status = "ALERT";
  else if (risk > 30) status = "CAUTION";
  else status = "STABLE";
}

// ---------------- ALERT ----------------
void handleAlert() {
  if (status == "CRITICAL") {
    digitalWrite(BUZZER_PIN, HIGH);
  } else if (bodyTemp > 38.5) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// ---------------- DISPLAY ----------------
void updateDisplay() {
  static int lastBPM = -1, lastAir = -1, lastSpO2 = -1;
  static float lastTemp = -1, lastHum = -1;
  static String lastStatus = "";

  if (bpm == lastBPM && airPercent == lastAir && status == lastStatus && spo2 == lastSpO2 && temperature == lastTemp && humidity == lastHum) return;

  lastBPM = bpm;
  lastAir = airPercent;
  lastStatus = status;
  lastSpO2 = spo2;
  lastTemp = temperature;
  lastHum = humidity;

  // Clear center area
  tft.fillRect(0, 40, 160, 65, ST77XX_BLACK);

  // ================= BPM =================
  String bpmStr = String(bpm);

  int16_t x1, y1;
  uint16_t w, h;

  tft.setTextSize(4);
  tft.setTextColor(ST77XX_WHITE);
  tft.getTextBounds(bpmStr, 0, 0, &x1, &y1, &w, &h);

  int x = (160 - w) / 2;

  tft.setCursor(x, 50);
  tft.print(bpmStr);

  // BPM Label
  tft.setTextSize(1);
  tft.setCursor(x + w + 5, 70);
  tft.print("BPM");

  // Heart dot ❤️
  tft.fillCircle(x + w + 8, 58, 3, ST77XX_RED);

  // ================= STATUS =================
  uint16_t color =
    status == "STABLE" ? ST77XX_GREEN : status == "CAUTION" ? ST77XX_YELLOW
                                      : status == "ALERT"   ? tft.color565(255, 140, 0)
                                                            : ST77XX_RED;

  // Rounded status bar
  tft.fillRoundRect(20, 85, 120, 18, 5, color);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_BLACK);

  tft.getTextBounds(status, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(20 + (120 - w) / 2, 85 + (18 - h) / 2 + 1);
  tft.print(status);

  // ================= BOTTOM DATA =================
  tft.fillRect(0, 112, 160, 30, ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  // Air 🌬
  tft.setCursor(5, 115);
  tft.print("Air:");
  tft.print(airPercent);
  tft.print("%");

  // SpO2 🩸
  // tft.setCursor(5, 125);
  // tft.print("SpO2:");
  // tft.print(spo2);
  // tft.print("%");

  // Temperature 🌡
  tft.setCursor(80, 115);
  tft.print("Temp:");
  tft.print(temperature, 1);
  tft.print("C");

  // Humidity 💧
  // tft.setCursor(80, 125);
  // tft.print("Hum:");
  // tft.print(humidity, 0);
  // tft.print("%");

  // ================= TOP RIGHT TEMP =================
  tft.fillRect(100, 2, 60, 16, tft.color565(20, 20, 60));

  tft.setCursor(100, 4);
  tft.setTextColor(ST77XX_YELLOW);
  tft.print(temperature, 1);
  tft.print("C");
}


// ---------------- WEB ----------------
void handleData() {
  String json = "{";
  json += "\"bpm\":" + String(bpm) + ",";
  json += "\"spo2\":" + String(spo2) + ",";
  json += "\"air\":" + String(airPercent) + ",";
  json += "\"bodyTemp\":" + String(bodyTemp, 1) + ",";
  json += "\"status\":\"" + status + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleRoot() {
  String page = R"====(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Asthma Alert System Monitor</title>
<link href="https://fonts.googleapis.com/css2?family=Space+Mono:wght@400;700&family=Syne:wght@400;600;800&display=swap" rel="stylesheet">
<style>
  :root {
    --bg: #050c18;
    --surface: #0b1628;
    --border: #1a2d4a;
    --text: #c8d8f0;
    --muted: #4a6080;
    --normal: #00e5a0;
    --normal-glow: rgba(0,229,160,0.25);
    --risk: #f5a623;
    --risk-glow: rgba(245,166,35,0.25);
    --critical: #ff2d55;
    --critical-glow: rgba(255,45,85,0.35);
    --accent: #3d8bff;
  }

  * { margin: 0; padding: 0; box-sizing: border-box; }

  body {
    font-family: 'Syne', sans-serif;
    background: var(--bg);
    color: var(--text);
    min-height: 100vh;
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 20px 16px 40px;
    position: relative;
    overflow-x: hidden;
  }

  /* Subtle grid background */
  body::before {
    content: '';
    position: fixed;
    inset: 0;
    background-image:
      linear-gradient(rgba(61,139,255,0.04) 1px, transparent 1px),
      linear-gradient(90deg, rgba(61,139,255,0.04) 1px, transparent 1px);
    background-size: 40px 40px;
    pointer-events: none;
    z-index: 0;
  }

  /* Corner scanline shimmer */
  body::after {
    content: '';
    position: fixed;
    top: -100%;
    left: 0; right: 0;
    height: 60%;
    background: linear-gradient(to bottom, transparent, rgba(61,139,255,0.03), transparent);
    animation: scanline 6s linear infinite;
    pointer-events: none;
    z-index: 0;
  }
  @keyframes scanline { to { top: 200%; } }

  /* ── Header ── */
  .header {
    position: relative;
    z-index: 1;
    width: 100%;
    max-width: 520px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    margin-bottom: 24px;
    padding-bottom: 16px;
    border-bottom: 1px solid var(--border);
  }
  .logo {
    display: flex;
    align-items: center;
    gap: 10px;
  }
  .logo-icon {
    width: 36px; height: 36px;
    background: linear-gradient(135deg, #3d8bff, #00e5a0);
    border-radius: 8px;
    display: flex; align-items: center; justify-content: center;
    font-size: 18px;
  }
  .logo-text { font-size: 18px; font-weight: 800; letter-spacing: -0.5px; color: white; }
  .logo-text span { color: var(--accent); }
  .live-badge {
    display: flex; align-items: center; gap: 6px;
    font-family: 'Space Mono', monospace;
    font-size: 11px;
    color: var(--normal);
    background: rgba(0,229,160,0.08);
    border: 1px solid rgba(0,229,160,0.2);
    padding: 4px 10px;
    border-radius: 20px;
  }
  .live-dot {
    width: 6px; height: 6px;
    background: var(--normal);
    border-radius: 50%;
    animation: pulse-dot 1.5s ease-in-out infinite;
  }
  @keyframes pulse-dot {
    0%, 100% { opacity: 1; transform: scale(1); }
    50%       { opacity: 0.4; transform: scale(0.7); }
  }

  /* ── Status Banner ── */
  .status-banner {
    position: relative;
    z-index: 1;
    width: 100%;
    max-width: 520px;
    border-radius: 16px;
    padding: 20px 24px;
    margin-bottom: 20px;
    display: flex;
    align-items: center;
    gap: 16px;
    border: 1.5px solid transparent;
    transition: all 0.5s cubic-bezier(0.4,0,0.2,1);
    overflow: hidden;
  }
  .status-banner::before {
    content: '';
    position: absolute;
    inset: 0;
    opacity: 0.07;
    background: var(--status-color, var(--normal));
    z-index: -1;
    transition: background 0.5s;
  }

  .status-ring {
    flex-shrink: 0;
    width: 56px; height: 56px;
    border-radius: 50%;
    border: 3px solid var(--status-color, var(--normal));
    display: flex; align-items: center; justify-content: center;
    font-size: 24px;
    position: relative;
    transition: border-color 0.5s;
  }
  .status-ring::after {
    content: '';
    position: absolute;
    inset: -6px;
    border-radius: 50%;
    border: 1.5px solid var(--status-color, var(--normal));
    opacity: 0.3;
    animation: ring-expand 2s ease-out infinite;
    transition: border-color 0.5s;
  }
  @keyframes ring-expand {
    0%   { transform: scale(0.95); opacity: 0.4; }
    100% { transform: scale(1.4);  opacity: 0; }
  }

  .status-info { flex: 1; }
  .status-label {
    font-size: 11px;
    font-family: 'Space Mono', monospace;
    letter-spacing: 2px;
    text-transform: uppercase;
    color: var(--muted);
    margin-bottom: 4px;
  }
  .status-value {
    font-size: 26px;
    font-weight: 800;
    letter-spacing: -0.5px;
    color: var(--status-color, var(--normal));
    transition: color 0.5s;
  }
  .status-time {
    font-family: 'Space Mono', monospace;
    font-size: 10px;
    color: var(--muted);
    margin-top: 2px;
  }

  /* Critical: flicker animation on banner */
  .state-critical .status-banner {
    animation: critical-flash 1.2s ease-in-out infinite;
  }
  @keyframes critical-flash {
    0%, 100% { box-shadow: 0 0 0 0 var(--critical-glow); }
    50%       { box-shadow: 0 0 30px 6px var(--critical-glow); }
  }

  /* Risk: subtle pulse */
  .state-risk .status-banner {
    animation: risk-pulse 2s ease-in-out infinite;
  }
  @keyframes risk-pulse {
    0%, 100% { box-shadow: 0 0 0 0 var(--risk-glow); }
    50%       { box-shadow: 0 0 20px 4px var(--risk-glow); }
  }

  /* ── Metric Grid ── */
  .grid {
    position: relative;
    z-index: 1;
    width: 100%;
    max-width: 520px;
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 12px;
    margin-bottom: 16px;
  }

  .card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 14px;
    padding: 18px 16px;
    position: relative;
    overflow: hidden;
    transition: border-color 0.4s, box-shadow 0.4s;
  }
  .card::before {
    content: '';
    position: absolute;
    top: 0; left: 0; right: 0;
    height: 2px;
    background: var(--card-accent, var(--accent));
    border-radius: 2px 2px 0 0;
    opacity: 0.7;
  }
  .card-icon {
    font-size: 20px;
    margin-bottom: 8px;
    display: block;
  }
  .card-label {
    font-family: 'Space Mono', monospace;
    font-size: 10px;
    letter-spacing: 1.5px;
    text-transform: uppercase;
    color: var(--muted);
    margin-bottom: 6px;
  }
  .card-value {
    font-size: 32px;
    font-weight: 800;
    color: white;
    line-height: 1;
    transition: color 0.4s;
  }
  .card-unit {
    font-family: 'Space Mono', monospace;
    font-size: 12px;
    color: var(--muted);
    margin-top: 4px;
  }

  /* Progress bar for SpO2 and BPM */
  .card-bar {
    margin-top: 12px;
    height: 4px;
    background: var(--border);
    border-radius: 2px;
    overflow: hidden;
  }
  .card-bar-fill {
    height: 100%;
    background: var(--card-accent, var(--accent));
    border-radius: 2px;
    transition: width 0.8s cubic-bezier(0.4,0,0.2,1), background 0.4s;
  }

  /* ── Alert Action (Critical only) ── */
  .alert-action {
    position: relative;
    z-index: 1;
    width: 100%;
    max-width: 520px;
    display: none;
    flex-direction: column;
    gap: 10px;
    margin-bottom: 16px;
  }
  .state-critical .alert-action { display: flex; }

  .btn-emergency {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 10px;
    background: var(--critical);
    color: white;
    border: none;
    border-radius: 12px;
    padding: 16px 24px;
    font-family: 'Syne', sans-serif;
    font-size: 16px;
    font-weight: 700;
    cursor: pointer;
    text-decoration: none;
    letter-spacing: 0.3px;
    animation: btn-pulse 1.4s ease-in-out infinite;
    transition: transform 0.1s;
  }
  .btn-emergency:active { transform: scale(0.97); }
  @keyframes btn-pulse {
    0%, 100% { box-shadow: 0 0 0 0 rgba(255,45,85,0.5); }
    50%       { box-shadow: 0 0 0 10px rgba(255,45,85,0); }
  }

  .alert-note {
    font-family: 'Space Mono', monospace;
    font-size: 11px;
    color: var(--critical);
    text-align: center;
    opacity: 0.7;
  }

  /* ── Risk Tips (Risk only) ── */
  .risk-tips {
    position: relative;
    z-index: 1;
    width: 100%;
    max-width: 520px;
    background: rgba(245,166,35,0.06);
    border: 1px solid rgba(245,166,35,0.2);
    border-radius: 14px;
    padding: 16px;
    margin-bottom: 16px;
    display: none;
  }
  .state-risk .risk-tips { display: block; }
  .risk-tips-title {
    font-size: 13px;
    font-weight: 700;
    color: var(--risk);
    margin-bottom: 10px;
    display: flex; align-items: center; gap: 6px;
  }
  .risk-tips ul {
    list-style: none;
    display: flex; flex-direction: column; gap: 6px;
  }
  .risk-tips li {
    font-family: 'Space Mono', monospace;
    font-size: 11px;
    color: var(--text);
    opacity: 0.8;
    display: flex; gap: 8px;
  }
  .risk-tips li::before { content: '→'; color: var(--risk); flex-shrink: 0; }

  /* ── Footer ── */
  .footer {
    position: relative;
    z-index: 1;
    font-family: 'Space Mono', monospace;
    font-size: 10px;
    color: var(--muted);
    margin-top: 8px;
    letter-spacing: 0.5px;
  }
</style>
</head>
<body id="root">

<!-- Header -->
<header class="header">
  <div class="logo">
    <div class="logo-icon">🫁</div>
    <div class="logo-text">Asthma<span> Alert</span></div>
  </div>
  <div class="live-badge">
    <div class="live-dot"></div>
    LIVE
  </div>
</header>

<!-- Status Banner -->
<div class="status-banner" id="statusBanner" style="--status-color: var(--normal); border-color: rgba(0,229,160,0.3)">
  <div class="status-ring" id="statusRing">🟢</div>
  <div class="status-info">
    <div class="status-label">Patient Status</div>
    <div class="status-value" id="statusValue">NORMAL</div>
    <div class="status-time" id="statusTime">--:--:--</div>
  </div>
</div>

<!-- Metric Cards -->
<div class="grid">
  <!-- BPM -->
  <div class="card" style="--card-accent:#ff6b6b">
    <span class="card-icon">💓</span>
    <div class="card-label">Heart Rate</div>
    <div class="card-value" id="bpm">--</div>
    <div class="card-unit">BPM</div>
    <div class="card-bar"><div class="card-bar-fill" id="bpmBar" style="width:0%;background:#ff6b6b"></div></div>
  </div>

  <!-- SpO2 -->
  <div class="card" style="--card-accent:#3d8bff">
    <span class="card-icon">🩸</span>
    <div class="card-label">Blood Oxygen</div>
    <div class="card-value" id="spo2">--</div>
    <div class="card-unit">SpO₂ %</div>
    <div class="card-bar"><div class="card-bar-fill" id="spo2Bar" style="width:0%;background:#3d8bff"></div></div>
  </div>

  <!-- Air Quality -->
  <div class="card" style="--card-accent:#00e5a0">
    <span class="card-icon">💨</span>
    <div class="card-label">Air Quality</div>
    <div class="card-value" id="air">--</div>
    <div class="card-unit">AQI</div>
    <div class="card-bar"><div class="card-bar-fill" id="airBar" style="width:0%;background:#00e5a0"></div></div>
  </div>

  <!-- Body Temp -->
  <div class="card" style="--card-accent:#f5a623">
    <span class="card-icon">🌡️</span>
    <div class="card-label">Body Temp</div>
    <div class="card-value" id="bodyTemp">--</div>
    <div class="card-unit">°C</div>
    <div class="card-bar"><div class="card-bar-fill" id="tempBar" style="width:0%;background:#f5a623"></div></div>
  </div>
</div>

<!-- Risk Tips (visible in RISK state) -->
<div class="risk-tips">
  <div class="risk-tips-title">⚠️ Elevated Risk Detected</div>
  <ul>
    <li>Use your rescue inhaler if prescribed</li>
    <li>Move to a well-ventilated area immediately</li>
    <li>Sit upright and breathe slowly</li>
    <li>Alert a caregiver or nearby person</li>
  </ul>
</div>

<!-- Emergency Action (visible in CRITICAL state) -->
<div class="alert-action">
  <a class="btn-emergency" id="mapsLink" href="#" target="_blank">
    🚨 &nbsp; NAVIGATE TO NEAREST HOSPITAL
  </a>
  <div class="alert-note">⚡ Critical alert — contact emergency services immediately</div>
</div>

<div class="footer">Asthma Alert System v1.0 · ESP32 · Updates every 2s</div>

<script>
  const MAPS_URL = "https://maps.app.goo.gl/Vy91bg2KMgsSKbVgE7";
  let lastStatus = null;

  function formatTime() {
    return new Date().toLocaleTimeString('en-US', { hour12: false });
  }

  function applyState(status) {
    const root = document.getElementById('root');
    const banner = document.getElementById('statusBanner');
    const ring = document.getElementById('statusRing');
    const val = document.getElementById('statusValue');

    root.className = '';
    if (status === 'CRITICAL') {
      root.classList.add('state-critical');
      banner.style.cssText = '--status-color: var(--critical); border-color: rgba(255,45,85,0.4)';
      ring.textContent = '🔴';
      document.getElementById('mapsLink').href = MAPS_URL;
    } else if (status === 'ALERT') {
      root.classList.add('state-risk');
      banner.style.cssText = '--status-color: var(--risk); border-color: rgba(245,166,35,0.35)';
      ring.textContent = '🟡';
    } else {
      root.classList.add('CAUTION');
      banner.style.cssText = '--status-color: var(--normal); border-color: rgba(0,229,160,0.3)';
      ring.textContent = '🟢';
    }
    val.textContent = status;
  }

  function clamp(v, lo, hi) { return Math.max(lo, Math.min(hi, v)); }

  function update() {
    fetch('/data')
      .then(r => r.json())
      .then(d => {
        document.getElementById('bpm').textContent = d.bpm;
        document.getElementById('spo2').textContent = d.spo2;
        document.getElementById('air').textContent = d.air;
        document.getElementById('bodyTemp').textContent = parseFloat(d.bodyTemp).toFixed(1);
        document.getElementById('statusTime').textContent = 'Updated ' + formatTime();

        // Progress bars
        document.getElementById('bpmBar').style.width = clamp((d.bpm / 180) * 100, 0, 100) + '%';
        document.getElementById('spo2Bar').style.width = clamp(d.spo2, 0, 100) + '%';
        document.getElementById('airBar').style.width = clamp(100 - (d.air / 3), 0, 100) + '%';
        document.getElementById('tempBar').style.width = clamp(((d.bodyTemp - 35) / 7) * 100, 0, 100) + '%';

        if (d.status !== lastStatus) {
          applyState(d.status);
          lastStatus = d.status;
        }
      })
      .catch(() => {
        document.getElementById('statusValue').textContent = 'NO SIGNAL';
      });
  }

  // Demo simulation when no ESP32 is connected (for UI preview)
  // Comment out the setInterval below and use this for testing:
  // setTimeout(() => simulateState('CRITICAL'), 3000);
  // setTimeout(() => simulateState('RISK'), 6000);
  // setTimeout(() => simulateState('NORMAL'), 9000);

  function simulateState(st) {
    const mock = { bpm: 95, spo2: 92, air: 120, bodyTemp: 37.8, status: st };
    document.getElementById('bpm').textContent = mock.bpm;
    document.getElementById('spo2').textContent = mock.spo2;
    document.getElementById('air').textContent = mock.air;
    document.getElementById('bodyTemp').textContent = mock.bodyTemp;
    document.getElementById('spo2Bar').style.width = mock.spo2 + '%';
    document.getElementById('bpmBar').style.width = clamp((mock.bpm/180)*100,0,100) + '%';
    document.getElementById('airBar').style.width = clamp(100-(mock.air/3),0,100) + '%';
    document.getElementById('tempBar').style.width = '60%';
    document.getElementById('statusTime').textContent = 'Updated ' + formatTime();
    applyState(st);
  }

  update();
  setInterval(update, 2000);
</script>
</body>
</html>
)====";

  server.send(200, "text/html", page);
}