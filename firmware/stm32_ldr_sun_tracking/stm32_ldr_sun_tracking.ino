/*
================================================================================
 POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 Subsystem: STM32 Sun Tracking + Manual Pot/Btn + Solar Power (V+I+P) + ESP32
 Platform : STM32F103C8T6 "Blue Pill" (32-bit ARM Cortex-M3 @ 72 MHz)
 Authors  : Prathamesh Sapate (Lead), Shreyash Pachade, Vansh Dobhale, Prachi Ronge
 Purpose  : 100% Pure STM32 Firmware (Dual UART: PC Debug + ESP32 Link)
================================================================================

 HARDWARE WIRING SPECIFICATIONS (STM32 BLUE PILL):
   1. A4988 STEPPER DRIVER:
      - PB8   -> A4988 STEP (Pulse)
      - PB9   -> A4988 DIR  (Direction)
      - PB10  -> A4988 ENABLE (Active-LOW: 0=Moving, 1=Silent 0W Sleep)
   2. ZERO DATUM HOME SENSOR:
      - PB11  -> Hall Effect 0.0° Home Sensor (Active-LOW)
   3. 4-QUADRANT LDR SUN SENSORS:
      - PA0   -> Top Sector Sensor 1 (ADC1_IN0)
      - PA1   -> Top Sector Sensor 2 (ADC1_IN1)
      - PA4   -> Bottom Sector Sensor 1 (ADC1_IN4)
      - PA5   -> Bottom Sector Sensor 2 (ADC1_IN5)
   4. MANUAL CONTROLS:
      - PB0   -> 10k Potentiometer Wiper (ADC1_IN8) [-40° to +40° Manual Angle]
      - PB12  -> Auto/Manual Mode Push Button (Active-LOW with Internal Pullup)
   5. SOLAR PV POWER MONITORING (Voltage + Current):
      - PA6   -> Solar PV Voltage (ADC1_IN6) [R1=33k, R2=6.8k divider]
      - PA7   -> Solar PV Current (ADC1_IN7) [ACS712 Sensor Out]
   6. BATTERY MONITORING:
      - PB1   -> Battery Voltage (ADC1_IN9) [Voltage divider]
   7. ENVIRONMENT SENSOR:
      - PB5   -> DHT11 Data Pin
   8. DUAL HARDWARE UART PORTS:
      - PA9   -> USART1_TX (PC USB-TTL RX) @ 9600 Baud
      - PA10  -> USART1_RX (PC USB-TTL TX) @ 9600 Baud
      - PA2   -> USART2_TX (ESP32 RX2 - GPIO 16) @ 9600 Baud
      - PA3   -> USART2_RX (ESP32 TX2 - GPIO 17) @ 9600 Baud
   9. VISUAL STATUS:
      - PC13  -> On-board LED (Active-LOW, Heartbeat Pulse)
================================================================================
*/

#include <Arduino.h>

// =============================================================================
// 1. USER SPEED & TIMING VARIABLES
// =============================================================================
int ZERO_HOMING_SPEED_US = 2500;  // Speed during 0.0 deg ZERO search (default: 2500 us)
int TRACKING_SPEED_US = 3500;     // Speed during Sun Tracking motion (default: 2000 us)

// ---------------- GPIO PIN DEFINITIONS (STM32 BLUE PILL) ----------------
#define PIN_STEP PB8         // A4988 STEP pulse
#define PIN_DIR PB9          // A4988 DIR direction
#define PIN_ENABLE PB10      // A4988 ENABLE (Active LOW)
#define PIN_STATUS_LED PC13  // Onboard LED (Active LOW)

#define PIN_LDR_TOP1 PA0  // Top Sector Sensor 1
#define PIN_LDR_TOP2 PA1  // Top Sector Sensor 2
#define PIN_LDR_BOT1 PA4  // Bottom Sector Sensor 1
#define PIN_LDR_BOT2 PA5  // Bottom Sector Sensor 2

#define PIN_HALL_HOME PB11  // Hall-effect Home Sensor (Active LOW)
#define PIN_DHT11 PB5       // DHT11 Data Pin

#define PIN_POT_MANUAL   PB0   // 10k Potentiometer (ADC1_IN8)
#define PIN_SW_MODE      PB12  // 2-Position ON/OFF Switch Primary (Active LOW to GND)
#define PIN_SW_MODE_ALT  PB13  // 2-Position ON/OFF Switch Alternate (Active LOW to GND)
#define PIN_BTN_MODE     PIN_SW_MODE
#define PIN_BTN_MODE_ALT PIN_SW_MODE_ALT

// ---------------- POWER MONITORING PINS (SOLAR & BATTERY) ----------------
#define PIN_SOLAR_VOLT PA6  // Solar Voltage Analog Input
#define PIN_SOLAR_CURR PA7  // Solar Current Analog Input
#define PIN_BATT_VOLT PB1   // Battery Voltage Analog Input

const float VOLT_DIVIDER_RATIO = (33000.0f + 6800.0f) / 6800.0f;  // 5.8529
float refVoltage = 3.3f;                                          // STM32 ADC Reference Voltage

float currentSensitivity = 0.100f;  // 100 mV/A (ACS712-20A)
float currentZeroOffset = 2.50f;    // Auto-calibrated at boot (Nominal 2.5V for 5V ACS712)

float solarVoltage = 0.0f;  // Volts
float solarCurrent = 0.0f;  // Amps
float solarPower = 0.0f;    // Watts
float battVoltage = 0.0f;   // Volts
unsigned long lastPowerReadTime = 0;

// ---------------- DHT11 ENVIRONMENT ----------------
float currentTemp = 0.0f;      // deg C
float currentHumidity = 0.0f;  // %
unsigned long lastDhtReadTime = 0;

// ---------------- KINEMATICS & BENCH CONSTANTS ----------------
// ---------------- KINEMATICS & BENCH CONSTANTS ----------------
const float STEPS_PER_DEGREE = 10.556f;  // 19:1 Worm gear ratio
const float MIN_ANGLE = -45.0f;          // Hard Mechanical Structural Limit (-45.0 deg)
const float MAX_ANGLE = 45.0f;           // Hard Mechanical Structural Limit (+45.0 deg)

int deadbandThreshold = 50;
int nightDarkThreshold = 40;
const unsigned long NIGHT_PARK_DELAY_MS = 8000;

bool isAutoTracking = true;
bool invertMotorDir = false;
float currentAngle = 0.0f;
bool isHomed = false;
unsigned long lastTrackTime = 0;
unsigned long darknessStartMs = 0;
unsigned long frameCount = 0;
unsigned long lastHeartbeatTime = 0;
bool heartbeatState = false;
unsigned long lastTelemetryTime = 0;

