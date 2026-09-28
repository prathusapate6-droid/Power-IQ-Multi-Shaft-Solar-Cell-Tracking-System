/*
================================================================================
 POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 Subsystem: STM32 Closed-Loop 4-Quadrant LDR Sun Tracking & Stepper Actuator
 Platform : STM32F103C8T6 "Blue Pill" (32-bit ARM Cortex-M3 @ 72 MHz)
 Authors  : Prathamesh Sapate (Lead), Shreyash Pachade, Vansh Dobhale, Prachi Ronge
 Purpose  : High-Performance, Flash-Optimized Closed-Loop Sun Tracking
================================================================================

 HARDWARE WIRING SPECIFICATIONS (100% PURE STM32 BLUE PILL):

 1. A4988 STEPPER MOTOR DRIVER:
    - STM32 Pin PB8   -> A4988 STEP
    - STM32 Pin PB9   -> A4988 DIR
    - STM32 Pin PB10  -> A4988 ENABLE (Active-LOW: 0=Moving, 1=Silent 0W Sleep)
    - STM32 Pin 3.3V  -> A4988 VDD (Logic supply)
    - STM32 Pin GND   -> A4988 GND (Logic GND)
    - Jumper Wire     -> A4988 RESET bridged to SLEEP
    - External 12V +  -> A4988 VMOT (with 100uF capacitor)
    - External 12V -  -> A4988 GND (Motor GND) -> Connected to STM32 GND!
    - NEMA 17 Stepper -> 1A, 1B (Phase A) & 2A, 2B (Phase B)

 2. 4-QUADRANT LDR SUN SENSORS (12-bit ADC1: 0 - 4095):
    - Left / Top-Left (TL)     -> STM32 Pin PA0 (ADC1_IN0)
    - Right / Top-Right (TR)   -> STM32 Pin PA1 (ADC1_IN1)
    - Bottom-Left (BL)         -> STM32 Pin PA4 (ADC1_IN4) [Optional]
    - Bottom-Right (BR)        -> STM32 Pin PA5 (ADC1_IN5) [Optional]
    - LDR VCC                  -> STM32 Pin 3.3V ONLY (NOT 5V!)
    - LDR GND                  -> STM32 GND (via 10k pull-downs for bare LDRs)

 3. ZERO / HOME DATUM SENSOR:
    - Hall-Effect Signal       -> STM32 Pin PB11 (Internal Pull-Up enabled)
    - Sensor VCC & GND         -> STM32 3.3V / 5V & GND

 4. PC PROGRAMMING & SERIAL TELEMETRY (USART1 @ 9600 Baud):
    - USB-to-TTL RX            -> STM32 Pin PA9  (USART1_TX)
    - USB-to-TTL TX            -> STM32 Pin PA10 (USART1_RX)
    - USB-to-TTL GND           -> STM32 GND

 ARDUINO IDE TOOLS MENU SETTINGS:
   - Board         : "Generic STM32F103C series" (NOT C6/fake C8!)
   - Variant       : "STM32F103C8 (20k RAM. 64k Flash)"
   - Upload method : "Serial" (or "STM32duino bootloader")
   - CPU Speed     : "72MHz (Normal)"
================================================================================
*/

#include <Arduino.h>

// ---------------- GPIO PIN DEFINITIONS (STM32 BLUE PILL) ----------------
#define PIN_STEP        PB8   // A4988 STEP pulse
#define PIN_DIR         PB9   // A4988 DIR direction
#define PIN_ENABLE      PB10  // A4988 ENABLE (Active LOW)
#define PIN_STATUS_LED  PC13  // STM32 Blue Pill Onboard LED (Active LOW)

#define PIN_LDR_TL      PA0   // Left / Top-Left (ADC1_IN0)
#define PIN_LDR_TR      PA1   // Right / Top-Right (ADC1_IN1)
#define PIN_LDR_BL      PA4   // Bottom-Left (ADC1_IN4)
#define PIN_LDR_BR      PA5   // Bottom-Right (ADC1_IN5)

#define PIN_HALL_HOME   PB11  // Hall-effect zero datum (EXTI11)

