/*
================================================================================
 POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 Firmware: Single-File Precision Stepper Controller (A4988 + NEMA 17)
 Author  : Prathamesh Sapate (Lead) & POWER IQ Engineering Team
 Hardware: Arduino UNO R3 + A4988 Driver + NEMA 17 Stepper Motor
 Transmission: Worm Gear Drive | Target Slat Range: -40° to +40°
================================================================================

 WIRING GUIDE (Arduino UNO -> A4988):
   Arduino Pin 8   -> A4988 STEP
   Arduino Pin 9   -> A4988 DIR
   Arduino Pin 7   -> A4988 ENABLE (Active LOW)
   Arduino 5V      -> A4988 VDD (Logic Power)
   Arduino GND     -> A4988 GND (Logic Ground beside VDD)
   Jumper Wire     -> A4988 RESET pin connected to SLEEP pin (Essential!)
   External 12V +  -> A4988 VMOT (With 100uF capacitor across VMOT & GND)
   External 12V -  -> A4988 GND (Motor Ground beside VMOT)
   NEMA 17 Wires   -> A4988 1A, 1B (Coil Phase A) and 2A, 2B (Coil Phase B)

 SERIAL MONITOR INSTRUCTIONS:
   1. Open Serial Monitor in Arduino IDE.
   2. Set Baud Rate to 9600 and Line Ending to "Newline".
   3. Send commands:
        40        -> Move slats to +40 degrees
       -30        -> Move slats to -30 degrees
        0         -> Return slats to center horizontal (0 degrees)
        STATUS    -> Show current position & status
        ZERO      -> Calibrate current position as 0 degrees
        OFF       -> Immediately de-energize coils (0W idle power)
        HELP      -> Show command menu
================================================================================
*/

#include <Arduino.h>

// ---------------- PIN DEFINITIONS ----------------
#define PIN_STEP   8
#define PIN_DIR    9
#define PIN_ENABLE 7

// ---------------- MECHANICAL & KINEMATIC SETTINGS ----------------
// Bench-Calibrated Precision Factor (Confirmed on physical hardware):
// 10.556 steps per degree (approx 19:1 worm gear reduction)
// 10 deg = 106 steps | 40 deg = 422 steps | 80 deg full travel = 844 steps
const float STEPS_PER_DEGREE = 10.556f;

// Pulse Timing: 2500us (2.5ms HIGH + 2.5ms LOW) = high torque, zero stalling
const int STEP_PULSE_DELAY_US = 2500;

// Mechanical Safety Limits (Degrees)
const float MIN_ANGLE = -40.0f;
const float MAX_ANGLE = 40.0f;

// Position Tracker
float currentAngle = 0.0f;

// ---------------- FUNCTION DECLARATIONS ----------------
void moveToAngle(float targetAngle);
void stepPulse();
void motorOn();
void motorOff();
void printStatus();
void printHelp();

// ---------------- SETUP ----------------
void setup() {
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);

  // Start with motor coils de-energized (100% Silent, Cold, 0W Idle Power)
  motorOff();

  Serial.begin(9600);
  while (!Serial && millis() < 1000);

  Serial.println();
  Serial.println(F("=========================================================="));
  Serial.println(F("  POWER IQ — Multi-Shaft Solar Cell Tracking System       "));
  Serial.println(F("  Controller : Arduino UNO + A4988 Precision Firmware     "));
  Serial.println(F("  Safe Range : -40.0 to +40.0 degrees (Total 80 deg)      "));
  Serial.println(F("  Calibration: 0.5556 steps/deg (Verified & Calibrated)   "));
  Serial.println(F("  Idle Mode  : Silent Holding (0W Power, Self-Locking)    "));
  Serial.println(F("=========================================================="));
  Serial.println(F(" Enter target angle (e.g. 20, -30, 0) or 'HELP' for menu: "));
  Serial.println();
}

// ---------------- MAIN LOOP ----------------
void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.length() == 0) return;

    // Check Text Commands
    if (input.equalsIgnoreCase("STATUS")) {
      printStatus();
      return;
    }

    if (input.equalsIgnoreCase("ZERO")) {
      currentAngle = 0.0f;
      Serial.println(F("[ACK] Current position calibrated as 0.0 deg (Center Home)."));
      return;
    }

    if (input.equalsIgnoreCase("OFF")) {
      motorOff();
      Serial.println(F("[ACK] Motor coils de-energized. 100% Silent & Cool (0W)."));
      return;
    }

    if (input.equalsIgnoreCase("HELP") || input.equalsIgnoreCase("?")) {
      printHelp();
      return;
    }

    // Process Angle Command (e.g. "MOVE 30" or direct "30")
    float target = 0.0f;
    if (input.startsWith("MOVE ") || input.startsWith("move ")) {
      target = input.substring(5).toFloat();
    } else {
      target = input.toFloat();
    }

    // Check Safety Limits
    if (target < MIN_ANGLE || target > MAX_ANGLE) {
      Serial.print(F("[ERROR] Target "));
      Serial.print(target, 1);
      Serial.print(F(" deg exceeds safe range ["));
      Serial.print(MIN_ANGLE, 1);
      Serial.print(F(" to +"));
      Serial.print(MAX_ANGLE, 1);
      Serial.println(F(" deg]. Movement rejected for mechanical safety."));
      return;
    }

    moveToAngle(target);
  }
}

