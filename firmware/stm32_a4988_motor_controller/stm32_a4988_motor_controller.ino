/*
================================================================================
 POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 Firmware: STM32F103C8T6 "Blue Pill" Precision Stepper Controller (A4988 + NEMA 17)
 Author  : Prathamesh Sapate (Lead) & POWER IQ Engineering Team
 Hardware: STM32F103C8T6 (72 MHz ARM Cortex-M3) + A4988 Driver + NEMA 17 Motor
 Target Slat Range: -40.0° to +40.0° (Total 80° range)
 Calibration: 10.556 steps/deg (Bench-Confirmed Precision Factor)
================================================================================

 WIRING DIAGRAM (STM32 Blue Pill -> A4988):
   STM32 Pin PB8   -> A4988 STEP
   STM32 Pin PB9   -> A4988 DIR
   STM32 Pin PB10  -> A4988 ENABLE (Active LOW)
   STM32 Pin 3.3V  -> A4988 VDD (Logic Power - 3.3V is safe and matched!)
   STM32 Pin GND   -> A4988 GND (Logic Ground beside VDD)
   Jumper Wire     -> A4988 RESET connected to SLEEP (Essential!)
   External 12V +  -> A4988 VMOT (With 100uF capacitor across VMOT & GND)
   External 12V -  -> A4988 GND (Motor Ground beside VMOT)
   Common Ground   -> External 12V (-) MUST be connected to STM32 GND
   NEMA 17 Wires   -> A4988 1A, 1B (Phase A) and 2A, 2B (Phase B)

 USB-TO-TTL CONVERTER (For Programming & Serial Monitor on PC):
   USB-TTL 3.3V    -> STM32 3.3V
   USB-TTL GND     -> STM32 GND
   USB-TTL RX      -> STM32 PA9  (USART1_TX)
   USB-TTL TX      -> STM32 PA10 (USART1_RX)

 FLASHING JUMPER SETTINGS (BOOT0 & BOOT1):
   1. To Flash/Upload Code:
      - Set BOOT0 = 1 (jumper towards 3.3V rail)
      - Set BOOT1 = 0
      - Press Blue Pill RESET button once
      - Click "Upload" in Arduino IDE
   2. To Run Code Permanently:
      - Set BOOT0 = 0 (jumper towards GND rail)
      - Press Blue Pill RESET button once
================================================================================
*/

#include <Arduino.h>

// ---------------- PIN DEFINITIONS (STM32 BLUE PILL) ----------------
#define PIN_STEP       PB8   // A4988 STEP pulse
#define PIN_DIR        PB9   // A4988 DIR direction
#define PIN_ENABLE     PB10  // A4988 ENABLE (Active LOW: LOW=ON, HIGH=OFF)
#define PIN_STATUS_LED PC13  // Onboard Blue LED (Active LOW on Blue Pill)

// ---------------- KINEMATIC & CALIBRATION SETTINGS ----------------
// Bench-Calibrated Precision Factor (Confirmed on physical hardware):
// 10.556 steps per degree (approx 19:1 worm reduction)
// 10 deg = 106 steps | 40 deg = 422 steps | 80 deg full travel = 844 steps
const float STEPS_PER_DEGREE = 10.556f;

// Pulse Timing: 2500us (2.5ms HIGH + 2.5ms LOW) = smooth starting torque
const int STEP_PULSE_DELAY_US = 2500;

// Travel Safety Limits (Degrees)
const float MIN_ANGLE = -40.0f;
const float MAX_ANGLE = 40.0f;

// Position Tracker
float currentAngle = 0.0f;

// ---------------- UNIVERSAL DUAL-SERIAL INTERFACE ----------------
// Supports both USB-Serial CDC and Hardware USART1 (PA9/PA10)
#if defined(Serial1)
  #define HAS_SERIAL1 1
#else
  #define HAS_SERIAL1 0
#endif