// ---------------- HARDWARE ON/OFF MODE SWITCH & POTENTIOMETER ----------------
bool switchInvert = false;         // false: Switch ON (LOW to GND) = AUTO, OFF (HIGH) = MANUAL
bool potInvert = false;            // false: Clockwise = Positive (+), Anti-Clockwise = Negative (-)
int lastStableSwitchState = -1;    // -1 = uninitialized, 1 = ON (Closed to GND), 0 = OFF (Open)
int lastRawSwitchState = -1;
unsigned long lastSwitchDebounceMs = 0;
float lastPotTargetAngle = 0.0f;
float currentPotAngle = 0.0f;

int lastTopVal = 0;
int lastBotVal = 0;
int lastDiffVal = 0;
const char* lastTrackingStatus = "INITIALIZING";

// ---------------- DUAL-SERIAL OUTPUT HELPERS ----------------
// Serial1: Human-readable PC Telemetry on PA9/PA10 @ 115200 Baud
// Serial2: High-speed JSON Stream to ESP32 on PA2/PA3 @ 115200 Baud

template<typename T>
void printAll(T msg) {
  Serial1.print(msg);
}

template<typename T, typename P>
void printAll(T msg, P p) {
  Serial1.print(msg, p);
}

inline void printlnAll() {
  Serial1.println();
}

template<typename T>
void printlnAll(T msg) {
  Serial1.println(msg);
}

template<typename T, typename P>
void printlnAll(T msg, P p) {
  Serial1.println(msg, p);
}

inline void printBar() {
  Serial1.println(F("----------------------------------------"));
}

// Compact JSON Telemetry Packet to ESP32 over USART2 (PA2/PA3)
void sendEsp32JsonTelemetry() {
  Serial2.print(F("{\"ang\":"));
  Serial2.print(currentAngle, 1);
  Serial2.print(F(",\"pot\":"));
  Serial2.print(currentPotAngle, 1);
  Serial2.print(F(",\"mode\":\""));
  Serial2.print(isAutoTracking ? "AUTO" : "MAN");
  Serial2.print(F("\",\"homed\":"));
  Serial2.print(isHomed ? 1 : 0);
  Serial2.print(F(",\"v_pv\":"));
  Serial2.print(solarVoltage, 2);
  Serial2.print(F(",\"i_pv\":"));
  Serial2.print(solarCurrent, 2);
  Serial2.print(F(",\"p_pv\":"));
  Serial2.print(solarPower, 2);
  Serial2.print(F(",\"v_bat\":"));
  Serial2.print(battVoltage, 2);
  Serial2.print(F(",\"temp\":"));
  Serial2.print(currentTemp, 1);
  Serial2.print(F(",\"hum\":"));
  Serial2.print(currentHumidity, 1);
  Serial2.println(F("}"));
}

// ---------------- LIGHTWEIGHT PARSERS ----------------
float parseCustomFloat(const char* p) {
  while (*p == ' ') p++;
  float sign = 1.0f;
  if (*p == '-') {
    sign = -1.0f;
    p++;
  } else if (*p == '+') {
    p++;
  }
  float val = 0.0f;
  while (*p >= '0' && *p <= '9') {
    val = val * 10.0f + (*p - '0');
    p++;
  }
  if (*p == '.') {
    p++;
    float factor = 0.1f;
    while (*p >= '0' && *p <= '9') {
      val += (*p - '0') * factor;
      factor *= 0.1f;
      p++;
    }
  }
  return sign * val;
}

// ---------------- ULTRA-ROBUST CYCLE-COUNTING DHT11 / DHT22 READER ----------------
// Immune to STM32 SysTick overflow during noInterrupts()
// Uses ratio of pulse widths (highCycles vs lowCycles) for guaranteed decoding
bool readDHT11(uint8_t pin, float& temp, float& humidity) {
  uint8_t data[5] = { 0, 0, 0, 0, 0 };

  // 1. Host Start Signal: Pull bus LOW for 20ms
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(20);

  // 2. Release bus and configure as INPUT_PULLUP with interrupts locked
  noInterrupts();
  pinMode(pin, INPUT_PULLUP);
  delayMicroseconds(30);

  // 3. Timing-critical pulse measurement (Timeout ~250us @ 72MHz)
  const uint32_t MAX_CYCLES = 12000;
  uint32_t cycles[80];

  // Wait for sensor to pull line LOW (acknowledge start signal)
  uint32_t count = 0;
  while (digitalRead(pin) == HIGH) {
    if (++count >= MAX_CYCLES) {
      interrupts();
      return false;
    }
  }

  // Sensor response: 80us LOW
  count = 0;
  while (digitalRead(pin) == LOW) {
    if (++count >= MAX_CYCLES) {
      interrupts();
      return false;
    }
  }

  // Sensor response: 80us HIGH
  count = 0;
  while (digitalRead(pin) == HIGH) {
    if (++count >= MAX_CYCLES) {
      interrupts();
      return false;
    }
  }

  // Read 40 data bits (80 alternating pulses: 40 LOW + 40 HIGH)
  for (int i = 0; i < 80; i += 2) {
    // 50us LOW pulse (bit start marker)
    count = 0;
    while (digitalRead(pin) == LOW) {
      if (++count >= MAX_CYCLES) {
        interrupts();
        return false;
      }
    }
    cycles[i] = count;

    // HIGH pulse (26-28us for bit '0', ~70us for bit '1')
    count = 0;
    while (digitalRead(pin) == HIGH) {
      if (++count >= MAX_CYCLES) {
        interrupts();
        return false;
      }
    }
    cycles[i + 1] = count;
  }

  interrupts(); // Critical timing section finished (~4ms)

  // 4. Decode 40 bits
  // In DHT protocol:
  // Bit '0': HIGH pulse (28us) < LOW pulse (50us)
  // Bit '1': HIGH pulse (70us) > LOW pulse (50us)
  for (int i = 0; i < 40; i++) {
    uint32_t lowCycles = cycles[2 * i];
    uint32_t highCycles = cycles[2 * i + 1];

    data[i / 8] <<= 1;
    if (highCycles > lowCycles) {
      data[i / 8] |= 1;
    }
  }

  // 5. Verify Checksum: Byte 4 == (Byte 0 + Byte 1 + Byte 2 + Byte 3) & 0xFF
  uint8_t checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
  if (data[4] == checksum && (data[0] != 0 || data[2] != 0)) {
    if (data[0] <= 100 && data[2] <= 80) {
      // DHT11 format
      humidity = (float)data[0] + (float)data[1] * 0.1f;
      temp = (float)data[2] + (float)data[3] * 0.1f;
    } else {
      // DHT22 format
      humidity = ((data[0] << 8) | data[1]) * 0.1f;
      temp = (((data[2] & 0x7F) << 8) | data[3]) * 0.1f;
      if (data[2] & 0x80) temp = -temp;
    }
    return true;
  }

  return false;
}

// ---------------- FORWARD DECLARATIONS ----------------
void motorOn();
void motorOff();
void stepPulse(int delayUs);
void moveToAngle(float targetAngle, int speedUs = 0);
void findZeroHomeDatum();
void transitionToAutoMode();
void updateDhtSensors();
void updatePowerSensors();
void calibrateCurrentSensor();
void handleManualControls();
void processSerialInput();
void executeSunTracking();
void printTelemetry();
void printHelp();

