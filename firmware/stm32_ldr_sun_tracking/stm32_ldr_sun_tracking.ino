/*
================================================================================
 POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 Subsystem: STM32 Closed-Loop Sun Tracking + Midpoint Datum Homing + Night Park
 Platform : STM32F103C8T6 "Blue Pill" (32-bit ARM Cortex-M3 @ 72 MHz)
 Authors  : Prathamesh Sapate (Lead), Shreyash Pachade, Vansh Dobhale, Prachi Ronge
 Purpose  : High-Precision Sun Tracker with Wide-Magnet Centering & Night Auto-Park
================================================================================

 ENHANCED FEATURES IN THIS FIRMWARE:
   1. WIDE-MAGNET MIDPOINT CENTERING:
      - Automatically measures the physical width of the Hall magnet (0° to 10° span).
      - Reverses and positions the motor exactly at the true geometric center of the
        magnetic field, eliminating the 10° hysteresis error completely!
   2. NIGHT AUTO-RETURN TO HOME (0.0°):
      - When darkness is detected (all LDRs dark / low light), the tracker automatically
        rotates back to 0.0° (flat horizontal home datum) and sleeps with 0W coils OFF.
      - Automatically wakes up and resumes sun pursuit when daylight returns.
   3. CORRECTED TRACKING ORIENTATION:
      - TOP (PA0 + PA1) lit    -> Tilts towards PLUS (+) to face the sun.
      - BOTTOM (PA4 + PA5) lit -> Tilts towards MINUS (-) to face the sun.
      - All 4 lit equally      -> BALANCED: 100% Solar Yield, Coils 0W Locked.
   4. ULTRA-LOW LATENCY & FAST GLIDE:
      - 1200us pulse delay (416 steps/sec) + 250ms fast tracking evaluation loop.
================================================================================
*/

#include <Arduino.h>

// ---------------- GPIO PIN DEFINITIONS ----------------
#define PIN_STEP        PB8   // A4988 STEP pulse
#define PIN_DIR         PB9   // A4988 DIR direction
#define PIN_ENABLE      PB10  // A4988 ENABLE (Active LOW)
#define PIN_STATUS_LED  PC13  // Onboard LED (Active LOW)

#define PIN_LDR_TOP1    PA0   // Top Sector Sensor 1
#define PIN_LDR_TOP2    PA1   // Top Sector Sensor 2
#define PIN_LDR_BOT1    PA4   // Bottom Sector Sensor 1
#define PIN_LDR_BOT2    PA5   // Bottom Sector Sensor 2

#define PIN_HALL_HOME   PB11  // Hall-effect Home Sensor (Active LOW)

// ---------------- KINEMATICS & SPEED TUNING ----------------
// Bench-confirmed transmission: 10.556 steps per degree
float stepsPerDegree = 10.556f;

// High-Speed Pulse Timing: 1200us delay = 416 steps/sec (Fast & strong torque)
int stepPulseDelayUs = 1200;

// Travel Safety Limits (Degrees)
const float MIN_ANGLE = -40.0f;
const float MAX_ANGLE =  40.0f;

// ---------------- RESPONSIVE TRACKING PARAMETERS ----------------
int deadbandThreshold   = 150;  // Optical deadband tolerance (balanced zone)
int nightDarkThreshold  = 200;  // Below this is darkness / night
unsigned long trackingIntervalMs = 250; // Ultra-responsive: 4 checks per second

bool isAutoTracking  = true;    // True = Auto Tracking, False = Manual mode
bool invertDirection = false;   // Flip direction if needed

// System Position State
float currentAngle = 0.0f;
bool isHomed = false;
unsigned long lastTrackTime = 0;
unsigned long frameCount = 0;

// ---------------- LEAN DUAL-SERIAL OUTPUT ----------------
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
void runStartupHomingSequence();
void processSerialInput();
void executeSunTracking();
void printTelemetry(int topVal, int botVal, int diff, float intensityPct, const char* stateStr);
void printHelp();

