/*
================================================================================
 POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 Subsystem: STM32 Sun Tracking + Solar Power (V+I+P) + Battery Side + DHT11
 Platform : STM32F103C8T6 "Blue Pill" (32-bit ARM Cortex-M3 @ 72 MHz)
 Authors  : Prathamesh Sapate (Lead), Shreyash Pachade, Vansh Dobhale, Prachi Ronge
 Purpose  : 100% Pure STM32F103C8T6 ARM Firmware (Guaranteed Serial Stream)
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
   4. TEMPERATURE & HUMIDITY:
      - PB5   -> DHT11 Data Pin (with 4.7k pullup or module)
   5. SOLAR PV POWER MONITORING (Voltage + Current):
      - PA6   -> Solar PV Voltage (ADC1_IN6) [R1=33k, R2=6.8k divider]
      - PA7   -> Solar PV Current (ADC1_IN7) [ACS712 or Shunt Sensor]
   6. BATTERY POWER MONITORING (Optional / Reserved):
      - PB0   -> Battery Voltage (ADC1_IN8) [R1=33k, R2=6.8k divider]
      - PB1   -> Battery Current (ADC1_IN9) [ACS712 or Shunt Sensor]
   7. SERIAL TELEMETRY (USART1 @ 9600 Baud):
      - PA9   -> USB-TTL RX (USART1_TX)
      - PA10  -> USB-TTL TX (USART1_RX)
   8. VISUAL HEARTBEAT LED:
      - PC13  -> On-board Green LED (Blinks every 1s when active)
================================================================================
*/

#include <Arduino.h>

// =============================================================================
// 1. USER SPEED & TIMING VARIABLES
// =============================================================================
int ZERO_HOMING_SPEED_US = 2500;  // Speed during 0.0 deg ZERO search (default: 2500 us)
int TRACKING_SPEED_US    = 2000;  // Speed during Sun Tracking motion (default: 2000 us)

// ---------------- GPIO PIN DEFINITIONS (STM32 BLUE PILL) ----------------
#define PIN_STEP        PB8   // A4988 STEP pulse
#define PIN_DIR         PB9   // A4988 DIR direction
#define PIN_ENABLE      PB10  // A4988 ENABLE (Active LOW)
#define PIN_STATUS_LED  PC13  // Onboard LED (Active LOW)

#define PIN_LDR_TOP1    PA0   // Top Sector Sensor 1
#define PIN_LDR_TOP2    PA1   // Top Sector Sensor 2
#define PIN_LDR_BOT1    PA4   // Bottom Sector Sensor 1
#define PIN_LDR_BOT2    PA5   // Bottom Sector Sensor 2

#define PIN_HALL_HOME   PB11  // Hall-effect Home Sensor (Active LOW)
#define PIN_DHT11       PB5   // DHT11 Data Pin

// ---------------- POWER MONITORING PINS (SOLAR & BATTERY) ----------------
#define PIN_SOLAR_VOLT  PA6   // Solar Voltage Analog Input
#define PIN_SOLAR_CURR  PA7   // Solar Current Analog Input
#define PIN_BATT_VOLT   PB0   // Battery Voltage Analog Input
#define PIN_BATT_CURR   PB1   // Battery Current Analog Input

// Resistor Divider: R1=33k, R2=6.8k -> Factor = (33000 + 6800) / 6800 = 5.8529
const float VOLT_DIVIDER_RATIO = (33000.0f + 6800.0f) / 6800.0f;
float refVoltage = 3.3f; // STM32 ADC Reference Voltage

// Current Sensor Sensitivity (ACS712 20A = 0.100 V/A, 5A = 0.185 V/A, 30A = 0.066 V/A)
float currentSensitivity = 0.100f; // Default 100 mV/A (ACS712-20A)
float currentZeroOffset   = 1.65f;  // Zero current voltage offset (VCC/2 for 3.3V or 2.5V for 5V)

// Power Metrics
float solarVoltage = 0.0f; // Volts
float solarCurrent = 0.0f; // Amps
float solarPower   = 0.0f; // Watts

float battVoltage  = 0.0f; // Volts
float battCurrent  = 0.0f; // Amps
float battPower    = 0.0f; // Watts

unsigned long lastPowerReadTime = 0;

// ---------------- DHT11 ENVIRONMENT ----------------
float currentTemp     = 0.0f; // deg C
float currentHumidity = 0.0f; // %
unsigned long lastDhtReadTime = 0;