// =============================================================================
// SETUP
// =============================================================================
void setup() {
  // 1. Initialize Dual Serial Channels @ 115200 Baud (High-speed, zero buffer overrun)
  Serial1.begin(115200);  // PC Flashing & Telemetry on PA9/PA10 @ 115200 Baud
  Serial2.begin(115200);  // ESP32 Telemetry Link on PA2/PA3 @ 115200 Baud

  // 2. Hardware Pins Configuration
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  motorOff();  // 0W silent coils

  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);  // LED ON at boot

  pinMode(PIN_LDR_TOP1, INPUT);
  pinMode(PIN_LDR_TOP2, INPUT);
  pinMode(PIN_LDR_BOT1, INPUT);
  pinMode(PIN_LDR_BOT2, INPUT);
  pinMode(PIN_HALL_HOME, INPUT_PULLUP);  // Internal pull-up (Active LOW when magnet detected)

  pinMode(PIN_POT_MANUAL, INPUT);
  pinMode(PIN_SW_MODE, INPUT_PULLUP);      // PB12 ON/OFF switch (Active LOW to GND)
  pinMode(PIN_SW_MODE_ALT, INPUT_PULLUP);  // PB13 ON/OFF switch (Active LOW to GND)

  pinMode(PIN_SOLAR_VOLT, INPUT);
  pinMode(PIN_SOLAR_CURR, INPUT);
  pinMode(PIN_BATT_VOLT, INPUT);
  pinMode(PIN_DHT11, INPUT_PULLUP);  // Hold DHT11 bus idle HIGH at boot

  // Triple blink on boot for instant visual confirmation
  for (int b = 0; b < 3; b++) {
    digitalWrite(PIN_STATUS_LED, LOW);
    delay(80);
    digitalWrite(PIN_STATUS_LED, HIGH);
    delay(80);
  }

  printlnAll();
  printBar();
  printlnAll(F("POWER IQ — SOLAR TRACKER (STM32F103 @ 115200)"));
  printBar();
  Serial1.flush();

  // 3. Auto-calibrate Current Sensor Zero Baseline
  calibrateCurrentSensor();

  // 4. Initial Sensor Readings
  updatePowerSensors();

  // 5. Check if Hall sensor detects magnet at boot
  if (digitalRead(PIN_HALL_HOME) == LOW) {
    currentAngle = 0.0f;
    isHomed = true;
    printlnAll(F("[BOOT] Hall magnet detected: 0.0 deg ZERO datum LOCKED."));
  } else {
    printlnAll(F("[BOOT] Use Pot or 'ZERO' command to align physical slats to 0.0 deg."));
  }

  // 6. Read Environment Sensor after homing stabilization
  updateDhtSensors();

  digitalWrite(PIN_STATUS_LED, HIGH);  // LED OFF (Ready)
  printlnAll(F("[SYSTEM READY] Tracking ACTIVE @ 115200 Baud. Telemetry streaming below.\n"));
  Serial1.flush();
}

// =============================================================================
// MAIN LOOP
// =============================================================================
void loop() {
  // 1. Process Serial Commands (From PC or ESP32)
  processSerialInput();

  // 2. Hardware Manual Mode Button & Potentiometer Processing
  handleManualControls();

  // 3. Read Power & Environment Sensors Periodically
  updatePowerSensors();
  updateDhtSensors();

  // 4. Sun Tracking (every 500ms in AUTO mode)
  unsigned long now = millis();
  if (now - lastTrackTime >= 500) {
    lastTrackTime = now;
    if (isAutoTracking) {
      executeSunTracking();
    }
  }

  // 5. Periodic 1-Second Telemetry (PC Debug & ESP32 JSON Stream)
  if (now - lastTelemetryTime >= 1000) {
    lastTelemetryTime = now;
    printTelemetry();
    sendEsp32JsonTelemetry();
  }

  // 6. Heartbeat LED (Toggles every 1s when active)
  if (now - lastHeartbeatTime >= 1000) {
    lastHeartbeatTime = now;
    heartbeatState = !heartbeatState;
    digitalWrite(PIN_STATUS_LED, heartbeatState ? LOW : HIGH);
  }
}

