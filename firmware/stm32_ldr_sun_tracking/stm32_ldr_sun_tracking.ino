/*
================================================================================
 POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 Subsystem: STM32 Rock-Solid Closed-Loop Sun Tracker
 Platform : STM32F103C8T6 "Blue Pill" (32-bit ARM Cortex-M3 @ 72 MHz)
 Authors  : Prathamesh Sapate (Lead), Shreyash Pachade, Vansh Dobhale, Prachi Ronge
 Purpose  : 100% Stable, Non-Blocking, Bench-Proven Sun Tracking Firmware
================================================================================

 BENCH-PROVEN CONSTANTS (NEVER COMPROMISED):
   - Steps per Degree : 10.556 steps/deg (19:1 Worm Gear Ratio)
   - Pulse Delay      : 2500 us (2.5ms HIGH + 2.5ms LOW) -> 100% Torque, Zero Stalling!
   - Driver Wakeup    : 5 ms delay in motorOn() for A4988 charge pump stabilization
   - Coil De-energize : ENABLE = HIGH when stationary -> 100% Silent, 0W Idle Power
   - Range Envelope   : -40.0 deg to +40.0 deg (80.0 deg Total Travel)

 SENSOR ORIENTATION:
   - TOP SECTOR    : PA0 + PA1 (Top Sensors)
   - BOTTOM SECTOR : PA4 + PA5 (Bottom Sensors)
   - When TOP has more light    -> Slats tilt towards TOP (PA0+PA1)
   - When BOTTOM has more light -> Slats tilt towards BOTTOM (PA4+PA5)
   - When light is equal        -> BALANCED (100% Solar Yield, Coils 0W)
   - When room is dark          -> Holds position 8s, then smoothly parks at 0.0°
================================================================================
*/

#include <Arduino.h>

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

// ---------------- KINEMATICS & PROVEN BENCH SETTINGS ----------------
const float STEPS_PER_DEGREE = 10.556f;
const int   STEP_PULSE_DELAY_US = 2500; // Proven 2500us for full stepper torque

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
void stepPulse();
void moveToAngle(float targetAngle);
void processSerialInput();
void executeSunTracking();
void runHomingRoutine();
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

  // Immediately ensure motor is 100% silent and cool (0W)
  motorOff();

  // 2. Status LED & Sensor Pins
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW); // LED ON at boot

  pinMode(PIN_LDR_TOP1, INPUT);
  pinMode(PIN_LDR_TOP2, INPUT);
  pinMode(PIN_LDR_BOT1, INPUT);
  pinMode(PIN_LDR_BOT2, INPUT);
  pinMode(PIN_HALL_HOME, INPUT_PULLUP);

  // 3. Serial Communication @ 9600 Baud
  Serial.begin(9600);
#if HAS_SERIAL1
  Serial1.begin(9600);
#endif

  delay(1000);

  printlnAll();
  printlnAll(F("============================================================================"));
  printlnAll(F(" POWER IQ — ROCK-SOLID STM32 SUN TRACKER (PROVEN BENCH CALIBRATION)        "));
  printlnAll(F(" Calibration  : 10.556 steps/deg | Pulse Delay: 2500 us (Full Torque)       "));
  printlnAll(F(" Orientation  : TOP (PA0+PA1) vs BOTTOM (PA4+PA5)                           "));
  printlnAll(F(" Safety Range : -40.0 deg to +40.0 deg (80.0 deg Total Travel)               "));
  printlnAll(F(" Startup State: Slats at 0.0 deg. Coils 0W OFF. AUTO Tracking ACTIVE!       "));
  printlnAll(F("============================================================================"));

  // Check if Hall magnet is currently at 0.0 datum
  if (digitalRead(PIN_HALL_HOME) == LOW) {
    printlnAll(F("[INFO] Hall sensor magnet detected at 0.0 deg Home Datum.\n"));
  }

  digitalWrite(PIN_STATUS_LED, HIGH); // LED OFF (Ready)
  printlnAll(F("[READY] Sun tracking running! Type 'HELP' in Serial Monitor for commands.\n"));
}