void printAll(const String& msg) {
  Serial.print(msg);
  #if HAS_SERIAL1
    Serial1.print(msg);
  #endif
}

void printlnAll(const String& msg = "") {
  Serial.println(msg);
  #if HAS_SERIAL1
    Serial1.println(msg);
  #endif
}

// ---------------- FUNCTION DECLARATIONS ----------------
void moveToAngle(float targetAngle);
void stepPulse();
void motorOn();
void motorOff();
void printStatus();
void printHelp();

// ---------------- SETUP ----------------
void setup() {
  // Initialize GPIO Pins
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);

  // Start with motor coils de-energized (100% Silent, Cold, 0W Idle Power)
  motorOff();
  digitalWrite(PIN_STATUS_LED, HIGH); // LED OFF (Active LOW)

  // Start Serial Interfaces (9600 baud for standard USB-TTL converters)
  Serial.begin(9600);
  #if HAS_SERIAL1
    Serial1.begin(9600);
  #endif

  // Brief delay for oscillator stabilization
  delay(500);

  printlnAll();
  printlnAll(F("=========================================================="));
  printlnAll(F("  POWER IQ — Multi-Shaft Solar Cell Tracking System       "));
  printlnAll(F("  Controller : STM32F103C8T6 Blue Pill (72 MHz ARM-M3)    "));
  printlnAll(F("  Driver     : A4988 (STEP=PB8, DIR=PB9, ENABLE=PB10)     "));
  printlnAll(F("  Safe Range : -40.0 to +40.0 degrees (Total 80 deg)      "));
  printlnAll(F("  Calibration: 10.556 steps/deg (Empirically Confirmed)   "));
  printlnAll(F("  Idle Mode  : Silent Holding (0W Power, Worm Self-Lock)  "));
  printlnAll(F("=========================================================="));
  printlnAll(F(" Enter target angle (e.g. 20, -30, 0) or 'HELP':          "));
  printlnAll();
}