// =============================================================================
// MANUAL CONTROLS: HARDWARE ON/OFF SWITCH (PB12/PB13) & 10K POTENTIOMETER (PB0)
// =============================================================================
void handleManualControls() {
  // 1. Hardware 2-Position ON/OFF Switch (PB12 or PB13 to GND)
  // WIRING GUIDE:
  // - Terminal 1 -> STM32 Pin PB12 (or PB13)
  // - Terminal 2 -> GND (Negative side)
  // When switch is ON (closed to GND) -> Pin reads LOW (0V) -> AUTO MODE (Sun Tracking)
  // When switch is OFF (open circuit) -> Pin reads HIGH (3.3V) -> MANUAL MODE (Potentiometer)
  int raw12 = digitalRead(PIN_SW_MODE);
  int raw13 = digitalRead(PIN_SW_MODE_ALT);

  // Switch is ON (closed to GND) if either PB12 or PB13 is pulled LOW
  int currentRaw = (raw12 == LOW || raw13 == LOW) ? 1 : 0;

  // Initial read at boot
  if (lastStableSwitchState == -1) {
    lastStableSwitchState = currentRaw;
    lastRawSwitchState = currentRaw;
    bool targetAuto = (currentRaw == 1);
    if (switchInvert) targetAuto = !targetAuto;
    isAutoTracking = targetAuto;
    lastTrackingStatus = isAutoTracking ? "AUTO (LDR Sun Tracking)" : "MANUAL MODE";

    printAll(F(">>> [MODE SWITCH AT BOOT] Switch is "));
    printAll(currentRaw == 1 ? F("ON (PIN LOW / GND) -> AUTO MODE") : F("OFF (PIN HIGH / OPEN) -> MANUAL MODE"));
    printlnAll();
    Serial1.flush();
  }

  // 50ms Debounce for Rocker/Toggle Switch
  if (currentRaw != lastRawSwitchState) {
    lastSwitchDebounceMs = millis();
    lastRawSwitchState = currentRaw;
  }

  if ((millis() - lastSwitchDebounceMs) > 50) {
    if (currentRaw != lastStableSwitchState) {
      lastStableSwitchState = currentRaw;
      bool targetAuto = (lastStableSwitchState == 1);
      if (switchInvert) targetAuto = !targetAuto;

      if (targetAuto != isAutoTracking) {
        if (targetAuto) {
          // Switching from MANUAL to AUTO: ALWAYS align to ZERO DATUM first before tracking!
          transitionToAutoMode();
        } else {
          // Switching from AUTO to MANUAL
          isAutoTracking = false;
          printlnAll();
          printAll(F(">>> [MODE SWITCH FLIPPED] Switch is now: OFF (PIN HIGH / OPEN) -> MANUAL MODE (Potentiometer Control)\n"));
          Serial1.flush();
          if (currentPotAngle == 0.0f) {
            lastTrackingStatus = "MANUAL: 0.0 deg ZERO (START)";
          } else if (currentPotAngle > 0.0f) {
            lastTrackingStatus = "MANUAL: RIGHT (+ POSITIVE)";
          } else {
            lastTrackingStatus = "MANUAL: LEFT (- NEGATIVE)";
          }
          moveToAngle(currentPotAngle, TRACKING_SPEED_US);
        }
      }
    }
  }

  // 2. Hardware 10k Potentiometer Manual Controller (PB0):
  // Advanced Anti-Jerk DSP Filter: 32x Multisampling + Adaptive IIR Filter + Hysteresis Engine

  // A. 32-Sample Multisampling to cancel high-frequency ADC & motor driver noise
  long potSum = 0;
  for (int i = 0; i < 32; i++) {
    potSum += analogRead(PIN_POT_MANUAL);
    delayMicroseconds(20);
  }
  float rawAveraged = (float)potSum / 32.0f;

  // B. Adaptive Exponential Moving Average (EMA) Low-Pass Filter
  static float filteredPotAdc = -1.0f;
  if (filteredPotAdc < 0.0f) {
    filteredPotAdc = rawAveraged;
  } else {
    float deltaAdc = fabs(rawAveraged - filteredPotAdc);
    // Heavy smoothing (alpha = 0.08) for small noise; faster response (alpha = 0.28) for rapid manual turns
    float alpha = (deltaAdc > 120.0f) ? 0.28f : 0.08f;
    filteredPotAdc = (filteredPotAdc * (1.0f - alpha)) + (rawAveraged * alpha);
  }

  // C. Calculate Target Angle (-45.0° to +45.0°) with Center & Limit Snapping
  // Center (ADC ~2048) -> 0.0 deg ZERO DATUM
  // Clockwise (ADC -> 4095) -> Positive (+0.1 to +45.0 deg)
  // Anti-Clockwise (ADC -> 0) -> Negative (-0.1 to -45.0 deg)
  float rawDeg = ((filteredPotAdc / 4095.0f) * 90.0f) - 45.0f;
  if (potInvert) rawDeg = -rawDeg;

  // Center Zero Deadband: snap cleanly to exact 0.0 deg ZERO datum within +/- 1.5 deg
  if (fabs(rawDeg) <= 1.5f) {
    rawDeg = 0.0f;
  }

  // Hard limit clamping to exactly -45.0 deg and +45.0 deg
  if (rawDeg > 45.0f) rawDeg = 45.0f;
  if (rawDeg < -45.0f) rawDeg = -45.0f;

  // End limit snapping near full physical boundaries
  if (rawDeg >= 43.5f) rawDeg = 45.0f;
  if (rawDeg <= -43.5f) rawDeg = -45.0f;

  // 0.5° Resolution Quantization (eliminates fractional bouncing)
  float targetPotAngle = roundf(rawDeg * 2.0f) / 2.0f;
  currentPotAngle = targetPotAngle;

  // D. Anti-Jerk Hysteresis & Rate-Limited Motor Actuation
  static float lastActuatedAngle = 999.0f;
  static unsigned long lastPotMotionMs = 0;
  static unsigned long potSettleStartTime = 0;

  if (!isAutoTracking) {
    float angleDiff = fabs(currentPotAngle - currentAngle);

    // Track when the potentiometer target is actively changing
    static float prevTargetPotAngle = 999.0f;
    if (fabs(currentPotAngle - prevTargetPotAngle) >= 0.5f) {
      prevTargetPotAngle = currentPotAngle;
      potSettleStartTime = millis();
    }

    // Motion Condition 1: Significant Intentional Rotation (>= 1.5° difference)
    // Rate-limited to max once every 120ms to allow smooth continuous stepper sweeps
    bool triggerMajorMove = (angleDiff >= 1.5f) && (millis() - lastPotMotionMs >= 120);

    // Motion Condition 2: Fine Settle Completion
    // If user made a small fine adjustment (>= 0.5°), wait until the knob has settled stationary
    // for at least 300ms, then do a single gentle alignment and stop
    bool triggerFineSettle = (angleDiff >= 0.5f) && (millis() - potSettleStartTime > 300) && (millis() - lastPotMotionMs >= 300);

    if (triggerMajorMove || triggerFineSettle) {
      lastPotMotionMs = millis();
      lastActuatedAngle = currentPotAngle;

      // Update status string
      if (currentPotAngle == 0.0f) {
        lastTrackingStatus = "MANUAL: 0.0 deg ZERO (CENTER)";
      } else if (currentPotAngle > 0.0f) {
        lastTrackingStatus = "MANUAL: CLOCKWISE (+ POSITIVE)";
      } else {
        lastTrackingStatus = "MANUAL: ANTI-CLOCKWISE (- NEGATIVE)";
      }

      // Smoothly move solar slats to commanded angle
      moveToAngle(currentPotAngle, TRACKING_SPEED_US);

      // Print live angle telemetry
      printAll(F(">>> [POT ROTATION] Pot: "));
      if (currentPotAngle >= 0) printAll(F("+"));
      printAll(currentPotAngle, 1);
      if (currentPotAngle == 0.0f) {
        printAll(F(" deg [CENTER ZERO DATUM]"));
      } else if (currentPotAngle > 0.0f) {
        printAll(F(" deg [CLOCKWISE / POSITIVE (+)]"));
      } else {
        printAll(F(" deg [ANTI-CLOCKWISE / NEGATIVE (-)]"));
      }
      printAll(F(" | Solar Slat: "));
      if (currentAngle >= 0) printAll(F("+"));
      printAll(currentAngle, 1);
      printlnAll(F(" deg"));
      Serial1.flush();
    }
  }
}

// =============================================================================
// CURRENT SENSOR AUTO-ZERO CALIBRATION
// =============================================================================
void calibrateCurrentSensor() {
  long sum = 0;
  for (int i = 0; i < 64; i++) {
    sum += analogRead(PIN_SOLAR_CURR);
    delayMicroseconds(250);
  }
  currentZeroOffset = ((sum / 64.0f) * refVoltage) / 4095.0f;
}

