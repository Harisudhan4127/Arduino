#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "Pathology Lab Robot";
const char* password = "admin@123";

ESP8266WebServer server(80);

// ---------------- ROBOT STATE ----------------
String mode = "AUTO";        // AUTO or MANUAL
int robotX = 50;
int robotY = 120;

// ---------------- HTML PAGE ----------------
String page() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Pathology Lab Robot</title>
<style>
body{margin:0;font-family:Arial;background:#0b1c2d;color:#e0f2fe;text-align:center}
header{background:#00b4d8;padding:14px;font-size:22px}
.card{background:#122040;margin:14px;padding:14px;border-radius:16px}
button{padding:10px 18px;margin:6px;border:none;border-radius:10px;background:#00b4d8;color:#001219;font-size:15px}
#map{position:relative;width:260px;height:160px;margin:auto;background:#081a33;border-radius:12px}
#robot{position:absolute;width:14px;height:14px;border-radius:50%;background:#00ff9c;left:50px;top:120px}
</style>

<script>
let mode = "AUTO";

function setMode(m){
  mode = m;
  fetch('/mode?m='+m);
  document.getElementById("m").innerText = m;
}

function send(cmd){
  fetch('/cmd?c='+cmd);
}

function move(x,y){
  let r = document.getElementById("robot");
  r.style.left = x+"px";
  r.style.top  = y+"px";
}
</script>
</head>

<body>
<header>Pathology Lab Robot</header>

<div class="card">
  <b>Mode:</b> <span id="m">AUTO</span><br><br>
  <button onclick="setMode('AUTO')">AUTO</button>
  <button onclick="setMode('MANUAL')">MANUAL</button>
</div>

<div class="card">
  <h3>Live Map</h3>
  <div id="map">
    <div id="robot"></div>
  </div>
</div>

<div class="card">
  <h3>Manual Control</h3>
  <button onclick="send('F')">F</button><br>
  <button onclick="send('L')">L</button>
  <button onclick="send('S')">S</button>
  <button onclick="send('R')">R</button><br>
  <button onclick="send('B')">B</button>
</div>

<div class="card">
  <h3>Mapped Travel</h3>
  <button onclick="send('1');move(50,20)">Collection</button>
  <button onclick="send('2');move(200,20)">Testing</button>
  <button onclick="send('3');move(200,120)">Storage</button>
</div>

</body>
</html>
)rawliteral";
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(9600);
  WiFi.softAP(ssid, password);

  Serial.println("ESP8266 Started");
  Serial.println("WiFi AP Created: Pathology Lab Robot");

  server.on("/", [](){
    Serial.println("Client Connected");
    server.send(200,"text/html",page());
  });

  // MODE CHANGE
  server.on("/mode", [](){
    if(server.hasArg("m")){
      mode = server.arg("m");
      Serial.print("MODE CHANGED: ");
      Serial.println(mode);
      Serial.print("M");

    }
    server.send(200,"text/plain","OK");
  });

  // COMMAND HANDLER
  server.on("/cmd", [](){
    if(server.hasArg("c")){
      String cmd = server.arg("c");

      // Manual commands allowed only in MANUAL mode
      if(mode == "MANUAL" || cmd == "1" || cmd == "2" || cmd == "3"){
        // Serial.print("CMD SENT: ");
        // Serial.println(cmd);
        Serial.print(cmd);   // Send to Arduino

        // Update location logically
        if(cmd == "F") robotY -= 5;
        if(cmd == "B") robotY += 5;
        if(cmd == "L") robotX -= 5;
        if(cmd == "R") robotX += 5;

        // Serial.print("\nLocation: ");
        // Serial.print(robotX);
        // Serial.print(",");
        // Serial.println(robotY);
      }
    }
    server.send(200,"text/plain","OK");
  });

  server.begin();
  Serial.println("Web Server Started");
}

// ---------------- LOOP ----------------
void loop() {
  server.handleClient();
}
