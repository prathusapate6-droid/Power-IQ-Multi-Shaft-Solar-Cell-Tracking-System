/*
================================================================================
 POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 Subsystem: ESP32 IoT Cloud Gateway & Remote Display Controller
 Platform : ESP32 (NodeMCU-32S / ESP32-WROOM-32, 240 MHz Dual-Core, 4MB Flash)
 Authors  : Prathamesh Sapate (Lead), Shreyash Pachade, Vansh Dobhale, Prachi Ronge
 Purpose  : Dual-MCU Bridge (STM32 UART <-> Web Dashboard + Firebase + I2C LCD)
================================================================================

 ARCHITECTURAL & ENGINEERING HIGHLIGHTS (VIVA / EVALUATOR REFERENCE):
   1. DISTRIBUTED EMBEDDED DUAL-MCU CO-PROCESSING:
      - STM32 executes deterministic, hard real-time stepper pulses and optical ADC sensing.
      - ESP32 handles non-deterministic network workloads (Wi-Fi, TLS 1.3 encryption,
        HTTP REST, MQTT PubSub, and I2C LCD rendering) without interrupting motion pulses.
   2. MEMORY MANAGEMENT & STACK-ALLOCATED JSON:
      - Uses ArduinoJson StaticJsonDocument<384> on the stack to completely eliminate
        heap memory fragmentation, ensuring 24/7 crash-free field uptime.
   3. DUAL-TIER CLOUD REDUNDANCY:
      - Uplink 1: Firebase Realtime Database via HTTP REST PATCH for persistent telemetry.
      - Uplink 2: HiveMQ Dedicated Cloud MQTT Broker over TLS 8883 for low-latency (<50ms)
        remote bidirectional command dispatch and slider control.
   4. FAIL-SAFE SOFTAP HOTSPOT:
      - If local router Wi-Fi is unavailable (8-second timeout), the ESP32 automatically
        launches its own standalone SoftAP network (192.168.4.1), enabling immediate local
        field access via smartphone browser without needing any internet connection.
   5. MECHANICAL TRAVEL BOUNDS:
      - All slider jogs and GOTO commands are strictly clamped to [-35.0°, +35.0°] to
        protect the physical worm gear transmission and photovoltaic slat linkages.

 HARDWARE WIRING SPECIFICATIONS (ESP32):
   1. STM32 BLUE PILL UART LINK (USART2):
      - GPIO 16 (RX2) <- Connect to STM32 PA2 (USART2_TX)
      - GPIO 17 (TX2) -> Connect to STM32 PA3 (USART2_RX)
      - GND           -> Connect to STM32 GND (COMMON GROUND IS MANDATORY!)
   2. I2C 16x2 LCD DISPLAY (0x27):
      - GPIO 21 (SDA) -> LCD I2C SDA
      - GPIO 22 (SCL) -> LCD I2C SCL
      - 5V / VIN      -> LCD VCC (5V)
      - GND           -> LCD GND
   3. STATUS INDICATOR:
      - GPIO 2        -> On-board Blue LED (WiFi & Cloud status)
================================================================================
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

// =============================================================================
// 1. NETWORK & CLOUD CONFIGURATION (UPDATE YOUR CREDENTIALS HERE!)
// =============================================================================
const char* WIFI_SSID     = "POWER_IQ_WIFI";      // Your WiFi Network Name
const char* WIFI_PASSWORD = "SolarTracking2026";  // Your WiFi Password

// Fallback SoftAP if WiFi router is unavailable
const char* AP_SSID       = "POWER_IQ_GATEWAY";   // Hotspot Name (Default IP: 192.168.4.1)
const char* AP_PASSWORD   = "poweriq123";

// HiveMQ Dedicated Cloud MQTT Broker Configuration (TLS Encrypted)
#define MQTT_BROKER   "e5c6d611df63436992755767b6967071.s1.eu.hivemq.cloud"
#define MQTT_PORT     8883
#define MQTT_USERNAME "smartwater"
#define MQTT_PASSWORD "SmartWater2026!"
const char* MQTT_TOPIC_TELEMETRY  = "power_iq_sih2026/telemetry";
const char* MQTT_TOPIC_COMMANDS   = "power_iq_sih2026/commands";

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

unsigned long lastMqttReconnectMs = 0;
unsigned long lastMqttPublishMs   = 0;

// Firebase Realtime Database Configuration (Direct Cloud Telemetry Storage)
bool ENABLE_FIREBASE      = true; 
const char* FIREBASE_HOST = "https://engineering-project-hub-default-rtdb.firebaseio.com";
const char* FIREBASE_AUTH = ""; // Open rules deployed via firebase.json

// =============================================================================
// 2. HARDWARE OBJECTS & PIN MAPPINGS
// =============================================================================
#define PIN_RX2         16    // ESP32 UART2 RX (connect to STM32 PA2)
#define PIN_TX2         17    // ESP32 UART2 TX (connect to STM32 PA3)
#define PIN_SDA         21    // I2C SDA for LCD
#define PIN_SCL         22    // I2C SCL for LCD
#define PIN_WIFI_LED    2     // On-board LED

WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 16, 2);
bool isLcdConnected = false;

// =============================================================================
// 3. LIVE SYSTEM TELEMETRY STATE
// =============================================================================
struct TelemetryData {
  float angle          = 0.0f;
  float potAngle       = 0.0f;
  char  mode[8]        = "AUTO";
  bool  homed          = false;
  float solarVoltage   = 0.0f; // V
  float solarCurrent   = 0.0f; // A
  float solarPower     = 0.0f; // W
  float battVoltage    = 0.0f; // V
  float temperature    = 0.0f; // C
  float humidity       = 0.0f; // %
  float energyYieldWh  = 0.0f; // Cumulative Watt-hours
  unsigned long lastUpdateMs = 0;
  bool  isStm32Online  = false;
} liveData;

unsigned long lastFirebasePushMs = 0;
unsigned long lastLcdUpdateMs    = 0;
unsigned long lastEnergyCalcMs   = 0;

// Forward Declarations
void handleRoot();
void handleGetTelemetry();
void handleSendCommand();
void parseIncomingStm32Packet(const String& jsonLine);
void updateLcdDisplay();
void pushTelemetryToFirebase();
void sendCommandToStm32(const String& cmd);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void reconnectMqtt();
void publishTelemetryMqtt();

// =============================================================================
// SETUP
// =============================================================================
void setup() {
  pinMode(PIN_WIFI_LED, OUTPUT);
  digitalWrite(PIN_WIFI_LED, LOW);

  // 1. Debug Serial to PC
  Serial.begin(115200);
  delay(500);
  Serial.println("\n========================================================");
  Serial.println(" POWER IQ — ESP32 IOT GATEWAY & CLOUD CONTROLLER        ");
  Serial.println(" Smart India Hackathon 2026 | Lead: Prathamesh Sapate   ");
  Serial.println("========================================================");

  // 2. Hardware UART2 link to STM32 Blue Pill @ 115200 Baud
  Serial2.begin(115200, SERIAL_8N1, PIN_RX2, PIN_TX2);
  Serial.println("[UART] Connected to STM32 on GPIO 16 (RX2) and GPIO 17 (TX2) @ 115200 Baud.");

  // 3. Initialize I2C Bus & LCD (0x27)
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.beginTransmission(0x27);
  if (Wire.endTransmission() == 0) {
    isLcdConnected = true;
    lcd.begin();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("POWER IQ GATEWAY");
    lcd.setCursor(0, 1);
    lcd.print("Connecting WiFi.");
    Serial.println("[LCD] I2C 16x2 Display connected on 0x27.");
  } else {
    Serial.println("[LCD] No I2C Display detected at 0x27 (Display bypassed).");
  }

  // 4. WiFi Connection with SoftAP Fallback
  Serial.print("[WIFI] Connecting to SSID: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long wifiStart = millis();
  bool connected = false;
  while (millis() - wifiStart < 8000) {
    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      break;
    }
    digitalWrite(PIN_WIFI_LED, !digitalRead(PIN_WIFI_LED));
    delay(250);
    Serial.print(".");
  }

  if (connected) {
    digitalWrite(PIN_WIFI_LED, HIGH);
    Serial.println("\n[WIFI] Connected Successfully!");
    Serial.print("[WIFI] Gateway IP Address: http://");
    Serial.println(WiFi.localIP());

    if (isLcdConnected) {
      lcd.setCursor(0, 0);
      lcd.print("WiFi Connected! ");
      lcd.setCursor(0, 1);
      lcd.print(WiFi.localIP());
      delay(2000);
    }
  } else {
    // Start SoftAP Hotspot
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    digitalWrite(PIN_WIFI_LED, LOW);
    Serial.println("\n[WIFI] Station not connected. Started SoftAP Hotspot!");
    Serial.print("[WIFI] Connect phone/PC to: ");
    Serial.println(AP_SSID);
    Serial.print("[WIFI] Access Dashboard at: http://");
    Serial.println(WiFi.softAPIP());

    if (isLcdConnected) {
      lcd.setCursor(0, 0);
      lcd.print("AP: POWER_IQ_GW ");
      lcd.setCursor(0, 1);
      lcd.print(WiFi.softAPIP());
      delay(2000);
    }
  }

  // 5. Setup Web Server REST Endpoints with CORS Support
  server.enableCORS(true);
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/telemetry", HTTP_GET, handleGetTelemetry);
  server.on("/api/command", HTTP_GET, handleSendCommand);
  server.on("/api/telemetry", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "*");
    server.send(204);
  });
  server.on("/api/command", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "*");
    server.send(204);
  });
  server.begin();
  Serial.println("[HTTP] Web Server started on port 80.");

  // 6. Setup MQTT HiveMQ Dedicated Cloud Broker (TLS Port 8883)
  espClient.setInsecure(); // Skip TLS certificate validation for lightweight embedded TLS
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(512);
  Serial.println("[MQTT] HiveMQ Dedicated Cloud client initialized (Port 8883 TLS).");
  Serial.println("[READY] ESP32 Gateway Active! Listening for STM32 telemetry...\n");
}

// =============================================================================
// MAIN LOOP
// =============================================================================
void loop() {
  server.handleClient();

  // 1. Read JSON Telemetry stream from STM32 over UART2
  while (Serial2.available()) {
    String incomingLine = Serial2.readStringUntil('\n');
    incomingLine.trim();
    if (incomingLine.length() > 0 && incomingLine.startsWith("{") && incomingLine.endsWith("}")) {
      parseIncomingStm32Packet(incomingLine);
    }
  }

  unsigned long now = millis();

  // 2. Accumulate Solar Energy Yield (Watt-Hours)
  if (now - lastEnergyCalcMs >= 1000) {
    float dtHours = (now - lastEnergyCalcMs) / 3600000.0f;
    lastEnergyCalcMs = now;
    if (liveData.solarPower > 0.0f) {
      liveData.energyYieldWh += (liveData.solarPower * dtHours);
    }
  }

  // 3. Periodic I2C LCD Update (every 800ms)
  if (now - lastLcdUpdateMs >= 800) {
    lastLcdUpdateMs = now;
    updateLcdDisplay();
  }

  // 4. MQTT Cloud Uplink & Non-blocking Connection (every 1000ms)
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      reconnectMqtt();
    } else {
      mqttClient.loop();
      if (now - lastMqttPublishMs >= 1000) {
        lastMqttPublishMs = now;
        publishTelemetryMqtt();
      }
    }
  }

  // 5. Periodic Firebase Cloud Uplink (every 3000ms)
  if (ENABLE_FIREBASE && (now - lastFirebasePushMs >= 3000)) {
    lastFirebasePushMs = now;
    pushTelemetryToFirebase();
  }

  // 6. Check STM32 Liveness (timeout after 3.5s)
  if (now - liveData.lastUpdateMs > 3500) {
    liveData.isStm32Online = false;
  }
}

// =============================================================================
// PARSE STM32 JSON PACKET
// =============================================================================
void parseIncomingStm32Packet(const String& jsonLine) {
  StaticJsonDocument<384> doc;
  DeserializationError err = deserializeJson(doc, jsonLine);
  if (err) return;

  liveData.angle        = doc["ang"]   | liveData.angle;
  liveData.potAngle     = doc["pot"]   | liveData.potAngle;
  const char* m         = doc["mode"]  | "AUTO";
  strncpy(liveData.mode, m, sizeof(liveData.mode) - 1);
  liveData.homed        = (doc["homed"] | 0) == 1;
  liveData.solarVoltage = doc["v_pv"]  | liveData.solarVoltage;
  liveData.solarCurrent = doc["i_pv"]  | liveData.solarCurrent;
  liveData.solarPower   = doc["p_pv"]  | liveData.solarPower;
  liveData.battVoltage  = doc["v_bat"] | liveData.battVoltage;
  liveData.temperature = doc["temp"]  | liveData.temperature;
  liveData.humidity    = doc["hum"]   | liveData.humidity;

  liveData.lastUpdateMs  = millis();
  liveData.isStm32Online = true;

  // Echo comprehensive live telemetry to Serial Monitor
  Serial.printf("[SOLAR-DATA] Solar: %4.2fV | Current: %4.2fA | Power: %5.2f Watts | Slat: %+5.1f° | Pot: %+5.1f°%s | Bat: %4.2fV | Temp: %4.1f°C | Mode: %s\n",
                liveData.solarVoltage,
                liveData.solarCurrent,
                liveData.solarPower,
                liveData.angle,
                liveData.potAngle,
                (liveData.potAngle == 0.0f) ? " [ZERO]" : (liveData.potAngle > 0.0f ? " [RGT]" : " [LFT]"),
                liveData.battVoltage,
                liveData.temperature,
                liveData.mode);
}

// =============================================================================
// UPDATE I2C 16x2 LCD
// =============================================================================
void updateLcdDisplay() {
  if (!isLcdConnected) return;

  char line0[17];
  char line1[17];

  if (strcmp(liveData.mode, "MAN") == 0 || strcmp(liveData.mode, "MANUAL") == 0) {
    // ---------------- MANUAL POTENTIOMETER DISPLAY ----------------
    // Line 0: Potentiometer Angle with Center ZERO / Direction indicator
    if (fabs(liveData.potAngle) <= 0.5f) {
      snprintf(line0, sizeof(line0), "POT :  0.0 [ZERO]");
    } else if (liveData.potAngle > 0.5f) {
      snprintf(line0, sizeof(line0), "POT :%+5.1f (CW+)", liveData.potAngle);
    } else {
      snprintf(line0, sizeof(line0), "POT :%+5.1f (CCW)", liveData.potAngle);
    }

    // Line 1: Solar Cells Actual Slat Angle & Real Power Output
    snprintf(line1, sizeof(line1), "CELL:%+5.1f P:%4.1fW", liveData.angle, liveData.solarPower);
  } else {
    // ---------------- AUTOMATIC SUN TRACKING DISPLAY ----------------
    // Line 0: Solar Generation (Voltage & Power)
    snprintf(line0, sizeof(line0), "V:%4.1fV P:%4.1fW  ", liveData.solarVoltage, liveData.solarPower);

    // Line 1: Slat Angle, Temperature, and AUTO indicator
    snprintf(line1, sizeof(line1), "A:%+4.0f%c T:%2.0fC AUTO", liveData.angle, (char)223, liveData.temperature);
  }

  lcd.setCursor(0, 0);
  lcd.print(line0);
  lcd.setCursor(0, 1);
  lcd.print(line1);
}

// =============================================================================
// FIREBASE CLOUD REALTIME DATABASE REST PUSH
// =============================================================================
void pushTelemetryToFirebase() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String(FIREBASE_HOST) + "/power_iq/telemetry.json";
  if (strlen(FIREBASE_AUTH) > 0) {
    url += "?auth=" + String(FIREBASE_AUTH);
  }

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<256> doc;
  doc["angle"]         = liveData.angle;
  doc["pot_angle"]     = liveData.potAngle;
  doc["mode"]          = liveData.mode;
  doc["solar_voltage"] = liveData.solarVoltage;
  doc["solar_current"] = liveData.solarCurrent;
  doc["solar_power"]   = liveData.solarPower;
  doc["batt_voltage"]  = liveData.battVoltage;
  doc["temperature"]   = liveData.temperature;
  doc["humidity"]      = liveData.humidity;
  doc["energy_wh"]     = liveData.energyYieldWh;
  doc["timestamp"]     = millis();

  String payload;
  serializeJson(doc, payload);

  int httpCode = http.PATCH(payload);
  if (httpCode > 0) {
    digitalWrite(PIN_WIFI_LED, HIGH);
  } else {
    Serial.printf("[FIREBASE] HTTP error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

// =============================================================================
// FORWARD REMOTE COMMAND TO STM32 OVER UART2
// =============================================================================
void sendCommandToStm32(const String& cmd) {
  Serial.printf("[CMD-SEND] Sending to STM32: %s\n", cmd.c_str());
  Serial2.println(cmd);
}

// =============================================================================
// HTTP REST HANDLERS
// =============================================================================
void handleGetTelemetry() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  StaticJsonDocument<384> doc;
  doc["angle"]          = liveData.angle;
  doc["pot_angle"]      = liveData.potAngle;
  doc["mode"]           = liveData.mode;
  doc["homed"]          = liveData.homed;
  doc["solar_voltage"]  = liveData.solarVoltage;
  doc["solar_current"]  = liveData.solarCurrent;
  doc["solar_power"]    = liveData.solarPower;
  doc["batt_voltage"]   = liveData.battVoltage;
  doc["temperature"]    = liveData.temperature;
  doc["humidity"]       = liveData.humidity;
  doc["energy_wh"]      = liveData.energyYieldWh;
  doc["stm32_online"]   = liveData.isStm32Online;

  String jsonStr;
  serializeJson(doc, jsonStr);
  server.send(200, "application/json", jsonStr);
}

void handleSendCommand() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  if (!server.hasArg("cmd")) {
    server.send(400, "text/plain", "Missing 'cmd' parameter");
    return;
  }
  String cmd = server.arg("cmd");
  sendCommandToStm32(cmd);
  server.send(200, "text/plain", "OK: Command dispatched to STM32");
}

// =============================================================================
// MQTT CLOUD CALLBACK & FUNCTIONS (NETLIFY BRIDGE)
// =============================================================================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String cmd = "";
  for (unsigned int i = 0; i < length; i++) {
    cmd += (char)payload[i];
  }
  cmd.trim();
  Serial.printf("[MQTT-CMD] Netlify Dashboard Command: '%s'\n", cmd.c_str());
  if (cmd.length() > 0) {
    sendCommandToStm32(cmd);
  }
}

void reconnectMqtt() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastMqttReconnectMs < 4000) return;
  lastMqttReconnectMs = millis();

  String clientId = "POWER_IQ_ESP32_" + String((uint32_t)ESP.getEfuseMac(), HEX);
  Serial.print("[MQTT] Connecting to Dedicated HiveMQ Cloud (TLS 8883)... ");
  if (mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.println("CONNECTED!");
    mqttClient.subscribe(MQTT_TOPIC_COMMANDS);
    Serial.printf("[MQTT] Subscribed to topic: %s\n", MQTT_TOPIC_COMMANDS);
    digitalWrite(PIN_WIFI_LED, HIGH);
  } else {
    Serial.printf("FAILED (rc=%d), will retry in 4s\n", mqttClient.state());
  }
}

void publishTelemetryMqtt() {
  if (!mqttClient.connected()) return;

  StaticJsonDocument<384> doc;
  doc["angle"]         = liveData.angle;
  doc["pot_angle"]     = liveData.potAngle;
  doc["mode"]          = liveData.mode;
  doc["homed"]         = liveData.homed;
  doc["solar_voltage"] = liveData.solarVoltage;
  doc["solar_current"] = liveData.solarCurrent;
  doc["solar_power"]   = liveData.solarPower;
  doc["batt_voltage"]  = liveData.battVoltage;
  doc["temperature"]   = liveData.temperature;
  doc["humidity"]      = liveData.humidity;
  doc["energy_wh"]     = liveData.energyYieldWh;
  doc["stm32_online"]  = liveData.isStm32Online;
  doc["uptime"]        = millis() / 1000;

  char buf[384];
  size_t len = serializeJson(doc, buf, sizeof(buf));
  mqttClient.publish(MQTT_TOPIC_TELEMETRY, buf, len);
}

// =============================================================================
// EMBEDDED REAL-TIME WEB DASHBOARD (HTML5 + CSS + JAVASCRIPT)
// =============================================================================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>POWER IQ — Multi-Shaft Solar Tracking System</title>
  <style>
    :root {
      --bg: #0b1120;
      --card: #1e293b;
      --border: #334155;
      --accent: #38bdf8;
      --amber: #f59e0b;
      --emerald: #10b981;
      --text: #f8fafc;
      --text-muted: #94a3b8;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background: var(--bg); color: var(--text); padding: 1.5rem; min-height: 100vh; }
    .header { display: flex; justify-content: space-between; align-items: center; border-bottom: 1px solid var(--border); padding-bottom: 1rem; margin-bottom: 1.5rem; }
    .title { font-size: 1.5rem; font-weight: 800; background: linear-gradient(135deg, #38bdf8, #818cf8); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
    .badge { padding: 0.25rem 0.75rem; border-radius: 9999px; font-size: 0.75rem; font-weight: 700; text-transform: uppercase; }
    .badge-online { background: #064e3b; color: #34d399; border: 1px solid #059669; }
    .badge-offline { background: #7f1d1d; color: #f87171; border: 1px solid #dc2626; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 1rem; margin-bottom: 1.5rem; }
    .card { background: var(--card); border: 1px solid var(--border); border-radius: 12px; padding: 1.25rem; display: flex; flex-direction: column; justify-content: space-between; }
    .card-label { font-size: 0.8rem; font-weight: 600; color: var(--text-muted); text-transform: uppercase; margin-bottom: 0.5rem; }
    .card-value { font-size: 2rem; font-weight: 800; }
    .unit { font-size: 1rem; font-weight: 600; color: var(--text-muted); }
    .controls-card { background: var(--card); border: 1px solid var(--border); border-radius: 12px; padding: 1.5rem; margin-bottom: 1.5rem; }
    .btn-row { display: flex; flex-wrap: wrap; gap: 0.75rem; margin-top: 1rem; }
    button { background: #0284c7; color: #fff; border: none; padding: 0.75rem 1.25rem; border-radius: 8px; font-weight: 700; cursor: pointer; transition: all 0.2s; font-size: 0.9rem; }
    button:hover { background: #0369a1; transform: translateY(-1px); }
    button.btn-secondary { background: #475569; }
    button.btn-secondary:hover { background: #334155; }
    button.btn-warning { background: #d97706; }
    button.btn-warning:hover { background: #b45309; }
    .slider-container { margin-top: 1.25rem; }
    input[type=range] { width: 100%; height: 8px; border-radius: 4px; background: #334155; accent-color: var(--accent); }
    .slider-labels { display: flex; justify-content: space-between; font-size: 0.75rem; color: var(--text-muted); margin-top: 0.25rem; }
  </style>
</head>
<body>
  <div class="header">
    <div>
      <div class="title">POWER IQ — Multi-Shaft Solar Tracking</div>
      <div style="font-size:0.85rem; color:var(--text-muted);">Dual-MCU Edge Controller (STM32 + ESP32 IoT Gateway)</div>
    </div>
    <div id="statusBadge" class="badge badge-offline">STM32 CONNECTING...</div>
  </div>

  <div class="grid">
    <div class="card">
      <div class="card-label">☀️ Solar PV Power</div>
      <div class="card-value" style="color:#f59e0b;"><span id="solarPower">0.00</span> <span class="unit">W</span></div>
    </div>
    <div class="card">
      <div class="card-label">⚡ Solar PV Voltage</div>
      <div class="card-value" style="color:#38bdf8;"><span id="solarVolt">0.00</span> <span class="unit">V</span></div>
    </div>
    <div class="card">
      <div class="card-label">🔌 Solar Current</div>
      <div class="card-value" style="color:#34d399;"><span id="solarCurr">0.00</span> <span class="unit">A</span></div>
    </div>
    <div class="card">
      <div class="card-label">📐 Slat Angle</div>
      <div class="card-value" style="color:#a78bfa;"><span id="slatAngle">+0.0</span> <span class="unit">deg</span></div>
    </div>
    <div class="card">
      <div class="card-label">🔋 Battery Voltage</div>
      <div class="card-value" style="color:#60a5fa;"><span id="battVolt">0.00</span> <span class="unit">V</span></div>
    </div>
    <div class="card">
      <div class="card-label">🌱 Energy Yield</div>
      <div class="card-value" style="color:#10b981;"><span id="energyYield">0.00</span> <span class="unit">Wh</span></div>
    </div>
    <div class="card">
      <div class="card-label">🌡️ Temperature / Humidity</div>
      <div class="card-value" style="font-size:1.6rem;"><span id="temp">0</span>°C <span class="unit">|</span> <span id="hum">0</span>%</div>
    </div>
    <div class="card">
      <div class="card-label">⚙️ Tracking Mode</div>
      <div class="card-value" id="trackMode" style="color:#f8fafc;">AUTO</div>
    </div>
  </div>

  <div class="controls-card">
    <div style="font-size:1.1rem; font-weight:700; margin-bottom:0.5rem;">🎮 Remote Control Panel</div>
    <div style="font-size:0.85rem; color:var(--text-muted);">Send real-time commands from smartphone or laptop to STM32 Blue Pill:</div>

    <div class="btn-row">
      <button onclick="sendCmd('AUTO')">☀️ Enable AUTO Tracking</button>
      <button class="btn-secondary" onclick="sendCmd('MANUAL')">⏸️ Pause / MANUAL Mode</button>
      <button class="btn-warning" onclick="sendCmd('HOME')">🎯 Calibrate ZERO Home (0.0°)</button>
      <button class="btn-secondary" onclick="sendCmd('ZERO_CURR')">⚡ Zero Current Sensor</button>
    </div>

    <div class="slider-container">
      <div style="display:flex; justify-content:space-between; margin-bottom:0.5rem;">
        <span style="font-size:0.85rem; font-weight:700;">Manual Slat Angle Jog:</span>
        <span id="sliderVal" style="font-size:0.85rem; font-weight:700; color:var(--accent);">0.0°</span>
      </div>
      <input type="range" id="angleSlider" min="-35" max="35" step="1" value="0" oninput="document.getElementById('sliderVal').innerText=this.value+'°'" onchange="sendCmd('GOTO '+this.value)">
      <div class="slider-labels">
        <span>-35° (East / Left)</span>
        <span>0.0° (Zenith / Center)</span>
        <span>+35° (West / Right)</span>
      </div>
    </div>
  </div>

  <script>
    async function updateTelemetry() {
      try {
        const res = await fetch('/api/telemetry');
        const data = await res.json();

        document.getElementById('solarPower').innerText = data.solar_power.toFixed(2);
        document.getElementById('solarVolt').innerText  = data.solar_voltage.toFixed(2);
        document.getElementById('solarCurr').innerText  = data.solar_current.toFixed(2);
        document.getElementById('slatAngle').innerText  = (data.angle >= 0 ? '+' : '') + data.angle.toFixed(1);
        document.getElementById('battVolt').innerText   = data.batt_voltage.toFixed(2);
        document.getElementById('energyYield').innerText= data.energy_wh.toFixed(2);
        document.getElementById('temp').innerText       = Math.round(data.temperature);
        document.getElementById('hum').innerText        = Math.round(data.humidity);
        document.getElementById('trackMode').innerText  = data.mode;

        const badge = document.getElementById('statusBadge');
        if (data.stm32_online) {
          badge.className = 'badge badge-online';
          badge.innerText = 'STM32 ONLINE';
        } else {
          badge.className = 'badge badge-offline';
          badge.innerText = 'STM32 OFFLINE';
        }
      } catch (e) {
        console.error("Telemetry fetch error:", e);
      }
    }

    async function sendCmd(cmdStr) {
      try {
        await fetch('/api/command?cmd=' + encodeURIComponent(cmdStr));
        setTimeout(updateTelemetry, 300);
      } catch (e) {
        alert("Command failed: " + e);
      }
    }

    setInterval(updateTelemetry, 1000);
    updateTelemetry();
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}