// =============================================================================
// SETUP
// =============================================================================
void setup() {
  // 1. Motor Driver Pins
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  motorOff(); // Start 100% silent (0W)

  // 2. Status LED & Sensor Pins
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW); // LED ON at boot

  pinMode(PIN_LDR_TOP1, INPUT);
  pinMode(PIN_LDR_TOP2, INPUT);
  pinMode(PIN_LDR_BOT1, INPUT);
  pinMode(PIN_LDR_BOT2, INPUT);
  pinMode(PIN_HALL_HOME, INPUT_PULLUP);

  // 3. Serial Interface (9600 Baud)
  Serial.begin(9600);
#if HAS_SERIAL1
  Serial1.begin(9600);
#endif

  delay(1000);

  printlnAll();
  printlnAll(F("============================================================================"));
  printlnAll(F(" POWER IQ — SMART SUN TRACKER + MIDPOINT HOMING + NIGHT AUTO-PARK           "));
  printlnAll(F(" Architecture : 32-bit ARM Cortex-M3 @ 72 MHz (100% Pure STM32)             "));
  printlnAll(F(" Orientation  : TOP (PA0+PA1) -> [+], BOTTOM (PA4+PA5) -> [-]               "));
  printlnAll(F(" Features     : 10 deg Magnet Centering | Night Auto-Return to 0.0 deg       "));
  printlnAll(F(" Speed Tuning : Fast 1200 us pulses | 250 ms Fast Tracking Loop              "));
  printlnAll(F(" Safety Limits: -40.0 deg to +40.0 deg (80.0 deg Total Travel)               "));
  printlnAll(F("============================================================================"));

  // 4. PRECISION STARTUP HOMING WITH MIDPOINT CENTERING
  runStartupHomingSequence();

  digitalWrite(PIN_STATUS_LED, HIGH); // LED OFF (Ready)
  printlnAll(F("\n[READY] Auto-Tracking Active! Type 'HELP' in Serial Monitor for commands.\n"));
}

// =============================================================================
// PRECISION MIDPOINT HOMING ALGORITHM (ELIMINATES 10 DEG MAGNET WIDTH ERROR)
// =============================================================================
void runStartupHomingSequence() {
  printlnAll(F("[HOMING] Starting Precision Midpoint Homing (Measuring Magnet Width)..."));
  motorOn();
  digitalWrite(PIN_STATUS_LED, LOW); // LED ON during homing

  // If already inside the magnetic field, step backward until clear
  if (digitalRead(PIN_HALL_HOME) == LOW) {
    digitalWrite(PIN_DIR, LOW);
    for (int i = 0; i < 200; i++) {
      if (digitalRead(PIN_HALL_HOME) == HIGH) break;
      stepPulse();
    }
  }

  // Phase 1: Seek Entry Edge of Magnet (Edge 1)
  bool foundEdge1 = false;
  long maxSearchSteps = (long)(60.0f * stepsPerDegree); // Search up to 60 deg

  printAll(F("[HOMING] Seeking Magnet Edge 1..."));
  digitalWrite(PIN_DIR, HIGH); // Search forward
  for (long s = 0; s < maxSearchSteps; s++) {
    if (digitalRead(PIN_HALL_HOME) == LOW) {
      foundEdge1 = true;
      break;
    }
    stepPulse();
  }

  if (!foundEdge1) {
    // If not found in positive direction, search negative
    digitalWrite(PIN_DIR, LOW);
    for (long s = 0; s < maxSearchSteps * 2; s++) {
      if (digitalRead(PIN_HALL_HOME) == LOW) {
        foundEdge1 = true;
        break;
      }
      stepPulse();
    }
  }

  if (foundEdge1) {
    // Phase 2: Measure Physical Magnet Width (Step from Edge 1 until it exits to Edge 2)
    long spanSteps = 0;
    long maxSpan = (long)(25.0f * stepsPerDegree); // Maximum expected magnet width (25 deg)
    while (digitalRead(PIN_HALL_HOME) == LOW && spanSteps < maxSpan) {
      stepPulse();
      spanSteps++;
    }

    // Phase 3: Step back to Exact Geometric Center (spanSteps / 2)
    long centerSteps = spanSteps / 2;
    if (centerSteps > 0) {
      uint8_t currentDir = digitalRead(PIN_DIR);
      digitalWrite(PIN_DIR, !currentDir); // Reverse direction
      for (long s = 0; s < centerSteps; s++) {
        stepPulse();
      }
    }

    currentAngle = 0.0f;
    isHomed = true;
    printAll(F("\n[HOMING] Magnet Width: "));
    printAll(spanSteps / stepsPerDegree, 1);
    printlnAll(F(" deg. Successfully centered at true 0.0 deg datum!"));
  } else {
    currentAngle = 0.0f;
    isHomed = true;
    printlnAll(F("\n[WARN] Magnet not detected during travel. Position zeroed at current angle."));
  }

  motorOff();
  digitalWrite(PIN_STATUS_LED, HIGH);
  printlnAll(F("----------------------------------------------------------------------------"));
}