// =============================================================================
// MAIN LOOP
// =============================================================================
void loop() {
  // 1. Process any incoming Serial commands immediately
  processSerialInput();

  // 2. Non-blocking tracking evaluation tick (every 500ms)
  unsigned long now = millis();
  if (now - lastTrackTime >= 500) {
    lastTrackTime = now;
    executeSunTracking();
  }
}

// =============================================================================
// CLOSED-LOOP SUN TRACKING CORE ALGORITHM
// =============================================================================
void executeSunTracking() {
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
        if (isAutoTracking) moveToAngle(0.0f);
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
          moveToAngle(nextAngle);
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
          moveToAngle(nextAngle);
        } else {
          trackingState = "LIMIT REACHED (-40.0 deg MIN)";
        }
      } else {
        trackingState = "SUN ON BOTTOM (Manual Mode)";
      }
    }
  }

  // 6. Print Telemetry every 2 cycles (every 1 second)
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
  printAll(F(" | Frame #")); printlnAll(frameCount);

  printAll(F(" TOP (PA0+PA1): ")); printAll(topVal);
  printAll(F(" | BOT (PA4+PA5): ")); printAll(botVal);
  printAll(F(" | Diff: "));
  if (diff >= 0) printAll(F("+"));
  printAll(diff);
  printAll(F(" (Deadband: +/-")); printAll(deadbandThreshold); printlnAll(F(")"));

  printAll(F(" Hall Magnet : "));
  printAll(digitalRead(PIN_HALL_HOME) == LOW ? F("DETECTED (0.0 HOME)") : F("OPEN"));
  printAll(F(" | Motor: "));
  printlnAll(F("0W Silent Holding"));

  printAll(F(" Status      : ")); printlnAll(stateStr);
  printlnAll(F("----------------------------------------------------------------------------"));
}

// =============================================================================
// MOTOR CONTROL (BENCH CALIBRATED TO 2500us PULSES & FULL TORQUE)
// =============================================================================
void moveToAngle(float targetAngle) {
  if (targetAngle < MIN_ANGLE) targetAngle = MIN_ANGLE;
  if (targetAngle > MAX_ANGLE) targetAngle = MAX_ANGLE;

  float deltaDeg = targetAngle - currentAngle;
  if (fabs(deltaDeg) < 0.05f) return;

  long steps = (long)(fabs(deltaDeg) * STEPS_PER_DEGREE + 0.5f);
  if (steps == 0) {
    currentAngle = targetAngle;
    return;
  }

  // 1. Energize Motor Coils & allow charge pump to stabilize
  motorOn();
  digitalWrite(PIN_STATUS_LED, LOW); // Active LOW: LED ON

  // 2. Set Direction
  digitalWrite(PIN_DIR, (deltaDeg > 0) ? HIGH : LOW);

  // 3. Step Pulses with proven 2500us delay (Full Torque)
  for (long i = 0; i < steps; i++) {
    stepPulse();
  }

  // 4. Update Position
  currentAngle = targetAngle;

  // 5. Automatic Coil De-energization: 100% silent, 0W idle power, no buzzing!
  motorOff();
  digitalWrite(PIN_STATUS_LED, HIGH); // LED OFF
}

void stepPulse() {
  digitalWrite(PIN_STEP, HIGH);
  delayMicroseconds(STEP_PULSE_DELAY_US);
  digitalWrite(PIN_STEP, LOW);
  delayMicroseconds(STEP_PULSE_DELAY_US);
}

void motorOn() {
  digitalWrite(PIN_ENABLE, LOW); // LOW = A4988 Active
  delay(5);                      // 5ms stabilization time for A4988 charge pump
}

void motorOff() {
  digitalWrite(PIN_ENABLE, HIGH); // HIGH = Coils Disconnected (0W)
}