// ---------------- KINEMATIC & CALIBRATION CONSTANTS ----------------
// Bench-confirmed physical transmission factor:
// 10.556 steps per degree (19:1 worm reduction)
// 10 deg = 106 steps | 40 deg = 422 steps | 80 deg full range = 844 steps
float stepsPerDegree = 10.556f;

// Pulse timing: 2500 microseconds = smooth starting torque, zero stalls
const int STEP_PULSE_DELAY_US = 2500;

// Mechanical travel envelope: strictly [-40.0 deg, +40.0 deg] (80.0 deg window)
const float MIN_ANGLE = -40.0f;
const float MAX_ANGLE =  40.0f;

// ---------------- CLOSED-LOOP TRACKING PARAMETERS ----------------
int deadbandThreshold   = 120;   // ADC counts tolerance (approx 3% deadband)
int nightDarkThreshold  = 200;   // Below this is darkness / night
float trackingStepAngle = 1.0f;  // Angular adjustment per tracking cycle (degrees)
unsigned long trackingIntervalMs = 800; // Time between tracking evaluation ticks

bool isAutoTracking = true;      // true = Auto Sun Tracking, false = Manual mode
bool invertDirection = false;    // Flip tracking direction if needed

// Tracking State
float currentAngle = 0.0f;
unsigned long lastTrackTime = 0;
unsigned long frameCount = 0;

// ---------------- ULTRA-LEAN DUAL-SERIAL OUTPUT HELPERS ----------------
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
void printTelemetry(int leftVal, int rightVal, int delta, float intensityPct, const char* stateStr);
void printHelp();

// =============================================================================
// SETUP
// =============================================================================
void setup() {
  // 1. Motor Driver Pins
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);

  // Immediately de-energize coils at startup (0W idle, zero buzz)
  motorOff();

  // 2. Status Indicator LED
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW); // LED ON at boot

  // 3. Sensor Pins
  pinMode(PIN_LDR_TL, INPUT);
  pinMode(PIN_LDR_TR, INPUT);
  pinMode(PIN_LDR_BL, INPUT);
  pinMode(PIN_LDR_BR, INPUT);
  pinMode(PIN_HALL_HOME, INPUT_PULLUP);

  // 4. Initialize Serial @ 9600 Baud
  Serial.begin(9600);
  #if HAS_SERIAL1
    Serial1.begin(9600);
  #endif

  delay(1000);

  printlnAll();
  printlnAll(F("============================================================================"));
  printlnAll(F(" POWER IQ — STM32 BLUE PILL CLOSED-LOOP LDR SUN TRACKER                     "));
  printlnAll(F(" Architecture : 32-bit ARM Cortex-M3 @ 72 MHz (100% Pure STM32 Firmware)   "));
  printlnAll(F(" Actuator     : NEMA 17 Stepper + A4988 Driver (PB8, PB9, PB10)            "));
  printlnAll(F(" Sensors      : 4-Quadrant LDR Array (PA0, PA1, PA4, PA5) + Hall (PB11)    "));
  printlnAll(F(" Range Limit  : -40.0 deg to +40.0 deg (80.0 deg total solar window)      "));
  printlnAll(F(" Calibration  : 10.556 steps/deg | Pulse: 2500 us | Idle Coils: 0W OFF     "));
  printlnAll(F("============================================================================"));
  printlnAll(F(" Tracking Mode: [AUTO] active. Type 'HELP' in Serial Monitor for commands.  "));
  printlnAll();

  // Check if home magnet is already detected
  if (digitalRead(PIN_HALL_HOME) == LOW) {
    currentAngle = 0.0f;
    printlnAll(F("[HOME] Magnet detected at startup! Datum synchronized to 0.0 deg.\n"));
  }

  digitalWrite(PIN_STATUS_LED, HIGH); // LED OFF (Ready)
}

// =============================================================================
// MAIN LOOP
// =============================================================================
void loop() {
  // 1. Process any incoming user commands from Serial Monitor
  processSerialInput();

  // 2. Periodic Closed-Loop Tracking Cycle
  unsigned long now = millis();
  if (now - lastTrackTime >= trackingIntervalMs) {
    lastTrackTime = now;
    executeSunTracking();
  }
}