// =============================================================================
// POWER SENSING (SOLAR & BATTERY)
// =============================================================================
void updatePowerSensors() {
  unsigned long now = millis();
  if (now - lastPowerReadTime < 500) return;
  lastPowerReadTime = now;

  // 1. Solar Voltage (PA6)
  long sumSV = 0;
  for (int i = 0; i < 8; i++) {
    sumSV += analogRead(PIN_SOLAR_VOLT);
    delayMicroseconds(40);
  }
  float adcVoltS = ((sumSV / 8.0f) * refVoltage) / 4095.0f;
  solarVoltage = adcVoltS * VOLT_DIVIDER_RATIO;
  if (solarVoltage < 0.20f) solarVoltage = 0.0f;  // Noise floor

  // 2. Solar Current (PA7) with auto-calibrated zero baseline
  long sumSC = 0;
  for (int i = 0; i < 16; i++) {
    sumSC += analogRead(PIN_SOLAR_CURR);
    delayMicroseconds(40);
  }
  float adcCurrS = ((sumSC / 16.0f) * refVoltage) / 4095.0f;
  float diffVolt = adcCurrS - currentZeroOffset;

  // Deadband around zero: if difference is less than 30mV (~0.3A), force 0.00 A
  if (fabs(diffVolt) < 0.030f) {
    solarCurrent = 0.0f;
  } else {
    solarCurrent = diffVolt / currentSensitivity;
    if (solarCurrent < 0.10f) solarCurrent = 0.0f;  // Positive unidirectional solar flow
  }

  // 3. Solar Power (Watts)
  solarPower = solarVoltage * solarCurrent;

  // 4. Battery Voltage (PB1)
  long sumBV = 0;
  for (int i = 0; i < 8; i++) {
    sumBV += analogRead(PIN_BATT_VOLT);
    delayMicroseconds(40);
  }
  float adcVoltB = ((sumBV / 8.0f) * refVoltage) / 4095.0f;
  battVoltage = adcVoltB * VOLT_DIVIDER_RATIO;
  if (battVoltage < 0.20f) battVoltage = 0.0f;
}

// =============================================================================
// DHT11 SENSOR READ (EVERY 2.5 SECONDS)
// =============================================================================
void updateDhtSensors() {
  unsigned long now = millis();
  if (now < 1500) return;  // Allow sensor 1.5s power stabilization after boot
  if (now - lastDhtReadTime >= 2500) {
    lastDhtReadTime = now;
    float t = 0.0f, h = 0.0f;
    if (readDHT11(PIN_DHT11, t, h)) {
      currentTemp = t;
      currentHumidity = h;
    }
  }
}

// =============================================================================
// =============================================================================
// SAFE ZERO HOMING DATUM (MOVES TOWARDS ZERO, NEVER FORCES POSITIVE CRASH)
// =============================================================================
void findZeroHomeDatum() {
  printlnAll(F("\n[HOMING] Calibrating ZERO Position (0.0 deg Datum)..."));

  // 1. If Hall sensor already detects magnet at 0.0 deg, confirm immediately
  if (digitalRead(PIN_HALL_HOME) == LOW) {
    currentAngle = 0.0f;
    isHomed = true;
    motorOff();
    printlnAll(F("[HOMING] Magnet detected at sensor! ZERO Datum Confirmed: 0.0 deg.\n"));
    return;
  }

  // 2. Determine safe homing direction:
  // If we are on the positive side (e.g. +45 deg), move NEGATIVE (LOW) towards 0.0 deg.
  // If we are on the negative side (e.g. -30 deg), move POSITIVE (HIGH) towards 0.0 deg.
  // If starting position is unknown, step NEGATIVE (LOW) to pull away from the +45 limit!
  uint8_t homeDir = (currentAngle < 0.0f) ? HIGH : LOW;

  motorOn();
  digitalWrite(PIN_STATUS_LED, LOW);
  digitalWrite(PIN_DIR, homeDir);

  bool foundMagnet = false;
  long maxSteps = (long)(50.0f * STEPS_PER_DEGREE);

  for (long s = 0; s < maxSteps; s++) {
    if (digitalRead(PIN_HALL_HOME) == LOW) {
      foundMagnet = true;
      break;
    }
    stepPulse(ZERO_HOMING_SPEED_US);
  }

  if (foundMagnet) {
    // Center on the magnet's active span
    long spanSteps = 0;
    long maxSpan = (long)(15.0f * STEPS_PER_DEGREE);
    while (digitalRead(PIN_HALL_HOME) == LOW && spanSteps < maxSpan) {
      stepPulse(ZERO_HOMING_SPEED_US);
      spanSteps++;
    }

    long centerSteps = spanSteps / 2;
    if (centerSteps > 0) {
      digitalWrite(PIN_DIR, (homeDir == HIGH) ? LOW : HIGH);
      for (long s = 0; s < centerSteps; s++) {
        stepPulse(ZERO_HOMING_SPEED_US);
      }
    }

    currentAngle = 0.0f;
    isHomed = true;
    lastTrackingStatus = "ZERO DATUM LOCKED (0.0 deg)";
    printlnAll(F("[HOMING] Magnet centered successfully! ZERO Position LOCKED at 0.0 deg."));
    Serial1.flush();
  } else {
    // Magnet was not reached: Step back to original position to prevent tilting
    digitalWrite(PIN_DIR, (homeDir == HIGH) ? LOW : HIGH);
    for (long s = 0; s < maxSteps; s++) {
      stepPulse(ZERO_HOMING_SPEED_US);
    }
    printlnAll(F("[HOMING] No magnet detected. Returned to start position."));
    printlnAll(F("[HOMING] Use Potentiometer or 'ZERO' command to calibrate."));
    Serial1.flush();
  }

  motorOff();
  digitalWrite(PIN_STATUS_LED, HIGH);
  printBar();
  Serial1.flush();
}

// =============================================================================
// TRANSITION TO AUTO MODE (MANDATORY RETURN TO ZERO DATUM BEFORE SUN TRACKING)
// =============================================================================
void transitionToAutoMode() {
  printBar();
  printlnAll(F("[AUTO SWITCH] Returning slats to 0.0 deg ZERO datum..."));

  lastTrackingStatus = "ALIGNING TO 0.0 deg ZERO";

  // 1. If currently at any non-zero angle, smoothly drive to 0.0 deg ZERO first
  if (fabs(currentAngle) > 0.5f) {
    printAll(F("[AUTO SWITCH] Slats moving "));
    printAll(currentAngle, 1);
    printlnAll(F(" deg -> 0.0 deg..."));
    moveToAngle(0.0f, ZERO_HOMING_SPEED_US);
  }

  // 2. Check if Hall sensor detects magnet at 0.0 deg
  if (digitalRead(PIN_HALL_HOME) == LOW) {
    printlnAll(F("[AUTO SWITCH] Hall Magnet confirmed at 0.0 deg ZERO."));
  }

  // 3. Confirm exact zero datum and enable auto tracking
  currentAngle = 0.0f;
  isHomed = true;
  isAutoTracking = true;
  lastTrackingStatus = "ZERO LOCKED -> Sun Search Active";

  printlnAll(F("[AUTO SWITCH] ZERO Locked! Auto Sun Tracking active."));
  printBar();
  printTelemetry();
  sendEsp32JsonTelemetry();
  Serial1.flush();
}