// ---------------- MAIN LOOP ----------------
void loop() {
  // Check if serial data is available on either port
  Stream* activeSerial = nullptr;
  if (Serial.available() > 0) {
    activeSerial = &Serial;
  }
  #if HAS_SERIAL1
  else if (Serial1.available() > 0) {
    activeSerial = &Serial1;
  }
  #endif

  if (activeSerial != nullptr) {
    String input = activeSerial->readStringUntil('\n');
    input.trim();

    if (input.length() == 0) return;

    // Command Parser
    if (input.equalsIgnoreCase("STATUS")) {
      printStatus();
      return;
    }

    if (input.equalsIgnoreCase("ZERO")) {
      currentAngle = 0.0f;
      printlnAll(F("[ACK] Current position calibrated as 0.0 deg (Center Home)."));
      return;
    }

    if (input.equalsIgnoreCase("OFF")) {
      motorOff();
      printlnAll(F("[ACK] Motor coils de-energized. 100% Silent & Cool (0W)."));
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

    // Safety Range Enforcement
    if (target < MIN_ANGLE || target > MAX_ANGLE) {
      printAll(F("[ERROR] Target "));
      printAll(String(target, 1));
      printAll(F(" deg exceeds safe range ["));
      printAll(String(MIN_ANGLE, 1));
      printAll(F(" to +"));
      printAll(String(MAX_ANGLE, 1));
      printlnAll(F(" deg]. Movement rejected for mechanical safety."));
      return;
    }

    moveToAngle(target);
  }
}

// =============================================================================
// MOTOR KINEMATIC CONTROL FUNCTION (HIGH PRECISION & ZERO DRIFT)
// =============================================================================
void moveToAngle(float targetAngle) {
  float delta = targetAngle - currentAngle;

  if (fabs(delta) < 0.05f) {
    printAll(F("[INFO] Already at target angle: "));
    printAll(String(currentAngle, 1));
    printlnAll(F(" deg."));
    return;
  }

  // Calculate integer steps with mathematical rounding
  long stepsToExecute = (long)round(fabs(delta) * STEPS_PER_DEGREE);

  if (stepsToExecute == 0) {
    currentAngle = targetAngle;
    printlnAll(F("[INFO] Angle delta below 1 step threshold. Position locked."));
    return;
  }

  printlnAll(F("--------------------------------------------------"));
  printAll(F(" Target Angle    : ")); printAll(String(targetAngle, 1)); printlnAll(F(" deg"));
  printAll(F(" Current Angle   : ")); printAll(String(currentAngle, 1)); printlnAll(F(" deg"));
  printAll(F(" Motion Delta    : ")); printAll(String(delta, 1)); printlnAll(F(" deg"));
  printAll(F(" Steps to Pulse  : ")); printlnAll(String(stepsToExecute));
  printAll(F(" Direction       : ")); printlnAll(delta > 0 ? F("CLOCKWISE (+)") : F("COUNTER-CLOCKWISE (-)"));
  printlnAll(F("--------------------------------------------------"));

  // 1. Wake up driver & turn on Blue Status LED
  motorOn();
  digitalWrite(PIN_STATUS_LED, LOW); // Active LOW: LED ON

  // 2. Set direction
  digitalWrite(PIN_DIR, (delta > 0) ? HIGH : LOW);

  // 3. Pulse the exact step count
  for (long i = 0; i < stepsToExecute; i++) {
    stepPulse();
  }

  // 4. Update tracking position
  currentAngle = targetAngle;

  // 5. POWER IQ POWER SAVING:
  // Cut coil current immediately. Worm gear mechanical teeth self-lock the
  // shafts in place, eliminating buzzing noise, heating, and cutting power to 0W.
  motorOff();
  digitalWrite(PIN_STATUS_LED, HIGH); // LED OFF

  printAll(F(">>> Target reached: "));
  printAll(String(currentAngle, 1));
  printlnAll(F(" deg. Transmission locked & silent.\n"));
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
  delay(5);                      // Settling time for driver charge pump
}

void motorOff() {
  digitalWrite(PIN_ENABLE, HIGH); // Disables MOSFET bridges (100% Silent, 0W Power)
  digitalWrite(PIN_STEP, LOW);
}

// =============================================================================
// TELEMETRY & STATUS DISPLAY
// =============================================================================
void printStatus() {
  printlnAll();
  printlnAll(F("--- POWER IQ STM32 TELEMETRY STATUS ---"));
  printAll(F("  Current Slat Angle : ")); printAll(String(currentAngle, 1)); printlnAll(F(" deg"));
  printAll(F("  Calibration Factor : ")); printAll(String(STEPS_PER_DEGREE, 4)); printlnAll(F(" steps/deg"));
  printAll(F("  Allowed Range      : [")); printAll(String(MIN_ANGLE, 1)); printAll(F(" to +")); printAll(String(MAX_ANGLE, 1)); printlnAll(F(" deg]"));
  printlnAll(F("  MCU Platform       : STM32F103C8T6 (ARM Cortex-M3 72MHz)"));
  printlnAll(F("  Coil Power State   : SILENT / OFF (0W Standby)"));
  printlnAll(F("  Transmission State : MECHANICALLY LOCKED"));
  printlnAll(F("---------------------------------------"));
  printlnAll();
}

void printHelp() {
  printlnAll();
  printlnAll(F("=== POWER IQ STM32 SERIAL COMMAND GUIDE ==="));
  printlnAll(F("  <angle>      : Enter target angle directly (e.g. 20, -30, 0)"));
  printlnAll(F("  MOVE <angle> : Move to target angle (-40 to +40 degrees)"));
  printlnAll(F("  STATUS       : Display current angle and calibration parameters"));
  printlnAll(F("  ZERO         : Define current position as 0.0 degrees"));
  printlnAll(F("  OFF          : Force coils to de-energize (Silent & Cool)"));
  printlnAll(F("  HELP         : Show this guide"));
  printlnAll(F("==========================================="));
  printlnAll();
}