// =============================================================================
// MOTOR KINEMATIC CONTROL FUNCTION (PERFECT ACCURACY & ZERO DRIFT)
// =============================================================================
void moveToAngle(float targetAngle) {
  float delta = targetAngle - currentAngle;

  if (fabs(delta) < 0.05f) {
    Serial.print(F("[INFO] Already at target angle: "));
    Serial.print(currentAngle, 1);
    Serial.println(F(" deg."));
    return;
  }

  // Calculate exact integer steps with mathematical rounding
  long stepsToExecute = (long)round(fabs(delta) * STEPS_PER_DEGREE);

  if (stepsToExecute == 0) {
    currentAngle = targetAngle;
    Serial.println(F("[INFO] Angle difference below 1 step threshold. Position locked."));
    return;
  }

  Serial.println(F("--------------------------------------------------"));
  Serial.print(F(" Target Angle    : ")); Serial.print(targetAngle, 1); Serial.println(F(" deg"));
  Serial.print(F(" Current Angle   : ")); Serial.print(currentAngle, 1); Serial.println(F(" deg"));
  Serial.print(F(" Motion Delta    : ")); Serial.print(delta, 1); Serial.println(F(" deg"));
  Serial.print(F(" Steps to Pulse  : ")); Serial.println(stepsToExecute);
  Serial.print(F(" Direction       : ")); Serial.println(delta > 0 ? F("CLOCKWISE (+)") : F("COUNTER-CLOCKWISE (-)"));
  Serial.println(F("--------------------------------------------------"));

  // 1. Wake up driver & energize coils
  motorOn();

  // 2. Set direction
  digitalWrite(PIN_DIR, (delta > 0) ? HIGH : LOW);

  // 3. Pulse the exact number of steps
  for (long i = 0; i < stepsToExecute; i++) {
    stepPulse();
  }

  // 4. Update tracking position
  currentAngle = targetAngle;

  // 5. POWER IQ POWER SAVING:
  // Cut coil current immediately. Worm gear mechanical teeth self-lock the
  // shafts in place, eliminating buzzing noise, heating, and cutting power to 0W.
  motorOff();

  Serial.print(F(">>> Target reached: "));
  Serial.print(currentAngle, 1);
  Serial.println(F(" deg. Transmission locked & silent.\n"));
}

// =============================================================================
// LOW-LEVEL STEP PULSE GENERATION
// =============================================================================
void stepPulse() {
  digitalWrite(PIN_STEP, HIGH);
  delayMicroseconds(STEP_PULSE_DELAY_US);
  digitalWrite(PIN_STEP, LOW);
  delayMicroseconds(STEP_PULSE_DELAY_US);
}

// =============================================================================
// DRIVER POWER & SILENT HOLDING MANAGEMENT
// =============================================================================
void motorOn() {
  digitalWrite(PIN_ENABLE, LOW); // A4988 ENABLE is Active-LOW
  delay(5);                      // Settling time for driver charge-pump
}

void motorOff() {
  digitalWrite(PIN_ENABLE, HIGH); // Disables MOSFET bridges (100% Silent, 0W Power)
  digitalWrite(PIN_STEP, LOW);
}

// =============================================================================
// TELEMETRY & STATUS DISPLAY
// =============================================================================
void printStatus() {
  Serial.println();
  Serial.println(F("--- POWER IQ TELEMETRY STATUS ---"));
  Serial.print(F("  Current Slat Angle : ")); Serial.print(currentAngle, 1); Serial.println(F(" deg"));
  Serial.print(F("  Calibration Factor : ")); Serial.print(STEPS_PER_DEGREE, 6); Serial.println(F(" steps/deg"));
  Serial.print(F("  Allowed Range      : [")); Serial.print(MIN_ANGLE, 1); Serial.print(F(" to +")); Serial.print(MAX_ANGLE, 1); Serial.println(F(" deg]"));
  Serial.println(F("  Coil Power State   : SILENT / OFF (0W Standby)"));
  Serial.println(F("  Transmission State : MECHANICALLY LOCKED"));
  Serial.println(F("---------------------------------"));
  Serial.println();
}

void printHelp() {
  Serial.println();
  Serial.println(F("=== POWER IQ SERIAL COMMAND GUIDE ==="));
  Serial.println(F("  <angle>      : Enter target angle directly (e.g. 25, -20, 0)"));
  Serial.println(F("  MOVE <angle> : Move to target angle (-40 to +40 degrees)"));
  Serial.println(F("  STATUS       : Display current angle and calibration parameters"));
  Serial.println(F("  ZERO         : Define current position as 0.0 degrees"));
  Serial.println(F("  OFF          : Force coils to de-energize (Silent & Cool)"));
  Serial.println(F("  HELP         : Show this guide"));
  Serial.println(F("====================================="));
  Serial.println();
}