// ---------------- KINEMATICS & BENCH CONSTANTS ----------------
const float STEPS_PER_DEGREE = 10.556f; // 19:1 Worm gear ratio
const float MIN_ANGLE = -40.0f;
const float MAX_ANGLE =  40.0f;

int deadbandThreshold   = 50;
int nightDarkThreshold  = 40;
const unsigned long NIGHT_PARK_DELAY_MS = 8000;

bool isAutoTracking   = true;
bool invertMotorDir   = false;
float currentAngle    = 0.0f;
bool  isHomed         = false;
unsigned long lastTrackTime = 0;
unsigned long darknessStartMs = 0;
unsigned long frameCount = 0;
unsigned long lastHeartbeatTime = 0;
bool heartbeatState = false;

// ---------------- DUAL-SERIAL OUTPUT HELPERS ----------------
// Always print to Serial1 (USART1 on PA9) and Serial (USB CDC if enabled)
#ifdef SERIAL_USB
  #define HAS_USB_SERIAL 1
#else
  #define HAS_USB_SERIAL 0
#endif

template<typename T>
void printAll(T msg) {
  Serial1.print(msg);
#if HAS_USB_SERIAL
  Serial.print(msg);
#endif
}

template<typename T, typename P>
void printAll(T msg, P p) {
  Serial1.print(msg, p);
#if HAS_USB_SERIAL
  Serial.print(msg, p);
#endif
}

inline void printlnAll() {
  Serial1.println();
#if HAS_USB_SERIAL
  Serial.println();
#endif
}

template<typename T>
void printlnAll(T msg) {
  Serial1.println(msg);
#if HAS_USB_SERIAL
  Serial.println(msg);
#endif
}

template<typename T, typename P>
void printlnAll(T msg, P p) {
  Serial1.println(msg, p);
#if HAS_USB_SERIAL
  Serial.println(msg, p);
#endif
}

// ---------------- LIGHTWEIGHT PARSERS ----------------
float parseCustomFloat(const char* p) {
  while (*p == ' ') p++;
  float sign = 1.0f;
  if (*p == '-') { sign = -1.0f; p++; }
  else if (*p == '+') { p++; }
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

// ---------------- DIRECT DHT11 BITBANG ----------------
bool readDHT11(uint8_t pin, float &temp, float &humidity) {
  uint8_t data[5] = {0, 0, 0, 0, 0};
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(20);
  digitalWrite(pin, HIGH);
  delayMicroseconds(30);
  pinMode(pin, INPUT_PULLUP);

  unsigned long timeout = micros();
  while (digitalRead(pin) == HIGH) { if (micros() - timeout > 100) return false; }
  timeout = micros();
  while (digitalRead(pin) == LOW) { if (micros() - timeout > 100) return false; }
  timeout = micros();
  while (digitalRead(pin) == HIGH) { if (micros() - timeout > 100) return false; }

  for (int i = 0; i < 40; i++) {
    timeout = micros();
    while (digitalRead(pin) == LOW) { if (micros() - timeout > 100) return false; }
    unsigned long t = micros();
    while (digitalRead(pin) == HIGH) { if (micros() - timeout > 150) return false; }
    if ((micros() - t) > 40) {
      data[i / 8] |= (1 << (7 - (i % 8)));
    }
  }

  if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
    if (data[0] != 0 || data[2] != 0) {
      humidity = (float)data[0];
      temp = (float)data[2];
      return true;
    }
  }
  return false;
}

// ---------------- FORWARD DECLARATIONS ----------------
void motorOn();
void motorOff();
void stepPulse(int delayUs);
void moveToAngle(float targetAngle, int speedUs = 0);
void findZeroHomeDatum();
void updateDhtSensors();
void updatePowerSensors();
void processSerialInput();
void executeSunTracking();
void printTelemetry(int topVal, int botVal, int diff, const char* stateStr);
void printHelp();

