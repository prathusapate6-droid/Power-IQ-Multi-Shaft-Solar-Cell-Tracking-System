/*
================================================================================
 POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 Subsystem: STM32 Sun Tracking + Voltage Sensor (33k/6.8k) + DHT11 + I2C LCD
 Platform : STM32F103C8T6 "Blue Pill" (32-bit ARM Cortex-M3 @ 72 MHz)
 Authors  : Prathamesh Sapate (Lead), Shreyash Pachade, Vansh Dobhale, Prachi Ronge
 Purpose  : 100% Pure STM32F103C8T6 ARM Firmware
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
   5. SOLAR PV VOLTAGE SENSOR (Exact User Divider: R1=33k, R2=6.8k):
      - PA6   -> Voltage Sensor Analog Signal (ADC1_IN6)
      - Scaling: R1 = 33000 Ohm, R2 = 6800 Ohm -> Multiplier = 5.853
   6. OPTIONAL I2C 16x2 LCD DISPLAY (0x27):
      - PB6   -> I2C1_SCL
      - PB7   -> I2C1_SDA
   7. PC SERIAL TELEMETRY (USART1 @ 9600 Baud):
      - PA9   -> USB-TTL RX (USART1_TX)
      - PA10  -> USB-TTL TX (USART1_RX)
================================================================================
*/

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// =============================================================================
// 1. USER SPEED & TIMING VARIABLES (TUNE YOUR SPEEDS HERE!)
// =============================================================================
// Pulse delay in microseconds (us) per step:
//   - 2500 us = Safe, full torque, smooth benchmark
//   - 2000 us = Fast, smooth tracking
//   - 1600 us = Very fast tracking
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

// ---------------- DHT11 TEMPERATURE & HUMIDITY ----------------
#define PIN_DHT11       PB5   // DHT11 Data Pin
#define DHTTYPE         DHT11 // Sensor Model: DHT 11

DHT dht(PIN_DHT11, DHTTYPE);

float currentTemp     = 0.0f; // Temperature in deg C
float currentHumidity = 0.0f; // Relative Humidity in %
unsigned long lastDhtReadTime = 0;

// ---------------- SOLAR PV VOLTAGE SENSOR (USER DIVIDER) ----------------
#define PIN_SOLAR_VOLT  PA6   // STM32 ADC1_IN6
const float RESISTOR_R1 = 33000.0f; // 33k Ohm
const float RESISTOR_R2 =  6800.0f; // 6.8k Ohm
float refVoltage        = 3.3f;     // STM32 3.3V reference voltage (adjustable)

float solarVoltage      = 0.0f;     // Calculated input voltage (V)
unsigned long lastVoltReadTime = 0;

// ---------------- OPTIONAL I2C 16x2 LCD DISPLAY (0x27) ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
bool isLcdPresent = false;
unsigned long lastLcdUpdateTime = 0;

// ---------------- KINEMATICS & PROVEN BENCH CONSTANTS ----------------
const float STEPS_PER_DEGREE = 10.556f; // 19:1 Worm gear ratio

const float MIN_ANGLE = -40.0f;
const float MAX_ANGLE =  40.0f;

// ---------------- TRACKING THRESHOLDS ----------------
int deadbandThreshold   = 50;   // Deadband tolerance (counts)
int nightDarkThreshold  = 40;   // Below this is darkness / night
const unsigned long NIGHT_PARK_DELAY_MS = 8000; // 8 seconds continuous darkness before parking

bool isAutoTracking   = true;   // Default: Auto Sun Tracking
bool invertMotorDir   = false;  // Toggle if physical direction needs flip

// State Tracking
float currentAngle = 0.0f;
bool  isHomed      = false;     // True only after 0.0 deg ZERO datum is calibrated
unsigned long lastTrackTime = 0;
unsigned long darknessStartMs = 0;
unsigned long frameCount = 0;

// ---------------- DUAL-SERIAL OUTPUT HELPERS ----------------
#if defined(Serial1)
  #define HAS_SERIAL1 1