// =============================================================================
// SUN TRACKING CORE ALGORITHM
// =============================================================================
void executeSunTracking() {
  frameCount++;

  // 1. Read 4 LDRs with 4x oversampling to filter optical/ADC noise
  long sumTL = 0, sumTR = 0, sumBL = 0, sumBR = 0;
  for (int i = 0; i < 4; i++) {
    sumTL += analogRead(PIN_LDR_TL);
    sumTR += analogRead(PIN_LDR_TR);
    sumBL += analogRead(PIN_LDR_BL);
    sumBR += analogRead(PIN_LDR_BR);
    delayMicroseconds(250);
  }
  int rawTL = sumTL / 4;
  int rawTR = sumTR / 4;
  int rawBL = sumBL / 4;
  int rawBR = sumBR / 4;

  // 2. Detect 2-LDR vs 4-LDR Mode automatically
  int leftLight  = 0;
  int rightLight = 0;

  if (rawBL < 40 && rawBR < 40) {
    // 2-LDR Configuration: PA0 = Left/East, PA1 = Right/West
    leftLight  = rawTL;
    rightLight = rawTR;
  } else {
    // 4-LDR Differential Configuration
    leftLight  = (rawTL + rawBL) / 2;
    rightLight = (rawTR + rawBR) / 2;
  }

  // 3. Compute Differential Metrics
  int deltaLight = leftLight - rightLight;
  if (invertDirection) deltaLight = -deltaLight;

  int avgLight = (leftLight + rightLight) / 2;
  float intensityPct = (avgLight / 4095.0f) * 100.0f;

  // 4. Decision Engine
  const char* trackingState = "BALANCED";

  if (avgLight < nightDarkThreshold) {
    trackingState = "NIGHT MODE (Sleeping, Coils OFF)";
  }
  else if (abs(deltaLight) <= deadbandThreshold) {
    trackingState = "BALANCED (Sun Locked, 0W Holding)";
  }
  else if (deltaLight > deadbandThreshold) {
    if (isAutoTracking) {
      float nextAngle = currentAngle + trackingStepAngle;
      if (nextAngle <= MAX_ANGLE) {
        trackingState = "SUN ON LEFT -> Adjusting (+)";
        moveToAngle(nextAngle);
      } else {
        trackingState = "LIMIT REACHED (+40.0 deg MAX)";
      }
    } else {
      trackingState = "SUN ON LEFT (Manual Paused)";
    }
  }
  else {
    if (isAutoTracking) {
      float nextAngle = currentAngle - trackingStepAngle;
      if (nextAngle >= MIN_ANGLE) {
        trackingState = "SUN ON RIGHT -> Adjusting (-)";
        moveToAngle(nextAngle);
      } else {
        trackingState = "LIMIT REACHED (-40.0 deg MIN)";
      }
    } else {
      trackingState = "SUN ON RIGHT (Manual Paused)";
    }
  }

  // 5. Print Telemetry Frame every 2 cycles (approx every 1.6 sec)
  if (frameCount % 2 == 0) {
    printTelemetry(leftLight, rightLight, deltaLight, intensityPct, trackingState);
  }
}

// =============================================================================
// TELEMETRY OUTPUT TO SERIAL MONITOR
// =============================================================================
void printTelemetry(int leftVal, int rightVal, int delta, float intensityPct, const char* stateStr) {
  printlnAll(F("----------------------------------------------------------------------------"));
  printAll(F(" [POWER IQ] Angle: "));
  if (currentAngle >= 0) printAll(F("+"));
  printAll(currentAngle, 1);
  printAll(F(" deg | Mode: "));
  printAll(isAutoTracking ? F("AUTO TRACKING") : F("MANUAL PAUSED"));
  printAll(F(" | Frame #")); printlnAll(frameCount);

  printAll(F(" LDR Left : ")); printAll(leftVal);
  printAll(F(" | LDR Right: ")); printAll(rightVal);
  printAll(F(" | Delta: "));
  if (delta >= 0) printAll(F("+"));
  printAll(delta);
  printAll(F(" (Deadband: +/-")); printAll(deadbandThreshold); printlnAll(F(")"));

  printAll(F(" Intensity: ")); printAll(intensityPct, 1);
  printAll(F("% | Hall Magnet: "));
  printAll(digitalRead(PIN_HALL_HOME) == LOW ? F("TRIGGERED (0.0 HOME)") : F("OPEN"));
  printlnAll();

  printAll(F(" Decision : ")); printlnAll(stateStr);
  printlnAll(F(" Motor    : 100% Silent & Cool (A4988 ENABLE = HIGH, 0W Idle)"));
  printlnAll(F("----------------------------------------------------------------------------"));
}