// =============================================================================
// SETUP
// =============================================================================
void setup() {
  // 1. Start Serial immediately on PA9 (TX) and PA10 (RX)
  Serial1.begin(9600);
#if HAS_USB_SERIAL
  Serial.begin(9600);
#endif

  // 2. Hardware Pins Configuration
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  motorOff(); // 0W silent coils

  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW); // LED ON at boot

  pinMode(PIN_LDR_TOP1, INPUT);
  pinMode(PIN_LDR_TOP2, INPUT);
  pinMode(PIN_LDR_BOT1, INPUT);
  pinMode(PIN_LDR_BOT2, INPUT);
  pinMode(PIN_HALL_HOME, INPUT_PULLUP);

  pinMode(PIN_SOLAR_VOLT, INPUT);
  pinMode(PIN_SOLAR_CURR, INPUT);
  pinMode(PIN_BATT_VOLT, INPUT);
  pinMode(PIN_BATT_CURR, INPUT);

  // Triple blink on boot for instant visual confirmation
  for (int b = 0; b < 3; b++) {
    digitalWrite(PIN_STATUS_LED, LOW);  delay(80);
    digitalWrite(PIN_STATUS_LED, HIGH); delay(80);
  }

  printlnAll();
  printlnAll(F("========================================================"));
  printlnAll(F(" POWER IQ — STM32 SUN TRACKER & POWER TELEMETRY         "));
  printlnAll(F(" Platform     : STM32F103C8T6 ARM Cortex-M3 (72 MHz)    "));
  printlnAll(F(" Serial Port  : USART1 (PA9 TX, PA10 RX) @ 9600 Baud    "));
  printlnAll(F(" Solar Power  : Voltage (PA6 33k/6.8k) + Current (PA7)  "));
  printlnAll(F(" Battery Side : Voltage (PB0) + Current (PB1)           "));
  printlnAll(F(" Environment  : DHT11 Temp/Humidity (PB5)               "));
  printlnAll(F(" Calibration  : 10.556 steps/deg | Angle: -40 to +40 deg"));
  printAll(F(" Speeds       : Tracking: ")); printAll(TRACKING_SPEED_US);
  printAll(F(" us | ZERO Homing: ")); printAll(ZERO_HOMING_SPEED_US); printlnAll(F(" us"));
  printlnAll(F("========================================================"));

  // 3. Initial Sensor Readings
  updatePowerSensors();
  updateDhtSensors();

  // 4. Calibrate ZERO Datum
  findZeroHomeDatum();

  digitalWrite(PIN_STATUS_LED, HIGH); // LED OFF (Ready)
  printlnAll(F("[SYSTEM READY] Tracking ACTIVE. Telemetry streaming below.\n"));
}

// =============================================================================
// MAIN LOOP
// =============================================================================
void loop() {
  // 1. Process Serial Commands
  processSerialInput();

  // 2. Read Sensors Periodically
  updatePowerSensors();
  updateDhtSensors();

  // 3. Closed-Loop Sun Tracking (every 500ms)
  unsigned long now = millis();
  if (now - lastTrackTime >= 500) {
    lastTrackTime = now;
    executeSunTracking();
  }

  // 4. Heartbeat LED (Toggles every 1s when running normally)
  if (now - lastHeartbeatTime >= 1000) {
    lastHeartbeatTime = now;
    heartbeatState = !heartbeatState;
    digitalWrite(PIN_STATUS_LED, heartbeatState ? LOW : HIGH);
  }
}

// =============================================================================
// POWER SENSING (SOLAR & BATTERY SIDES)
// =============================================================================
void updatePowerSensors() {
  unsigned long now = millis();
  if (now - lastPowerReadTime < 500) return;
  lastPowerReadTime = now;

  // 1. Read Solar Voltage (PA6) with 8x oversampling
  long sumSV = 0;
  for (int i = 0; i < 8; i++) {
    sumSV += analogRead(PIN_SOLAR_VOLT);
    delayMicroseconds(40);
  }
  float adcVoltS = ((sumSV / 8.0f) * refVoltage) / 4095.0f;
  solarVoltage = adcVoltS * VOLT_DIVIDER_RATIO;
  if (solarVoltage < 0.15f) solarVoltage = 0.0f;

  // 2. Read Solar Current (PA7) with 8x oversampling
  long sumSC = 0;
  for (int i = 0; i < 8; i++) {
    sumSC += analogRead(PIN_SOLAR_CURR);
    delayMicroseconds(40);
  }
  float adcCurrS = ((sumSC / 8.0f) * refVoltage) / 4095.0f;
  // Current I = (V_adc - V_zero) / Sensitivity
  solarCurrent = (adcCurrS - currentZeroOffset) / currentSensitivity;
  if (solarCurrent < 0.05f) solarCurrent = 0.0f; // Filter negative/noise floor

  // 3. Compute Solar Power (Watts)
  solarPower = solarVoltage * solarCurrent;

  // 4. Read Battery Voltage (PB0) & Current (PB1)
  long sumBV = 0;
  for (int i = 0; i < 8; i++) {
    sumBV += analogRead(PIN_BATT_VOLT);
    delayMicroseconds(40);
  }
  float adcVoltB = ((sumBV / 8.0f) * refVoltage) / 4095.0f;
  battVoltage = adcVoltB * VOLT_DIVIDER_RATIO;
  if (battVoltage < 0.15f) battVoltage = 0.0f;

  long sumBC = 0;
  for (int i = 0; i < 8; i++) {
    sumBC += analogRead(PIN_BATT_CURR);
    delayMicroseconds(40);
  }
  float adcCurrB = ((sumBC / 8.0f) * refVoltage) / 4095.0f;
  battCurrent = (adcCurrB - currentZeroOffset) / currentSensitivity;
  if (battCurrent < 0.05f) battCurrent = 0.0f;

  battPower = battVoltage * battCurrent;
}