#else
  #define HAS_SERIAL1 0
#endif

template<typename T>
void printAll(T msg) {
  Serial.print(msg);
#if HAS_SERIAL1
  Serial1.print(msg);
#endif
}

template<typename T, typename P>
void printAll(T msg, P p) {
  Serial.print(msg, p);
#if HAS_SERIAL1
  Serial1.print(msg, p);
#endif
}

inline void printlnAll() {
  Serial.println();
#if HAS_SERIAL1
  Serial1.println();
#endif
}

template<typename T>
void printlnAll(T msg) {
  Serial.println(msg);
#if HAS_SERIAL1
  Serial1.println(msg);
#endif
}

template<typename T, typename P>
void printlnAll(T msg, P p) {
  Serial.println(msg, p);
#if HAS_SERIAL1
  Serial1.println(msg, p);
#endif
}

// ---------------- FORWARD DECLARATIONS ----------------
void motorOn();
void motorOff();
void stepPulse(int delayUs);
void moveToAngle(float targetAngle, int speedUs = 0);
void findZeroHomeDatum();
void updateDhtSensors();
void updateVoltageSensor();
void updateLcdDisplay(const char* stateStr);
void processSerialInput();
void executeSunTracking();
void printTelemetry(int topVal, int botVal, int diff, const char* stateStr);
void printHelp();

// =============================================================================
// SETUP
// =============================================================================
void setup() {
  // 1. Motor Driver Pins
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  motorOff(); // Start 100% silent and cool (0W)

  // 2. Status LED & Sensor Pins
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW); // LED ON at boot

  pinMode(PIN_LDR_TOP1, INPUT);
  pinMode(PIN_LDR_TOP2, INPUT);
  pinMode(PIN_LDR_BOT1, INPUT);
  pinMode(PIN_LDR_BOT2, INPUT);
  pinMode(PIN_HALL_HOME, INPUT_PULLUP);
  pinMode(PIN_SOLAR_VOLT, INPUT);

  // 3. Initialize DHT11 Sensor
  dht.begin();

  // 4. Initialize I2C Bus & Detect Optional LCD (0x27)
  Wire.begin();
  Wire.beginTransmission(0x27);
  if (Wire.endTransmission() == 0) {
    isLcdPresent = true;
    lcd.begin();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("POWER IQ TRACKER");
    lcd.setCursor(0, 1);
    lcd.print("Calibrating 0.0...");
  }

  // 5. Serial Communication @ 9600 Baud
  Serial.begin(9600);
#if HAS_SERIAL1
  Serial1.begin(9600);
#endif

  delay(1000);

  printlnAll();
  printlnAll(F("============================================================================"));
  printlnAll(F(" POWER IQ — STM32 SUN TRACKER + VOLTAGE (33k/6.8k) + DHT11 + I2C LCD        "));
  printlnAll(F(" Platform     : STM32F103C8T6 32-bit ARM Cortex-M3 @ 72 MHz (100% Pure)     "));
  printlnAll(F(" Calibration  : 10.556 steps/deg | Safety Limits: -40.0 to +40.0 deg        "));
  printAll(F(" Tracking Spd : ")); printAll(TRACKING_SPEED_US);
  printAll(F(" us | ZERO Homing Spd: ")); printAll(ZERO_HOMING_SPEED_US); printlnAll(F(" us"));
  printlnAll(F(" Sensors      : 4x LDRs, Hall (PB11), DHT11 (PB5), Voltage PA6 (33k/6.8k)   "));
  printlnAll(F("============================================================================"));

  // 6. Initial Sensor Readings
  updateDhtSensors();
  updateVoltageSensor();

  // 7. GUARANTEED ZERO POSITIONING AT STARTUP
  findZeroHomeDatum();

  digitalWrite(PIN_STATUS_LED, HIGH); // LED OFF (Ready)
  printlnAll(F("[READY] ZERO Established! Sun Tracking ACTIVE. Type 'HELP' for commands.\n"));
}