// =============================================================================
// MOTOR KINEMATIC CONTROL (BENCH CALIBRATED TO 10.556 STEPS/DEG)
// =============================================================================
void moveToAngle(float targetAngle) {
  // Clamp strictly within travel limits
  if (targetAngle < MIN_ANGLE) targetAngle = MIN_ANGLE;
  if (targetAngle > MAX_ANGLE) targetAngle = MAX_ANGLE;

  float deltaDeg = targetAngle - currentAngle;
  if (fabs(deltaDeg) < 0.05f) return;

  long steps = (long)(fabs(deltaDeg) * stepsPerDegree + 0.5f);
  if (steps == 0) {
    currentAngle = targetAngle;
    return;
  }

  // 1. Energize Motor Coils & turn on Status LED
  motorOn();
  digitalWrite(PIN_STATUS_LED, LOW); // Active LOW: LED ON

  // 2. Set Direction (HIGH = Positive, LOW = Negative)
  digitalWrite(PIN_DIR, (deltaDeg > 0) ? HIGH : LOW);

  // 3. Pulse Steps with bench-calibrated 2500us delay
  for (long i = 0; i < steps; i++) {
    stepPulse();
  }

  // 4. Update Position Tracker
  currentAngle = targetAngle;

  // 5. Automatic Coil De-energization (0W idle holding via worm gear)
  motorOff();
  digitalWrite(PIN_STATUS_LED, HIGH); // LED OFF
}

// =============================================================================
// STEP PULSE GENERATION
// =============================================================================
void stepPulse() {
  digitalWrite(PIN_STEP, HIGH);
  delayMicroseconds(STEP_PULSE_DELAY_US);
  digitalWrite(PIN_STEP, LOW);
  delayMicroseconds(STEP_PULSE_DELAY_US);
}

void motorOn() {
  digitalWrite(PIN_ENABLE, LOW); // LOW = A4988 Active
  delay(5);                      // Stabilization time
}

void motorOff() {
  digitalWrite(PIN_ENABLE, HIGH); // HIGH = A4988 Coils Disconnected (0W)
}

// =============================================================================
// SERIAL COMMAND PARSER (LEAN CHAR-BUFFER IMPLEMENTATION)
// =============================================================================
static char cmdBuf[32];
static byte cmdPos = 0;

