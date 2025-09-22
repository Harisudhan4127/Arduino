#include <PubSubClient.h>
#include <WiFi.h>

const char* ssid = "IOT@SPAK";
const char* password = "i0t@SP4K";

// MQTT settings
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "csm3313_umt/group09/airquality";
const char* mqtt_led_topic = "csm3313_umt/group09/led01"; // Updated LED control topic

const int MQ135_SENSOR_PIN = 35;
int led1_pin = 12;
int sensitivity = 200;  // Adjust this value based on your calibration

WiFiClient espClient;
PubSubClient client(espClient);

// Deep sleep and awake settings
const int sleepInterval = 3600; // Deep sleep for 1 hour (in seconds)

bool ledState = false;

void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe(mqtt_led_topic); // Subscribe to LED control topic
    } else {
      Serial.print("failed");
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void handleLedControl(String payload) {
  if (payload == "ON") {
    digitalWrite(led1_pin, HIGH);
    ledState = true;
  } else if (payload == "OFF") {
    digitalWrite(led1_pin, LOW);
    ledState = false;
  }
}

void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  pinMode(led1_pin, OUTPUT); // Set LED pin as output

  // Uncomment the next line if you want to see debug output during sleep
  // Serial.setDebugOutput(true);

  // Set up GPIO 16 as wake-up pin
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_16, 0);
}

String interpret_air_quality(int sensor_value) {
  if (sensor_value < 50) {
    return "Excellent";
  } else if (sensor_value < 100) {
    return "Good";
  } else if (sensor_value < 150) {
    return "Moderate";
  } else if (sensor_value < 200) {
    return "Poor";
  } else {
    return "Dangerous";
  }
}

void loop() {
  digitalWrite(led1_pin, HIGH);  // Turn on LED

  unsigned long startMillis = millis();

  while ((millis() - startMillis) < sleepInterval * 1000) {
    if (!client.connected()) {
      reconnect();
    }

    int sensor_value = analogRead(MQ135_SENSOR_PIN);
    int air_quality = sensor_value * sensitivity / 1023;
    String air_quality_label = interpret_air_quality(air_quality);

    char sensor_value_str[6];
    sprintf(sensor_value_str, "%d", air_quality);

    client.publish(mqtt_topic, sensor_value_str);

    // Check for incoming LED control messages
    client.loop();

    Serial.print("Sensor Value: ");
    Serial.println(sensor_value);
    Serial.print("Air Quality Index (Calibrated): ");
    Serial.println(air_quality);
    Serial.print("Air Quality: ");
    Serial.println(air_quality_label);

    delay(2000);
  }

  digitalWrite(led1_pin, LOW);  // Turn off LED
  client.publish(mqtt_led_topic, "OFF");

  client.disconnect();
  delay(1000);

  Serial.flush();  // Flush Serial buffer
  delay(2000);

  esp_deep_sleep(sleepInterval * 1000000);
  Serial.println("Going to sleep...");
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == mqtt_led_topic) {
    handleLedControl(message);
  }
}