// =============================================================================
// AUTOMATED ZERO POSITIONING (CENTERS ON HALL MAGNET DATUM)
// =============================================================================
void findZeroHomeDatum() {
  printlnAll(F("\n[HOMING] Calibrating ZERO Position (0.0 deg Datum)..."));

  // Check if magnet is already sitting at the sensor
  if (digitalRead(PIN_HALL_HOME) == LOW) {
    currentAngle = 0.0f;
    isHomed = true;
    motorOff();
    printlnAll(F("[HOMING] Magnet already at sensor! ZERO Datum Confirmed: 0.0 deg.\n"));
    return;
  }

  // Energize motor with proven 5ms charge pump wakeup
  motorOn();
  digitalWrite(PIN_STATUS_LED, LOW); // LED ON during zeroing

  bool foundMagnet = false;
  long searchLimit = (long)(45.0f * STEPS_PER_DEGREE); // Search up to 45 deg

  // Phase 1: Search in Direction A (Negative)
  digitalWrite(PIN_DIR, LOW);
  for (long s = 0; s < searchLimit; s++) {
    if (digitalRead(PIN_HALL_HOME) == LOW) {
      foundMagnet = true;
      break;
    }
    stepPulse(ZERO_HOMING_SPEED_US);
  }

  // Phase 2: If not found, sweep in Direction B (Positive) across 80 deg travel window
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
    // Phase 3: Centering inside magnet width (Eliminates 0-10 deg magnet span error)
    long spanSteps = 0;
    long maxSpan = (long)(20.0f * STEPS_PER_DEGREE); // Max 20 deg span
    uint8_t movingDir = digitalRead(PIN_DIR);

    while (digitalRead(PIN_HALL_HOME) == LOW && spanSteps < maxSpan) {
      stepPulse(ZERO_HOMING_SPEED_US);
      spanSteps++;
    }

    // Step back by half span to land dead-center in the magnetic field!
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
    // Fallback: If hall sensor is disconnected or magnet absent
    currentAngle = 0.0f;
    isHomed = true;
    printlnAll(F("[WARN] Hall sensor not reached. Position synchronized to 0.0 deg."));
  }

  motorOff(); // 100% silent (0W)
  digitalWrite(PIN_STATUS_LED, HIGH);
  printlnAll(F("----------------------------------------------------------------------------"));
}

// =============================================================================
// MAIN LOOP
// =============================================================================
void loop() {
  // 1. Process any incoming Serial commands
  processSerialInput();

  // 2. Periodic Sensor Updates (Non-blocking)
  updateDhtSensors();
  updateVoltageSensor();

  // 3. Closed-Loop Sun Tracking (every 500ms)
  unsigned long now = millis();
  if (now - lastTrackTime >= 500) {
    lastTrackTime = now;
    executeSunTracking();
  }
}

// =============================================================================
// DHT11 NON-BLOCKING SENSOR READ (EVERY 2.5 SECONDS)
// =============================================================================
void updateDhtSensors() {
  unsigned long now = millis();
  if (now - lastDhtReadTime >= 2500) {
    lastDhtReadTime = now;
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      currentTemp = t;
      currentHumidity = h;
    }
  }
}

// =============================================================================
// SOLAR PV VOLTAGE SENSOR (EXACT USER DIVIDER: R1=33k, R2=6.8k)
// =============================================================================
void updateVoltageSensor() {
  unsigned long now = millis();
  if (now - lastVoltReadTime >= 500) {
    lastVoltReadTime = now;
    long sumV = 0;
    for (int i = 0; i < 8; i++) {
      sumV += analogRead(PIN_SOLAR_VOLT);
      delayMicroseconds(50);
    }
    int rawV = sumV / 8;
    float adcVolt = (rawV * refVoltage) / 4095.0f;
    // V_in = V_adc / (R2 / (R1 + R2)) = V_adc * (R1 + R2) / R2
    solarVoltage = adcVolt / (RESISTOR_R2 / (RESISTOR_R1 + RESISTOR_R2));
    if (solarVoltage < 0.15f) solarVoltage = 0.0f; // Noise filter floor
  }
}