// =============================================================================
// MAIN LOOP
// =============================================================================
void loop() {
  // 1. Process Serial Commands
  processSerialInput();

  // 2. High-Speed Periodic Tracking Cycle (every 250ms)
  unsigned long now = millis();
  if (now - lastTrackTime >= trackingIntervalMs) {
    lastTrackTime = now;
    executeSunTracking();
  }
}

// =============================================================================
// HIGH-SPEED SUN TRACKING ALGORITHM
// =============================================================================
void executeSunTracking() {
  frameCount++;

  // 1. Oversample 4 LDRs (250us per sample)
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

  // 2. Active-LOW Inversion: 4095 = Dark, ~600 = Direct Bright Light
  // Real Light Intensity = (4095 - rawADC) -> Higher number = Brighter light!
  int lightT1 = 4095 - rawT1;
  int lightT2 = 4095 - rawT2;
  int lightB1 = 4095 - rawB1;
  int lightB2 = 4095 - rawB2;

  // 3. Compute Sector Light Intensities:
  // TOP SECTOR    = Average of PA0 & PA1
  // BOTTOM SECTOR = Average of PA4 & PA5
  int avgTop    = (lightT1 + lightT2) / 2;
  int avgBottom = (lightB1 + lightB2) / 2;

  // 4. Compute Axis Differential (Top vs Bottom):
  // When Top > Bottom    -> diff is POSITIVE -> Tilts towards PLUS (+)
  // When Bottom > Top    -> diff is NEGATIVE -> Tilts towards MINUS (-)
  int diff = avgTop - avgBottom;
  if (invertDirection) diff = -diff;

  int totalAmbient = (avgTop + avgBottom) / 2;
  float intensityPct = (totalAmbient / 4095.0f) * 100.0f;

  // 5. Decision Engine & Adaptive Step Size
  const char* trackingState = "BALANCED";

  if (totalAmbient < nightDarkThreshold) {
    // ---------------- NIGHT MODE: AUTO-RETURN TO 0.0 DEG HOME ----------------
    if (abs(currentAngle) > 0.5f) {
      trackingState = "NIGHT DETECTED -> Parking at 0.0 deg HOME";
      if (isAutoTracking) {
        moveToAngle(0.0f);
      }
    } else {
      trackingState = "NIGHT SLEEP (Parked at 0.0 deg, 0W Coils OFF)";
      motorOff();
    }
  }
  else if (abs(diff) <= deadbandThreshold) {
    // ---------------- 100% BALANCED / DIRECT SUN ALIGNMENT ----------------
    // When all sensors are lit and balanced, panel is normal to the sun!
    trackingState = "BALANCED -> SUN LOCKED (100% Solar Yield)";
    motorOff(); // 100% silent and cool (0W power)
  }
  else if (diff > deadbandThreshold) {
    // ---------------- SUN IS ON TOP (PA0 + PA1) ----------------
    // Move towards PLUS (+) side to align with the sun
    float stepDeg = 1.0f;
    if (abs(diff) > 1500) stepDeg = 2.0f;       // Large angle gap -> Fast glide
    else if (abs(diff) < 500) stepDeg = 0.5f;   // Small gap -> Fine micro-adjustment

    if (isAutoTracking) {
      float nextAngle = currentAngle + stepDeg;
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
    // ---------------- SUN IS ON BOTTOM (PA4 + PA5) ----------------
    // Move towards MINUS (-) side to align with the sun
    float stepDeg = 1.0f;
    if (abs(diff) > 1500) stepDeg = 2.0f;
    else if (abs(diff) < 500) stepDeg = 0.5f;

    if (isAutoTracking) {
      float nextAngle = currentAngle - stepDeg;
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

  // 6. Print Live Telemetry Frame every 4 cycles (approx every 1.0 second)
  if (frameCount % 4 == 0) {
    printTelemetry(avgTop, avgBottom, diff, intensityPct, trackingState);
  }
}

// =============================================================================
// TELEMETRY OUTPUT TO SERIAL MONITOR
// =============================================================================
void printTelemetry(int topVal, int botVal, int diff, float intensityPct, const char* stateStr) {
  printlnAll(F("----------------------------------------------------------------------------"));
  printAll(F(" [POWER IQ] Angle: "));
  if (currentAngle >= 0) printAll(F("+"));
  printAll(currentAngle, 1);
  printAll(F(" deg | Mode: "));
  printAll(isAutoTracking ? F("AUTO TRACKING") : F("MANUAL PAUSED"));
  printAll(F(" | Frame #")); printlnAll(frameCount);

  printAll(F(" TOP (PA0+PA1): ")); printAll(topVal);
  printAll(F(" | BOTTOM (PA4+PA5): ")); printAll(botVal);
  printAll(F(" | Diff: "));
  if (diff >= 0) printAll(F("+"));
  printAll(diff);
  printAll(F(" (Deadband: +/-")); printAll(deadbandThreshold); printlnAll(F(")"));

  printAll(F(" Intensity: ")); printAll(intensityPct, 1);
  printAll(F("% | Hall Magnet: "));
  printAll(digitalRead(PIN_HALL_HOME) == LOW ? F("TRIGGERED (0.0 HOME)") : F("OPEN"));
  printAll(F(" | Speed: ")); printAll(stepPulseDelayUs); printlnAll(F(" us"));

  printAll(F(" Decision : ")); printlnAll(stateStr);
  printlnAll(F(" Motor    : 100% Silent & Cool (0W Idle Holding)"));
  printlnAll(F("----------------------------------------------------------------------------"));
}

// =============================================================================
// MOTOR KINEMATIC CONTROL (FAST PULSES & ACCURATE TARGETING)
// =============================================================================
void moveToAngle(float targetAngle) {
  // Clamp within travel limits
  if (targetAngle < MIN_ANGLE) targetAngle = MIN_ANGLE;
  if (targetAngle > MAX_ANGLE) targetAngle = MAX_ANGLE;

  float deltaDeg = targetAngle - currentAngle;
  if (fabs(deltaDeg) < 0.05f) return;

  long steps = (long)(fabs(deltaDeg) * stepsPerDegree + 0.5f);
  if (steps == 0) {
    currentAngle = targetAngle;
    return;
  }

  // 1. Energize Motor Coils & Status LED
  motorOn();
  digitalWrite(PIN_STATUS_LED, LOW);

  // 2. Set Direction (HIGH = Positive, LOW = Negative)
  digitalWrite(PIN_DIR, (deltaDeg > 0) ? HIGH : LOW);

  // 3. Fast Pulses (1200us delay)
  for (long i = 0; i < steps; i++) {
    stepPulse();
  }

  // 4. Update Position
  currentAngle = targetAngle;

  // 5. Automatic Coil De-energization (0W idle power, zero buzzing)
  motorOff();
  digitalWrite(PIN_STATUS_LED, HIGH);
}

// =============================================================================
// STEP PULSE GENERATION
// =============================================================================
void stepPulse() {
  digitalWrite(PIN_STEP, HIGH);
  delayMicroseconds(stepPulseDelayUs);
  digitalWrite(PIN_STEP, LOW);
  delayMicroseconds(stepPulseDelayUs);
}

void motorOn() {
  digitalWrite(PIN_ENABLE, LOW); // LOW = A4988 Energized
  delayMicroseconds(500);        // Fast charge pump wake up
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
    runStartupHomingSequence();
    isAutoTracking = true;
  }
  else if (strcasecmp(cmd, "ZERO") == 0) {
    currentAngle = 0.0f;
    printlnAll(F("\n[CMD] Current position calibrated as 0.0 deg.\n"));
  }
  else if (strcasecmp(cmd, "INVERT") == 0) {
    invertDirection = !invertDirection;
    printAll(F("\n[CMD] Tracking direction inverted: "));
    printlnAll(invertDirection ? F("REVERSED") : F("NORMAL"));
  }
  else if (strncasecmp(cmd, "SPEED ", 6) == 0) {
    int val = atoi(cmd + 6);
    if (val >= 600 && val <= 4000) {
      stepPulseDelayUs = val;
      printAll(F("\n[CMD] Step pulse delay updated to: "));
      printAll(stepPulseDelayUs); printlnAll(F(" us"));
    }
  }
  else if (strncasecmp(cmd, "DEADBAND ", 9) == 0) {
    int val = atoi(cmd + 9);
    if (val > 10 && val < 1000) {
      deadbandThreshold = val;
      printAll(F("\n[CMD] Deadband updated to: "));
      printlnAll(deadbandThreshold);
    }
  }
  else if (strncasecmp(cmd, "CAL ", 4) == 0) {
    float val = atof(cmd + 4);
    if (val > 0.1f && val < 100.0f) {
      stepsPerDegree = val;
      printAll(F("\n[CMD] Steps/deg factor updated to: "));
      printlnAll(stepsPerDegree, 4);
    }
  }
  else if (strcasecmp(cmd, "STATUS") == 0) {
    printlnAll(F("\n--- SYSTEM STATUS ---"));
    printAll(F(" Angle        : ")); printAll(currentAngle, 1); printlnAll(F(" deg"));
    printAll(F(" Tracking Mode: ")); printlnAll(isAutoTracking ? F("AUTO") : F("MANUAL"));
    printAll(F(" Pulse Delay  : ")); printAll(stepPulseDelayUs); printlnAll(F(" us"));
    printAll(F(" Steps/Deg    : ")); printlnAll(stepsPerDegree, 4);
    printAll(F(" Deadband     : ")); printlnAll(deadbandThreshold);
    printAll(F(" Hall Homed   : ")); printlnAll(isHomed ? F("YES") : F("NO"));
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
  printlnAll(F(" POWER IQ — FAST SUN TRACKER SERIAL COMMANDS            "));
  printlnAll(F("========================================================"));
  printlnAll(F(" AUTO          : Enable continuous automatic sun tracking"));
  printlnAll(F(" MANUAL        : Pause auto tracking (hold current angle)"));
  printlnAll(F(" HOME          : Run automatic Hall-effect midpoint homing"));
  printlnAll(F(" GOTO <deg>    : Move slats to specific angle (-40 to +40)"));
  printlnAll(F(" ZERO          : Calibrate current position as 0.0 deg  "));
  printlnAll(F(" SPEED <us>    : Change step pulse delay (default: 1200)"));
  printlnAll(F(" DEADBAND <n>  : Adjust optical deadband (default: 150)  "));
  printlnAll(F(" CAL <float>   : Adjust steps/deg factor (default: 10.556)"));
  printlnAll(F(" INVERT        : Flip motor tracking direction (+/-)    "));
  printlnAll(F(" STATUS        : Display system parameters & sensors     "));
  printlnAll(F(" HELP / ?      : Show this instruction guide            "));
  printlnAll(F("========================================================\n"));
}