void handleCommand(char* cmd) {
  while (*cmd == ' ') cmd++;
  if (*cmd == 0) return;

  if (strcasecmp(cmd, "AUTO") == 0) {
    isAutoTracking = true;
    printlnAll(F("\n[CMD] Auto Sun Tracking ENABLED. Tracker is actively pursuing the sun!\n"));
  }
  else if (strcasecmp(cmd, "MANUAL") == 0 || strcasecmp(cmd, "STOP") == 0) {
    isAutoTracking = false;
    printlnAll(F("\n[CMD] Auto Sun Tracking PAUSED. Manual angle control mode active.\n"));
  }
  else if (strcasecmp(cmd, "ZERO") == 0) {
    currentAngle = 0.0f;
    printlnAll(F("\n[CMD] Current position zeroed: currentAngle = 0.0 deg (Datum set).\n"));
  }
  else if (strcasecmp(cmd, "HOME") == 0) {
    printlnAll(F("\n[CMD] Seeking Hall-effect 0.0 deg datum..."));
    isAutoTracking = false;
    motorOn();
    digitalWrite(PIN_DIR, (currentAngle > 0) ? LOW : HIGH);
    long timeoutSteps = (long)(90.0f * stepsPerDegree);
    bool foundHome = false;

    for (long s = 0; s < timeoutSteps; s++) {
      if (digitalRead(PIN_HALL_HOME) == LOW) {
        foundHome = true;
        break;
      }
      stepPulse();
    }
    motorOff();

    if (foundHome) {
      currentAngle = 0.0f;
      printlnAll(F("[CMD] Hall-effect sensor tripped! Successfully aligned at 0.0 deg.\n"));
    } else {
      printlnAll(F("[WARN] Hall magnet not detected during travel. Resetting to 0.0 deg.\n"));
      currentAngle = 0.0f;
    }
  }
  else if (strcasecmp(cmd, "INVERT") == 0) {
    invertDirection = !invertDirection;
    printAll(F("\n[CMD] Tracking direction inverted: "));
    printlnAll(invertDirection ? F("REVERSED") : F("NORMAL"));
  }
  else if (strncasecmp(cmd, "DEADBAND ", 9) == 0) {
    int val = atoi(cmd + 9);
    if (val > 10 && val < 1000) {
      deadbandThreshold = val;
      printAll(F("\n[CMD] Deadband threshold updated to: "));
      printlnAll(deadbandThreshold);
    }
  }
  else if (strncasecmp(cmd, "CAL ", 4) == 0) {
    float val = atof(cmd + 4);
    if (val > 0.1f && val < 100.0f) {
      stepsPerDegree = val;
      printAll(F("\n[CMD] Steps per degree updated to: "));
      printlnAll(stepsPerDegree, 4);
    }
  }
  else if (strcasecmp(cmd, "HELP") == 0 || strcmp(cmd, "?") == 0) {
    printHelp();
  }
  else if (strcasecmp(cmd, "STATUS") == 0) {
    printlnAll(F("\n--- POWER IQ SYSTEM HEALTH STATUS ---"));
    printAll(F(" Current Angle     : ")); printAll(currentAngle, 1); printlnAll(F(" deg"));
    printAll(F(" Tracking Mode     : ")); printlnAll(isAutoTracking ? F("AUTO") : F("MANUAL"));
    printAll(F(" Steps Per Degree  : ")); printlnAll(stepsPerDegree, 4);
    printAll(F(" Deadband Threshold: ")); printlnAll(deadbandThreshold);
    printAll(F(" Pulse Delay       : ")); printAll(STEP_PULSE_DELAY_US); printlnAll(F(" us"));
    printAll(F(" Hall Magnet Sensor: ")); printlnAll(digitalRead(PIN_HALL_HOME) == LOW ? F("TRIGGERED") : F("OPEN"));
    printlnAll(F("-------------------------------------\n"));
  }
  else {
    // Check if it is an angle command
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
        printAll(F("\n[ERROR] Target "));
        printAll(target, 1);
        printlnAll(F(" deg is outside safe travel [-40.0 deg to +40.0 deg]!\n"));
        return;
      }
      isAutoTracking = false;
      printlnAll(F("\n[MANUAL] Executing commanded angle..."));
      moveToAngle(target);
      printlnAll(F("[MANUAL] Move complete. (Type 'AUTO' to resume sun tracking)\n"));
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
  printlnAll(F(" POWER IQ — STM32 LDR SUN TRACKER SERIAL COMMANDS       "));
  printlnAll(F("========================================================"));
  printlnAll(F(" AUTO          : Enable continuous automatic sun tracking"));
  printlnAll(F(" MANUAL        : Pause auto tracking (hold current angle)"));
  printlnAll(F(" GOTO <deg>    : Move slats to specific angle (-40 to +40)"));
  printlnAll(F(" ZERO          : Calibrate current position as 0.0 deg  "));
  printlnAll(F(" HOME          : Auto-find 0.0 deg using Hall-effect sensor"));
  printlnAll(F(" DEADBAND <n>  : Adjust optical deadband (default: 120)  "));
  printlnAll(F(" CAL <float>   : Adjust steps/deg factor (default: 10.556)"));
  printlnAll(F(" INVERT        : Flip motor tracking direction (+/-)    "));
  printlnAll(F(" STATUS        : Display system parameters & sensors     "));
  printlnAll(F(" HELP / ?      : Show this instruction guide            "));
  printlnAll(F("========================================================\n"));
}