// =============================================================================
// DHT11 SENSOR READ (EVERY 2.5 SECONDS)
// =============================================================================
void updateDhtSensors() {
  unsigned long now = millis();
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
// ZERO HOMING DATUM (CENTERS ON WIDE MAGNET)
// =============================================================================
void findZeroHomeDatum() {
  printlnAll(F("\n[HOMING] Calibrating ZERO Position (0.0 deg Datum)..."));

  if (digitalRead(PIN_HALL_HOME) == LOW) {
    currentAngle = 0.0f;
    isHomed = true;
    motorOff();
    printlnAll(F("[HOMING] Magnet detected at sensor! ZERO Datum Confirmed: 0.0 deg.\n"));
    return;
  }

  motorOn();
  digitalWrite(PIN_STATUS_LED, LOW);

  bool foundMagnet = false;
  long searchLimit = (long)(45.0f * STEPS_PER_DEGREE);

  digitalWrite(PIN_DIR, LOW);
  for (long s = 0; s < searchLimit; s++) {
    if (digitalRead(PIN_HALL_HOME) == LOW) {
      foundMagnet = true;
      break;
    }
    stepPulse(ZERO_HOMING_SPEED_US);
  }

  if (!foundMagnet) {
    digitalWrite(PIN_DIR, HIGH);
    long fullSweep = (long)(90.0f * STEPS_PER_DEGREE);
    for (long s = 0; s < fullSweep; s++) {
      if (digitalRead(PIN_HALL_HOME) == LOW) {
        foundMagnet = true;
        break;
      }
      stepPulse(ZERO_HOMING_SPEED_US);
    }
  }

  if (foundMagnet) {
    long spanSteps = 0;
    long maxSpan = (long)(20.0f * STEPS_PER_DEGREE);
    uint8_t movingDir = digitalRead(PIN_DIR);

    while (digitalRead(PIN_HALL_HOME) == LOW && spanSteps < maxSpan) {
      stepPulse(ZERO_HOMING_SPEED_US);
      spanSteps++;
    }

    long centerSteps = spanSteps / 2;
    if (centerSteps > 0) {
      digitalWrite(PIN_DIR, (movingDir == HIGH) ? LOW : HIGH);
      for (long s = 0; s < centerSteps; s++) {
        stepPulse(ZERO_HOMING_SPEED_US);
      }
    }

    currentAngle = 0.0f;
    isHomed = true;
    printlnAll(F("[HOMING] Magnet centered successfully! ZERO Position LOCKED at 0.0 deg."));
  } else {
    currentAngle = 0.0f;
    isHomed = true;
    printlnAll(F("[WARN] Hall sensor not reached. Position synchronized to 0.0 deg."));
  }

  motorOff();
  digitalWrite(PIN_STATUS_LED, HIGH);
  printlnAll(F("--------------------------------------------------------"));
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

  int avgTop    = (lightT1 + lightT2) / 2;
  int avgBottom = (lightB1 + lightB2) / 2;
  int maxLight  = max(avgTop, avgBottom);

  int diff = avgTop - avgBottom;
  if (invertMotorDir) diff = -diff;

  const char* trackingState = "BALANCED";

  if (maxLight < nightDarkThreshold) {
    if (darknessStartMs == 0) darknessStartMs = millis();
    unsigned long darkDuration = millis() - darknessStartMs;

    if (darkDuration >= NIGHT_PARK_DELAY_MS) {
      if (fabs(currentAngle) > 0.5f) {
        trackingState = "NIGHT CONFIRMED (8s) -> Parking at 0.0 deg";
        if (isAutoTracking) moveToAngle(0.0f, TRACKING_SPEED_US);
      } else {
        trackingState = "NIGHT SLEEP (Parked at 0.0 deg, 0W Coils OFF)";
        motorOff();
      }
    } else {
      trackingState = "LOW LIGHT -> HOLDING POSITION (8s Timer)";
      motorOff();
    }
  } else {
    darknessStartMs = 0;

    if (abs(diff) <= deadbandThreshold) {
      trackingState = "BALANCED -> SUN LOCKED (100% Solar Yield)";
      motorOff();
    }
    else if (diff > deadbandThreshold) {
      if (isAutoTracking) {
        float nextAngle = currentAngle + 1.0f;
        if (nextAngle <= MAX_ANGLE) {
          trackingState = "SUN ON TOP (PA0+PA1) -> Moving (+)";
          moveToAngle(nextAngle, TRACKING_SPEED_US);
        } else {
          trackingState = "LIMIT REACHED (+40.0 deg MAX)";
        }
      } else {
        trackingState = "SUN ON TOP (Manual Mode)";
      }
    }
    else {
      if (isAutoTracking) {
        float nextAngle = currentAngle - 1.0f;
        if (nextAngle >= MIN_ANGLE) {
          trackingState = "SUN ON BOTTOM (PA4+PA5) -> Moving (-)";
          moveToAngle(nextAngle, TRACKING_SPEED_US);
        } else {
          trackingState = "LIMIT REACHED (-40.0 deg MIN)";
        }
      } else {
        trackingState = "SUN ON BOTTOM (Manual Mode)";
      }
    }
  }

  // Print Telemetry every 2 cycles (every 1 second)
  if (frameCount % 2 == 0) {
    printTelemetry(avgTop, avgBottom, diff, trackingState);
  }
}

// =============================================================================
// COMPREHENSIVE TELEMETRY STREAM
// =============================================================================
void printTelemetry(int topVal, int botVal, int diff, const char* stateStr) {
  printlnAll(F("--------------------------------------------------------"));
  printAll(F(" [SLAT ANGLE]   Angle: "));
  if (currentAngle >= 0) printAll(F("+"));
  printAll(currentAngle, 1);
  printAll(F(" deg | Mode: "));
  printAll(isAutoTracking ? F("AUTO") : F("MANUAL"));
  printAll(F(" | Homed: "));
  printlnAll(isHomed ? F("YES (0.0 ZERO)") : F("NO"));

  printAll(F(" [SUN SENSORS]  TOP: ")); printAll(topVal);
  printAll(F(" | BOT: ")); printAll(botVal);
  printAll(F(" | Diff: "));
  if (diff >= 0) printAll(F("+"));
  printAll(diff);
  printAll(F(" (Deadband: +/-")); printAll(deadbandThreshold); printlnAll(F(")"));

  printAll(F(" [SOLAR POWER]  Voltage: ")); printAll(solarVoltage, 2); printAll(F(" V"));
  printAll(F(" | Current: ")); printAll(solarCurrent, 2); printAll(F(" A"));
  printAll(F(" | Power: ")); printAll(solarPower, 2); printlnAll(F(" W"));

  printAll(F(" [BATTERY SIDE] Voltage: ")); printAll(battVoltage, 2); printAll(F(" V"));
  printAll(F(" | Current: ")); printAll(battCurrent, 2); printAll(F(" A"));
  printAll(F(" | Power: ")); printAll(battPower, 2); printlnAll(F(" W"));

  printAll(F(" [ENVIRONMENT]  Temp: ")); printAll(currentTemp, 1); printAll(F(" C"));
  printAll(F(" | Humidity: ")); printAll(currentHumidity, 1); printlnAll(F(" %"));

  printAll(F(" [HARDWARE]     Hall: "));
  printAll(digitalRead(PIN_HALL_HOME) == LOW ? F("MAGNET DETECTED") : F("OPEN"));
  printAll(F(" | Motor: 0W Silent Idle")); printlnAll();

  printAll(F(" [STATUS]       ")); printlnAll(stateStr);
  printlnAll(F("--------------------------------------------------------"));
}

// =============================================================================
// MOTOR KINEMATIC CONTROL
// =============================================================================
void moveToAngle(float targetAngle, int speedUs) {
  if (targetAngle < MIN_ANGLE) targetAngle = MIN_ANGLE;
  if (targetAngle > MAX_ANGLE) targetAngle = MAX_ANGLE;

  float deltaDeg = targetAngle - currentAngle;
  if (fabs(deltaDeg) < 0.05f) return;

  long steps = (long)(fabs(deltaDeg) * STEPS_PER_DEGREE + 0.5f);
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
  delayMicroseconds(delayUs);
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
static char cmdBuf[32];
static byte cmdPos = 0;

void handleCommand(char* cmd) {
  while (*cmd == ' ') cmd++;
  if (*cmd == 0) return;

  if (strcasecmp(cmd, "AUTO") == 0) {
    isAutoTracking = true;
    printlnAll(F("\n[CMD] Auto Sun Tracking ENABLED!\n"));
  }
  else if (strcasecmp(cmd, "MANUAL") == 0 || strcasecmp(cmd, "STOP") == 0) {
    isAutoTracking = false;
    printlnAll(F("\n[CMD] Auto Sun Tracking PAUSED. Manual control active.\n"));
  }
  else if (strcasecmp(cmd, "HOME") == 0) {
    isAutoTracking = false;
    findZeroHomeDatum();
    isAutoTracking = true;
  }
  else if (strcasecmp(cmd, "ZERO") == 0) {
    currentAngle = 0.0f;
    isHomed = true;
    printlnAll(F("\n[CMD] Current position calibrated as 0.0 deg ZERO.\n"));
  }
  else if (strcasecmp(cmd, "INVERT") == 0) {
    invertMotorDir = !invertMotorDir;
    printAll(F("\n[CMD] Tracking direction inverted: "));
    printlnAll(invertMotorDir ? F("REVERSED") : F("NORMAL"));
  }
  else if (strncasecmp(cmd, "SPEED_TRACK ", 12) == 0) {
    int val = atoi(cmd + 12);
    if (val >= 800 && val <= 5000) {
      TRACKING_SPEED_US = val;
      printAll(F("\n[CMD] Tracking Speed updated to: "));
      printAll(TRACKING_SPEED_US); printlnAll(F(" us"));
    }
  }
  else if (strncasecmp(cmd, "SPEED_HOME ", 11) == 0) {
    int val = atoi(cmd + 11);
    if (val >= 800 && val <= 5000) {
      ZERO_HOMING_SPEED_US = val;
      printAll(F("\n[CMD] ZERO Homing Speed updated to: "));
      printAll(ZERO_HOMING_SPEED_US); printlnAll(F(" us"));
    }
  }
  else if (strncasecmp(cmd, "VREF ", 5) == 0) {
    float val = parseCustomFloat(cmd + 5);
    if (val > 2.0f && val < 5.5f) {
      refVoltage = val;
      printAll(F("\n[CMD] ADC VREF calibrated to: "));
      printAll(refVoltage, 2); printlnAll(F(" V"));
    }
  }
  else if (strncasecmp(cmd, "DEADBAND ", 9) == 0) {
    int val = atoi(cmd + 9);
    if (val >= 5 && val <= 1000) {
      deadbandThreshold = val;
      printAll(F("\n[CMD] Deadband updated to: "));
      printlnAll(deadbandThreshold);
    }
  }
  else if (strcasecmp(cmd, "STATUS") == 0) {
    printlnAll(F("\n--- SYSTEM PARAMETERS ---"));
    printAll(F(" Angle          : ")); printAll(currentAngle, 1); printlnAll(F(" deg"));
    printAll(F(" Homed (ZERO)   : ")); printlnAll(isHomed ? F("YES (0.0 deg)") : F("NO"));
    printAll(F(" Tracking Mode  : ")); printlnAll(isAutoTracking ? F("AUTO") : F("MANUAL"));
    printAll(F(" Tracking Speed : ")); printAll(TRACKING_SPEED_US); printlnAll(F(" us"));
    printAll(F(" Homing Speed   : ")); printAll(ZERO_HOMING_SPEED_US); printlnAll(F(" us"));
    printAll(F(" Solar Voltage  : ")); printAll(solarVoltage, 2); printlnAll(F(" V"));
    printAll(F(" Solar Current  : ")); printAll(solarCurrent, 2); printlnAll(F(" A"));
    printAll(F(" Solar Power    : ")); printAll(solarPower, 2); printlnAll(F(" W"));
    printAll(F(" Battery Volt   : ")); printAll(battVoltage, 2); printlnAll(F(" V"));
    printAll(F(" Battery Curr   : ")); printAll(battCurrent, 2); printlnAll(F(" A"));
    printAll(F(" Temperature    : ")); printAll(currentTemp, 1); printlnAll(F(" C"));
    printAll(F(" Humidity       : ")); printAll(currentHumidity, 1); printlnAll(F(" %"));
    printAll(F(" Steps/Deg      : ")); printlnAll(STEPS_PER_DEGREE, 4);
    printAll(F(" Deadband       : ")); printlnAll(deadbandThreshold);
    printAll(F(" Hall Magnet    : ")); printlnAll(digitalRead(PIN_HALL_HOME) == LOW ? F("DETECTED") : F("OPEN"));
    printlnAll(F("-------------------------\n"));
  }
  else if (strcasecmp(cmd, "HELP") == 0 || strcmp(cmd, "?") == 0) {
    printHelp();
  }
  else {
    float target = 0.0f;
    bool isAngle = false;

    if (strncasecmp(cmd, "GOTO ", 5) == 0) {
      target = parseCustomFloat(cmd + 5);
      isAngle = true;
    } else if (strncasecmp(cmd, "MOVE ", 5) == 0) {
      target = parseCustomFloat(cmd + 5);
      isAngle = true;
    } else if (*cmd == '-' || *cmd == '+' || isdigit(*cmd)) {
      target = parseCustomFloat(cmd);
      isAngle = true;
    }

    if (isAngle) {
      if (target < MIN_ANGLE || target > MAX_ANGLE) {
        printAll(F("\n[ERROR] Target ")); printAll(target, 1);
        printlnAll(F(" deg outside safe range [-40 to +40 deg]!\n"));
        return;
      }
      isAutoTracking = false;
      printlnAll(F("\n[MANUAL] Moving to commanded angle..."));
      moveToAngle(target, TRACKING_SPEED_US);
      printlnAll(F("[MANUAL] Reached. (Type 'AUTO' to resume sun tracking)\n"));
    }
  }
}

void processSerialInput() {
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\r') continue;
    if (c == '\n') {
      cmdBuf[cmdPos] = '\0';
      if (cmdPos > 0) {
        handleCommand(cmdBuf);
        cmdPos = 0;
      }
    } else if (cmdPos < sizeof(cmdBuf) - 1) {
      cmdBuf[cmdPos++] = c;
    }
  }

#if HAS_USB_SERIAL
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      cmdBuf[cmdPos] = '\0';
      if (cmdPos > 0) {
        handleCommand(cmdBuf);
        cmdPos = 0;
      }
    } else if (cmdPos < sizeof(cmdBuf) - 1) {
      cmdBuf[cmdPos++] = c;
    }
  }