// =============================================================================
// CLOSED-LOOP SUN TRACKING CORE ALGORITHM
// =============================================================================
void executeSunTracking() {
  if (!isHomed) {
    findZeroHomeDatum();
    return;
  }

  frameCount++;

  // 1. Read 4 LDRs
  long sumT1 = 0, sumT2 = 0, sumB1 = 0, sumB2 = 0;
  for (int i = 0; i < 4; i++) {
    sumT1 += analogRead(PIN_LDR_TOP1);
    sumT2 += analogRead(PIN_LDR_TOP2);
    sumB1 += analogRead(PIN_LDR_BOT1);
    sumB2 += analogRead(PIN_LDR_BOT2);
    delayMicroseconds(100);
  }
  int rawT1 = sumT1 / 4;
  int rawT2 = sumT2 / 4;
  int rawB1 = sumB1 / 4;
  int rawB2 = sumB2 / 4;

  int lightT1 = max(0, 4095 - rawT1);
  int lightT2 = max(0, 4095 - rawT2);
  int lightB1 = max(0, 4095 - rawB1);
  int lightB2 = max(0, 4095 - rawB2);

  int avgTop = (lightT1 + lightT2) / 2;
  int avgBottom = (lightB1 + lightB2) / 2;
  int maxLight = max(avgTop, avgBottom);

  int diff = avgTop - avgBottom;
  if (invertMotorDir) diff = -diff;

  lastTopVal = avgTop;
  lastBotVal = avgBottom;
  lastDiffVal = diff;

  if (maxLight < nightDarkThreshold) {
    if (darknessStartMs == 0) darknessStartMs = millis();
    unsigned long darkDuration = millis() - darknessStartMs;

    if (darkDuration >= NIGHT_PARK_DELAY_MS) {
      if (fabs(currentAngle) > 0.5f) {
        lastTrackingStatus = "NIGHT CONFIRMED -> Parking 0.0 deg";
        moveToAngle(0.0f, TRACKING_SPEED_US);
      } else {
        lastTrackingStatus = "NIGHT SLEEP (0.0 deg, 0W Coils OFF)";
        motorOff();
      }
    } else {
      lastTrackingStatus = "LOW LIGHT -> HOLDING (8s Timer)";
      motorOff();
    }
  } else {
    darknessStartMs = 0;

    if (abs(diff) <= deadbandThreshold) {
      lastTrackingStatus = "SUN BALANCED (Optimum Yield)";
      motorOff();
    } else if (diff > deadbandThreshold) {
      float nextAngle = currentAngle + 1.0f;
      if (nextAngle <= MAX_ANGLE) {
        lastTrackingStatus = "TRACKING SUN (+)";
        moveToAngle(nextAngle, TRACKING_SPEED_US);
      } else {
        lastTrackingStatus = "LIMIT REACHED (+45 deg MAX)";
      }
    } else {
      float nextAngle = currentAngle - 1.0f;
      if (nextAngle >= MIN_ANGLE) {
        lastTrackingStatus = "TRACKING SUN (-)";
        moveToAngle(nextAngle, TRACKING_SPEED_US);
      } else {
        lastTrackingStatus = "LIMIT REACHED (-45 deg MIN)";
      }
    }
  }
}

// =============================================================================
// STREAMLINED HIGH-SPEED TELEMETRY (CLEAN, JITTER-FREE @ 115200 BAUD)
// =============================================================================
void printTelemetry() {
  printBar();
  printAll(F("[SOLAR-DATA] Solar: "));
  printAll(solarVoltage, 2);
  printAll(F("V | Current: "));
  printAll(solarCurrent, 2);
  printAll(F("A | Power: "));
  printAll(solarPower, 2);
  printAll(F(" Watts | Bat: "));
  printAll(battVoltage, 2);
  printAll(F("V | Angle: "));
  if (currentAngle >= 0) printAll(F("+"));
  printAll(currentAngle, 1);
  printlnAll(F(" deg"));

  printAll(F("           LDR Top: "));
  printAll(lastTopVal);
  printAll(F(" | Bot: "));
  printAll(lastBotVal);
  printAll(F(" | Diff: "));
  if (lastDiffVal >= 0) printAll(F("+"));
  printAll(lastDiffVal);
  printAll(F(" (DB: +/-"));
  printAll(deadbandThreshold);
  printAll(F(") | Temp: "));
  printAll(currentTemp, 1);
  printAll(F("C | Hum: "));
  printAll(currentHumidity, 1);
  printlnAll(F("%"));

  printAll(F("           Mode: "));
  printAll(isAutoTracking ? F("AUTO (LDR)") : F("MANUAL (POT)"));
  printAll(F(" | Pot: "));
  if (currentPotAngle >= 0) printAll(F("+"));
  printAll(currentPotAngle, 1);
  if (currentPotAngle == 0.0f) printAll(F(" (ZERO)"));
  printAll(F(" | Zero: "));
  printAll(isHomed ? F("LOCKED (0.0 deg)") : F("HOMING"));
  printAll(F(" | Status: "));
  printlnAll(lastTrackingStatus);
  printBar();
  Serial1.flush();
}

// =============================================================================
// MOTOR KINEMATIC CONTROL
// =============================================================================
void moveToAngle(float targetAngle, int speedUs) {
  // HARD MECHANICAL PROTECTION: Never exceed -45.0 to +45.0 deg
  if (targetAngle < -45.0f) targetAngle = -45.0f;
  if (targetAngle > 45.0f) targetAngle = 45.0f;

  // Safe operating boundaries: keep safely within MIN_ANGLE (-40.0 deg) and MAX_ANGLE (+40.0 deg)
  if (targetAngle < MIN_ANGLE) targetAngle = MIN_ANGLE;
  if (targetAngle > MAX_ANGLE) targetAngle = MAX_ANGLE;

  float deltaDeg = targetAngle - currentAngle;
  if (fabs(deltaDeg) < 0.05f) return;

  long steps = (long)(fabs(deltaDeg) * STEPS_PER_DEGREE + 0.5f);
  // Maximum travel cap in a single move: strictly limited to 90 degrees total physical span
  long maxAllowedSteps = (long)(90.0f * STEPS_PER_DEGREE);
  if (steps > maxAllowedSteps) steps = maxAllowedSteps;
  if (steps == 0) {
    currentAngle = targetAngle;
    return;
  }

  int activeSpeed = (speedUs > 0) ? speedUs : TRACKING_SPEED_US;

  motorOn();
  digitalWrite(PIN_STATUS_LED, LOW);
  digitalWrite(PIN_DIR, (deltaDeg > 0) ? HIGH : LOW);

  for (long i = 0; i < steps; i++) {
    stepPulse(activeSpeed);
  }

  currentAngle = targetAngle;
  motorOff();
  digitalWrite(PIN_STATUS_LED, HIGH);
}

void stepPulse(int delayUs) {
  digitalWrite(PIN_STEP, HIGH);
  delayMicroseconds(5);  // Minimum 1us required by A4988 datasheet
  digitalWrite(PIN_STEP, LOW);
  delayMicroseconds(delayUs);
}

void motorOn() {
  digitalWrite(PIN_ENABLE, LOW);
  delay(5);
}