// =============================================================================
// OPTIONAL I2C 16x2 LCD DISPLAY REFRESH
// =============================================================================
void updateLcdDisplay(const char* stateStr) {
  if (!isLcdPresent) return;
  unsigned long now = millis();
  if (now - lastLcdUpdateTime < 800) return;
  lastLcdUpdateTime = now;

  // Line 0: Voltage & Angle
  lcd.setCursor(0, 0);
  lcd.print("V:");
  lcd.print(solarVoltage, 1);
  lcd.print("V A:");
  if (currentAngle >= 0) lcd.print("+");
  lcd.print(currentAngle, 0);
  lcd.print((char)223); // degree sign
  lcd.print("   ");

  // Line 1: Temp & Humidity
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print((int)currentTemp);
  lcd.print("C H:");
  lcd.print((int)currentHumidity);
  lcd.print("% ");
  if (isAutoTracking) lcd.print("AUTO");
  else lcd.print("MAN ");
}

// =============================================================================
// CLOSED-LOOP SUN TRACKING CORE ALGORITHM
// =============================================================================
void executeSunTracking() {
  // Ensure ZERO datum is known before tracking
  if (!isHomed) {
    findZeroHomeDatum();
    return;
  }

  frameCount++;

  // 1. Read 4 LDRs with 4x oversampling
  long sumT1 = 0, sumT2 = 0, sumB1 = 0, sumB2 = 0;
  for (int i = 0; i < 4; i++) {
    sumT1 += analogRead(PIN_LDR_TOP1);
    sumT2 += analogRead(PIN_LDR_TOP2);
    sumB1 += analogRead(PIN_LDR_BOT1);
    sumB2 += analogRead(PIN_LDR_BOT2);
    delayMicroseconds(150);
  }
  int rawT1 = sumT1 / 4;
  int rawT2 = sumT2 / 4;
  int rawB1 = sumB1 / 4;
  int rawB2 = sumB2 / 4;

  // 2. Active-LOW Inversion: 4095 = Dark, ~600 = Direct Bright Light
  // Real Light Intensity = 4095 - rawADC (Higher number = Brighter Light!)
  int lightT1 = 4095 - rawT1;
  int lightT2 = 4095 - rawT2;
  int lightB1 = 4095 - rawB1;
  int lightB2 = 4095 - rawB2;

  if (lightT1 < 0) lightT1 = 0;
  if (lightT2 < 0) lightT2 = 0;
  if (lightB1 < 0) lightB1 = 0;
  if (lightB2 < 0) lightB2 = 0;

  // 3. Compute Sector Light Levels
  int avgTop    = (lightT1 + lightT2) / 2;
  int avgBottom = (lightB1 + lightB2) / 2;
  int maxLight  = max(avgTop, avgBottom);

  // 4. Differential: Positive = TOP is brighter, Negative = BOTTOM is brighter
  int diff = avgTop - avgBottom;
  if (invertMotorDir) diff = -diff;

  // 5. Decision Engine
  const char* trackingState = "BALANCED";

  // CASE A: ROOM IS DARK (NIGHT DETECTION)
  if (maxLight < nightDarkThreshold) {
    if (darknessStartMs == 0) {
      darknessStartMs = millis(); // Start timing darkness
    }

    unsigned long darkDuration = millis() - darknessStartMs;
    if (darkDuration >= NIGHT_PARK_DELAY_MS) {
      // 8 full seconds of darkness confirmed -> Return to 0.0 Home
      if (abs(currentAngle) > 0.5f) {
        trackingState = "NIGHT CONFIRMED (8s) -> Parking at 0.0 deg";
        if (isAutoTracking) moveToAngle(0.0f, TRACKING_SPEED_US);
      } else {
        trackingState = "NIGHT SLEEP (Parked at 0.0 deg, 0W Coils OFF)";
        motorOff();
      }
    } else {
      // 0 to 8 seconds: FIRMLY HOLD CURRENT POSITION!
      trackingState = "LOW LIGHT -> HOLDING POSITION (8s Timer)";
      motorOff(); // 0W coils off, worm gear locks angle
    }
  }
  // CASE B: LIGHT DETECTED
  else {
    darknessStartMs = 0; // Reset night timer immediately

    if (abs(diff) <= deadbandThreshold) {
      // Both sectors balanced within deadband -> Perpendicular to sun!
      trackingState = "BALANCED -> SUN LOCKED (100% Solar Yield)";
      motorOff(); // Coils 0W silent holding
    }
    else if (diff > deadbandThreshold) {
      // TOP (PA0+PA1) has more light -> Move 1.0 deg towards TOP (+)
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
      // BOTTOM (PA4+PA5) has more light -> Move 1.0 deg towards BOTTOM (-)
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

  // 6. Update LCD Display if present
  updateLcdDisplay(trackingState);

  // 7. Print Telemetry every 2 cycles (every 1 second)
  if (frameCount % 2 == 0) {
    printTelemetry(avgTop, avgBottom, diff, trackingState);
  }
}

// =============================================================================
// TELEMETRY OUTPUT TO SERIAL MONITOR
// =============================================================================
void printTelemetry(int topVal, int botVal, int diff, const char* stateStr) {
  printlnAll(F("----------------------------------------------------------------------------"));
  printAll(F(" [POWER IQ] Angle: "));
  if (currentAngle >= 0) printAll(F("+"));
  printAll(currentAngle, 1);
  printAll(F(" deg | Mode: "));
  printAll(isAutoTracking ? F("AUTO") : F("MANUAL"));
  printAll(F(" | Homed: "));
  printlnAll(isHomed ? F("YES (0.0 ZERO)") : F("NO"));

  printAll(F(" TOP (PA0+PA1): ")); printAll(topVal);
  printAll(F(" | BOT (PA4+PA5): ")); printAll(botVal);
  printAll(F(" | Diff: "));
  if (diff >= 0) printAll(F("+"));
  printAll(diff);
  printAll(F(" (Deadband: +/-")); printAll(deadbandThreshold); printlnAll(F(")"));

  printAll(F(" Environment : Temp = ")); printAll(currentTemp, 1);
  printAll(F(" deg C | Humidity = ")); printAll(currentHumidity, 1); printlnAll(F(" %"));

  printAll(F(" Solar Yield  : PV Voltage = ")); printAll(solarVoltage, 2); printlnAll(F(" V"));

  printAll(F(" Hall Magnet : "));
  printAll(digitalRead(PIN_HALL_HOME) == LOW ? F("DETECTED (0.0 HOME)") : F("OPEN"));
  printAll(F(" | Motor: "));
  printlnAll(F("0W Silent Holding"));

  printAll(F(" Status      : ")); printlnAll(stateStr);
  printlnAll(F("----------------------------------------------------------------------------"));
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

  // 1. Energize Motor Coils & allow charge pump to stabilize
  motorOn();
  digitalWrite(PIN_STATUS_LED, LOW); // Active LOW: LED ON

  // 2. Set Direction
  digitalWrite(PIN_DIR, (deltaDeg > 0) ? HIGH : LOW);

  // 3. Step Pulses with selected speed
  for (long i = 0; i < steps; i++) {
    stepPulse(activeSpeed);
  }

  // 4. Update Position
  currentAngle = targetAngle;

  // 5. Automatic Coil De-energization: 100% silent, 0W idle power, no buzzing!
  motorOff();
  digitalWrite(PIN_STATUS_LED, HIGH); // LED OFF
}

void stepPulse(int delayUs) {
  digitalWrite(PIN_STEP, HIGH);
  delayMicroseconds(delayUs);
  digitalWrite(PIN_STEP, LOW);
  delayMicroseconds(delayUs);
}

void motorOn() {
  digitalWrite(PIN_ENABLE, LOW); // LOW = A4988 Active
  delay(5);                      // 5ms stabilization time for A4988 charge pump
}

void motorOff() {
  digitalWrite(PIN_ENABLE, HIGH); // HIGH = Coils Disconnected (0W)
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
  else if (strncasecmp(cmd, "SPEED ", 6) == 0) {
    int val = atoi(cmd + 6);
    if (val >= 800 && val <= 5000) {
      TRACKING_SPEED_US = val;
      ZERO_HOMING_SPEED_US = val;
      printAll(F("\n[CMD] Both Tracking & Homing speeds updated to: "));
      printAll(val); printlnAll(F(" us"));
    }
  }
  else if (strncasecmp(cmd, "VREF ", 5) == 0) {
    float val = atof(cmd + 5);
    if (val > 2.0f && val < 5.5f) {
      refVoltage = val;
      printAll(F("\n[CMD] Voltage Reference calibrated to: "));
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
    printlnAll(F("\n--- SYSTEM STATUS ---"));
    printAll(F(" Angle          : ")); printAll(currentAngle, 1); printlnAll(F(" deg"));
    printAll(F(" Homed (ZERO)   : ")); printlnAll(isHomed ? F("YES (0.0 deg)") : F("NO"));
    printAll(F(" Tracking Mode  : ")); printlnAll(isAutoTracking ? F("AUTO") : F("MANUAL"));
    printAll(F(" Tracking Speed : ")); printAll(TRACKING_SPEED_US); printlnAll(F(" us"));
    printAll(F(" Homing Speed   : ")); printAll(ZERO_HOMING_SPEED_US); printlnAll(F(" us"));
    printAll(F(" Temperature    : ")); printAll(currentTemp, 1); printlnAll(F(" deg C"));
    printAll(F(" Humidity       : ")); printAll(currentHumidity, 1); printlnAll(F(" %"));
    printAll(F(" Solar PV Volt  : ")); printAll(solarVoltage, 2); printlnAll(F(" V (R1=33k, R2=6.8k)"));
    printAll(F(" VREF Voltage   : ")); printAll(refVoltage, 2); printlnAll(F(" V"));
    printAll(F(" LCD 16x2 (I2C) : ")); printlnAll(isLcdPresent ? F("CONNECTED (0x27)") : F("NOT DETECTED"));
    printAll(F(" Steps/Deg      : ")); printlnAll(STEPS_PER_DEGREE, 4);
    printAll(F(" Deadband       : ")); printlnAll(deadbandThreshold);
    printAll(F(" Hall Magnet    : ")); printlnAll(digitalRead(PIN_HALL_HOME) == LOW ? F("DETECTED") : F("OPEN"));
    printlnAll(F("---------------------\n"));
  }
  else if (strcasecmp(cmd, "HELP") == 0 || strcmp(cmd, "?") == 0) {
    printHelp();
  }
  else {
    float target = 0.0f;
    bool isAngle = false;

    if (strncasecmp(cmd, "GOTO ", 5) == 0) {
      target = atof(cmd + 5);
      isAngle = true;
    } else if (strncasecmp(cmd, "MOVE ", 5) == 0) {
      target = atof(cmd + 5);
      isAngle = true;
    } else if (*cmd == '-' || *cmd == '+' || isdigit(*cmd)) {
      target = atof(cmd);
      isAngle = true;
    }

    if (isAngle) {
      if (target < MIN_ANGLE || target > MAX_ANGLE) {
        printAll(F("\n[ERROR] Target ")); printAll(target, 1);
        printlnAll(F(" deg is outside safe range [-40.0 deg to +40.0 deg]!\n"));
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
  Stream* activeStream = nullptr;
  if (Serial.available()) {
    activeStream = &Serial;
  }
#if HAS_SERIAL1
  else if (Serial1.available()) {
    activeStream = &Serial1;
  }
#endif

  if (activeStream == nullptr) return;

  while (activeStream->available()) {
    char c = activeStream->read();
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
}

// =============================================================================
// HELP MENU
// =============================================================================
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