#endif
}

void printHelp() {
  printlnAll(F("\n========================================================"));
  printlnAll(F(" POWER IQ — SUN TRACKER SERIAL COMMANDS                 "));
  printlnAll(F("========================================================"));
  printlnAll(F(" AUTO             : Enable continuous automatic sun tracking"));
  printlnAll(F(" MANUAL           : Pause auto tracking (hold current angle)"));
  printlnAll(F(" GOTO <deg>       : Move slats to specific angle (-40 to +40)"));
  printlnAll(F(" ZERO             : Calibrate current position as 0.0 deg  "));
  printlnAll(F(" HOME             : Re-run Hall-effect ZERO calibration    "));
  printlnAll(F(" SPEED_TRACK <us> : Adjust sun tracking speed (default: 2000)"));
  printlnAll(F(" SPEED_HOME <us>  : Adjust ZERO homing speed (default: 2500)"));
  printlnAll(F(" VREF <float>     : Calibrate ADC VREF voltage (default: 3.3)"));
  printlnAll(F(" INVERT           : Flip motor tracking direction (+/-)    "));
  printlnAll(F(" DEADBAND <n>     : Adjust optical deadband (default: 50)  "));
  printlnAll(F(" STATUS           : Display system parameters & sensors     "));
  printlnAll(F(" HELP / ?         : Show this instruction guide            "));
  printlnAll(F("========================================================\n"));
}