void motorOff() {
  digitalWrite(PIN_ENABLE, HIGH);
}

// =============================================================================
// SERIAL COMMAND PARSER
// =============================================================================
static char cmdBuf1[48];
static byte cmdPos1 = 0;
static char cmdBuf2[48];
static byte cmdPos2 = 0;

void handleCommand(char* cmd) {
  while (*cmd == ' ') cmd++;
  if (*cmd == 0) return;

  if (strcasecmp(cmd, "AUTO") == 0) {
    transitionToAutoMode();
  } else if (strcasecmp(cmd, "MANUAL") == 0) {
    isAutoTracking = false;
    motorOff();
    lastTrackingStatus = "MANUAL MODE";
    printlnAll(F("\n[CMD] Manual mode active (Awaiting remote GOTO/JOG or Potentiometer).\n"));
  } else if (strcasecmp(cmd, "STOP") == 0) {
    isAutoTracking = false;
    motorOff();
    lastTrackingStatus = "EMERGENCY STOP (Coils OFF)";
    printlnAll(F("\n[CMD] EMERGENCY STOP: Motor coils disabled.\n"));
  } else if (strcasecmp(cmd, "HOME") == 0) {
    isAutoTracking = false;
    findZeroHomeDatum();
    isAutoTracking = true;
  } else if (strcasecmp(cmd, "ZERO") == 0 || strcasecmp(cmd, "CAL_ZERO") == 0) {
    currentAngle = 0.0f;
    isHomed = true;
    lastTrackingStatus = "ZERO LOCKED (0.0 deg)";
    printlnAll(F("\n[CMD] Physical position CALIBRATED as 0.0 deg ZERO datum!\n"));
    printTelemetry();
    sendEsp32JsonTelemetry();
  } else if (strcasecmp(cmd, "INVERT") == 0) {
    invertMotorDir = !invertMotorDir;
    printAll(F("\n[CMD] Tracking direction inverted: "));
    printlnAll(invertMotorDir ? F("REVERSED") : F("NORMAL"));
  } else if (strncasecmp(cmd, "SPEED_TRACK ", 12) == 0) {
    int val = atoi(cmd + 12);
    if (val >= 800 && val <= 5000) {
      TRACKING_SPEED_US = val;
      printAll(F("\n[CMD] Tracking Speed updated to: "));
      printAll(TRACKING_SPEED_US);
      printlnAll(F(" us"));
    }
  } else if (strncasecmp(cmd, "SPEED_HOME ", 11) == 0) {
    int val = atoi(cmd + 11);
    if (val >= 800 && val <= 5000) {
      ZERO_HOMING_SPEED_US = val;
      printAll(F("\n[CMD] ZERO Homing Speed updated to: "));
      printAll(ZERO_HOMING_SPEED_US);
      printlnAll(F(" us"));
    }
  } else if (strncasecmp(cmd, "VREF ", 5) == 0) {
    float val = parseCustomFloat(cmd + 5);
    if (val > 2.0f && val < 5.5f) {
      refVoltage = val;
      printAll(F("\n[CMD] ADC VREF calibrated to: "));
      printAll(refVoltage, 2);
      printlnAll(F(" V"));
    }
  } else if (strncasecmp(cmd, "DEADBAND ", 9) == 0) {
    int val = atoi(cmd + 9);
    if (val >= 5 && val <= 1000) {
      deadbandThreshold = val;
      printAll(F("\n[CMD] Deadband updated to: "));
      printlnAll(deadbandThreshold);
    }
  } else if (strcasecmp(cmd, "ZERO_CURR") == 0 || strcasecmp(cmd, "CAL_CURR") == 0) {
    calibrateCurrentSensor();
    printAll(F("\n[CMD] Current Zero Baseline calibrated to: "));
    printAll(currentZeroOffset, 3);
    printlnAll(F(" V -> Current is now 0.00 A\n"));
  } else if (strcasecmp(cmd, "DHT") == 0 || strcasecmp(cmd, "TEMP") == 0) {
    printlnAll(F("\n[DHT11] Testing sensor read on pin PB5..."));
    float t = 0.0f, h = 0.0f;
    if (readDHT11(PIN_DHT11, t, h)) {
      currentTemp = t;
      currentHumidity = h;
      printAll(F("[DHT11] SUCCESS! Temp: "));
      printAll(currentTemp, 1);
      printAll(F(" C | Humidity: "));
      printAll(currentHumidity, 1);
      printlnAll(F(" % (Checksum Verified)\n"));
    } else {
      printlnAll(F("[DHT11] FAILED: No response or checksum error from PB5."));
      printAll(F("[DHT11] Pin PB5 State: "));
      printlnAll(digitalRead(PIN_DHT11) == HIGH ? F("IDLE HIGH (Pull-up OK)") : F("STUCK LOW (Short/GND)"));
      printlnAll(F("[DHT11] Troubleshooting: Verify VCC (3.3V/5V), GND, and PB5 signal wire.\n"));
    }
  } else if (strcasecmp(cmd, "SW") == 0 || strcasecmp(cmd, "SWITCH") == 0 || strcasecmp(cmd, "BTN") == 0 || strcasecmp(cmd, "BUTTON") == 0) {
    int r12 = digitalRead(PIN_SW_MODE);
    int r13 = digitalRead(PIN_SW_MODE_ALT);
    printBar();
    printAll(F("[SW] PB12: ")); printlnAll(r12 == LOW ? F("LOW (GND)") : F("HIGH"));
    printAll(F("[SW] PB13: ")); printlnAll(r13 == LOW ? F("LOW (GND)") : F("HIGH"));
    printAll(F("[SW] State: ")); printlnAll((r12 == LOW || r13 == LOW) ? F("ON") : F("OFF"));
    printAll(F("[SW] Mode: ")); printlnAll(isAutoTracking ? F("AUTO") : F("MANUAL"));
    printAll(F("[SW] Logic: ")); printlnAll(switchInvert ? F("INVERTED") : F("NORMAL"));
    printBar();
  } else if (strcasecmp(cmd, "INVERT_SW") == 0 || strcasecmp(cmd, "SW_INVERT") == 0) {
    switchInvert = !switchInvert;
    bool targetAuto = (lastStableSwitchState == 1);
    if (switchInvert) targetAuto = !targetAuto;
    isAutoTracking = targetAuto;
    printAll(F("\n[CMD] Switch logic inverted! Mapping is now: "));
    printlnAll(switchInvert ? F("ON=MANUAL, OFF=AUTO") : F("ON=AUTO, OFF=MANUAL"));
    printAll(F("[CMD] Current Active Mode is now: "));
    printlnAll(isAutoTracking ? F("AUTO (LDR Sun Tracking)") : F("MANUAL (Potentiometer Control)"));
    printlnAll();
  } else if (strcasecmp(cmd, "MODE") == 0 || strcasecmp(cmd, "TOGGLE") == 0) {
    if (!isAutoTracking) {
      transitionToAutoMode();
    } else {
      isAutoTracking = false;
      printlnAll(F("\n[CMD] Switched to MANUAL Mode (Potentiometer active).\n"));
    }
  } else if (strcasecmp(cmd, "RECOVER") == 0 || strcasecmp(cmd, "RECOVER_LEFT") == 0) {
    printlnAll(F("\n[RECOVERY] Recovering mechanism from >+45 deg back to 0.0 deg ZERO..."));
    motorOn();
    digitalWrite(PIN_STATUS_LED, LOW);
    digitalWrite(PIN_DIR, LOW); // Step in negative direction towards zero
    long steps45 = (long)(45.0f * STEPS_PER_DEGREE);
    for (long s = 0; s < steps45; s++) {
      if (digitalRead(PIN_HALL_HOME) == LOW) {
        printlnAll(F("[RECOVERY] Magnet detected! Centering on ZERO datum..."));
        break;
      }
      stepPulse(ZERO_HOMING_SPEED_US);
    }
    motorOff();
    digitalWrite(PIN_STATUS_LED, HIGH);
    currentAngle = 0.0f;
    isHomed = true;
    lastTrackingStatus = "ZERO RECOVERED (0.0 deg)";
    printlnAll(F("[RECOVERY] Slats recovered to ZERO position! Locked at 0.0 deg.\n"));
  } else if (strncasecmp(cmd, "LEFT ", 5) == 0 || strncasecmp(cmd, "JOG_LEFT ", 9) == 0) {
    float deg = parseCustomFloat(cmd + (*cmd == 'J' ? 9 : 5));
    if (deg > 0.0f && deg <= 45.0f) {
      isAutoTracking = false;
      printAll(F("\n[JOG] Jogging slats LEFT by "));
      printAll(deg, 1);
      printlnAll(F(" deg..."));
      moveToAngle(currentAngle - deg, TRACKING_SPEED_US);
    }
  } else if (strncasecmp(cmd, "RIGHT ", 6) == 0 || strncasecmp(cmd, "JOG_RIGHT ", 10) == 0) {
    float deg = parseCustomFloat(cmd + (*cmd == 'J' ? 10 : 6));
    if (deg > 0.0f && deg <= 45.0f) {
      isAutoTracking = false;
      printAll(F("\n[JOG] Jogging slats RIGHT by "));
      printAll(deg, 1);
      printlnAll(F(" deg..."));
      moveToAngle(currentAngle + deg, TRACKING_SPEED_US);
    }
  } else if (strcasecmp(cmd, "INVERT_POT") == 0 || strcasecmp(cmd, "POT_INVERT") == 0) {
    potInvert = !potInvert;
    printAll(F("\n[CMD] Potentiometer direction inverted! Direction is now: "));
    printlnAll(potInvert ? F("INVERTED (CW=Negative, CCW=Positive)") : F("NORMAL (CW=Positive, CCW=Negative)"));
    printlnAll();
  } else if (strcasecmp(cmd, "STATUS") == 0) {
    printBar();
    printAll(F("Angle: ")); printAll(currentAngle, 1);
    printAll(F(" deg | Homed: ")); printlnAll(isHomed ? F("YES") : F("NO"));
    printAll(F("Mode: ")); printlnAll(isAutoTracking ? F("AUTO") : F("MANUAL"));
    printAll(F("Pot: ")); printAll(currentPotAngle, 1);
    printlnAll(currentPotAngle == 0.0f ? F(" (ZERO)") : (currentPotAngle > 0 ? F(" (CW+)") : F(" (CCW-)")));
    printAll(F("Solar: ")); printAll(solarVoltage, 2); printAll(F("V | "));
    printAll(solarCurrent, 2); printAll(F("A | "));
    printAll(solarPower, 2); printlnAll(F("W"));
    printAll(F("Batt: ")); printAll(battVoltage, 2); printlnAll(F("V"));
    printAll(F("Temp: ")); printAll(currentTemp, 1); printAll(F("C | Hum: "));
    printAll(currentHumidity, 1); printlnAll(F("%"));
    printBar();
  } else if (strcasecmp(cmd, "HELP") == 0 || strcmp(cmd, "?") == 0) {
    printHelp();
  } else {
    float target = 0.0f;
    bool isAngle = false;

    if (strncasecmp(cmd, "GOTO ", 5) == 0) {
      target = parseCustomFloat(cmd + 5);
      isAngle = true;
    } else if (strncasecmp(cmd, "JOG ", 4) == 0) {
      float delta = parseCustomFloat(cmd + 4);
      target = currentAngle + delta;
      isAngle = true;
    } else if (strncasecmp(cmd, "MOVE ", 5) == 0) {
      target = parseCustomFloat(cmd + 5);
      isAngle = true;
    } else if (*cmd == '-' || *cmd == '+' || isdigit(*cmd)) {
      target = parseCustomFloat(cmd);
      isAngle = true;
    }

    if (isAngle) {
      if (target < MIN_ANGLE) target = MIN_ANGLE;
      if (target > MAX_ANGLE) target = MAX_ANGLE;
      isAutoTracking = false;
      lastTrackingStatus = "MANUAL COMMAND POSITION";
      printAll(F("\n[MANUAL] Moving slats to "));
      printAll(target, 1);
      printlnAll(F(" deg..."));
      moveToAngle(target, TRACKING_SPEED_US);
      printAll(F("[MANUAL] Reached "));
      printAll(currentAngle, 1);
      printlnAll(F(" deg. (Type 'AUTO' to resume sun tracking)\n"));
    }
  }
}