// =============================================================================
// PRECISION HOMING ROUTINE (RUNS ONLY WHEN REQUESTED VIA SERIAL)
// =============================================================================
void runHomingRoutine() {
  printlnAll(F("\n[HOMING] Seeking Hall-effect magnet..."));
  motorOn();
  digitalWrite(PIN_STATUS_LED, LOW);

  // Step towards home
  digitalWrite(PIN_DIR, (currentAngle > 0) ? LOW : HIGH);
  long maxSteps = (long)(60.0f * STEPS_PER_DEGREE);
  bool found = false;

  for (long s = 0; s < maxSteps; s++) {
    if (digitalRead(PIN_HALL_HOME) == LOW) {
      found = true;
      break;
    }
    stepPulse();
  }

  if (found) {
    // Measure magnet width and center
    long span = 0;
    while (digitalRead(PIN_HALL_HOME) == LOW && span < (long)(20.0f * STEPS_PER_DEGREE)) {
      stepPulse();
      span++;
    }
    // Reverse by half span
    uint8_t curDir = (currentAngle > 0) ? LOW : HIGH;
    digitalWrite(PIN_DIR, (curDir == LOW) ? HIGH : LOW);
    for (long s = 0; s < span / 2; s++) {
      stepPulse();
    }
    currentAngle = 0.0f;
    printlnAll(F("[HOMING] SUCCESS! Centered at true 0.0 deg Datum.\n"));
  } else {
    printlnAll(F("[WARN] Magnet not detected. Resetting current position to 0.0 deg.\n"));
    currentAngle = 0.0f;
  }

  motorOff();
  digitalWrite(PIN_STATUS_LED, HIGH);
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
    runHomingRoutine();
    isAutoTracking = true;
  }
  else if (strcasecmp(cmd, "ZERO") == 0) {
    currentAngle = 0.0f;
    printlnAll(F("\n[CMD] Current position calibrated as 0.0 deg.\n"));
  }
  else if (strcasecmp(cmd, "INVERT") == 0) {
    invertMotorDir = !invertMotorDir;
    printAll(F("\n[CMD] Tracking direction inverted: "));
    printlnAll(invertMotorDir ? F("REVERSED") : F("NORMAL"));
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
    printAll(F(" Tracking Mode  : ")); printlnAll(isAutoTracking ? F("AUTO") : F("MANUAL"));
    printAll(F(" Direction Invert: ")); printlnAll(invertMotorDir ? F("YES") : F("NO"));
    printAll(F(" Pulse Delay    : ")); printAll(STEP_PULSE_DELAY_US); printlnAll(F(" us"));
    printAll(F(" Steps/Deg      : ")); printlnAll(STEPS_PER_DEGREE, 4);
    printAll(F(" Deadband       : ")); printlnAll(deadbandThreshold);
    printAll(F(" Night Threshold: ")); printlnAll(nightDarkThreshold);
    printAll(F(" Night Park Wait: ")); printlnAll(F("8 seconds"));
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
      moveToAngle(target);
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
  printlnAll(F(" POWER IQ — ROCK-SOLID SUN TRACKER SERIAL COMMANDS      "));
  printlnAll(F("========================================================"));
  printlnAll(F(" AUTO          : Enable continuous automatic sun tracking"));
  printlnAll(F(" MANUAL        : Pause auto tracking (hold current angle)"));
  printlnAll(F(" GOTO <deg>    : Move slats to specific angle (-40 to +40)"));
  printlnAll(F(" ZERO          : Calibrate current position as 0.0 deg  "));
  printlnAll(F(" HOME          : Run Hall-effect magnet centering homing"));
  printlnAll(F(" INVERT        : Flip motor tracking direction (+/-)    "));
  printlnAll(F(" DEADBAND <n>  : Adjust optical deadband (default: 50)  "));
  printlnAll(F(" STATUS        : Display system parameters & sensors     "));
  printlnAll(F(" HELP / ?      : Show this instruction guide            "));
  printlnAll(F("========================================================\n"));
}