void processSerialInput() {
  // Read from USART1 (PC Debug PA9/10)
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\r') continue;
    if (c == '\n') {
      cmdBuf1[cmdPos1] = '\0';
      if (cmdPos1 > 0) {
        handleCommand(cmdBuf1);
        cmdPos1 = 0;
      }
    } else if (cmdPos1 < sizeof(cmdBuf1) - 1) {
      cmdBuf1[cmdPos1++] = c;
    }
  }

  // Read from USART2 (ESP32 Remote Link PA2/3)
  while (Serial2.available()) {
    char c = Serial2.read();
    if (c == '\r') continue;
    if (c == '\n') {
      cmdBuf2[cmdPos2] = '\0';
      if (cmdPos2 > 0) {
        handleCommand(cmdBuf2);
        cmdPos2 = 0;
      }
    } else if (cmdPos2 < sizeof(cmdBuf2) - 1) {
      cmdBuf2[cmdPos2++] = c;
    }
  }
}

void printHelp() {
  printBar();
  printlnAll(F("CMDS: AUTO | MANUAL | MODE | SW | INVERT_SW | INVERT_POT"));
  printlnAll(F("      RECOVER | LEFT <deg> | RIGHT <deg> | GOTO <deg>"));
  printlnAll(F("      ZERO | HOME | STATUS | DHT | DEADBAND <n> | VREF <f>"));
  printBar();
}
