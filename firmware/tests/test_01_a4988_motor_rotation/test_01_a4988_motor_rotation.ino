/**
 * ==============================================================================
 * POWER IQ — Phase 1 / Test 01: A4988 Stepper Driver + NEMA 17 Rotation Test
 * File: test_01_a4988_motor_rotation.ino
 * Target: Arduino UNO + A4988 Stepper Driver + NEMA 17 Motor
 * 
 * Hardware Connections:
 *   Arduino Pin 8  -> A4988 STEP
 *   Arduino Pin 9  -> A4988 DIR
 *   Arduino Pin 7  -> A4988 ENABLE (LOW = Driver Active, HIGH = Disabled)
 *   Arduino 5V     -> A4988 VDD (Logic Power)
 *   Arduino GND    -> A4988 GND (Logic Ground - beside VDD)
 * 
 * Jumper:
 *   A4988 RESET pin connected to A4988 SLEEP pin (Must be jumpered!)
 * 
 * Motor Power (External 12V DC):
 *   12V DC (+)     -> A4988 VMOT (With 100uF capacitor between VMOT & GND)
 *   12V DC (-)     -> A4988 GND (Motor Ground - beside VMOT)
 * 
 * Stepper Motor Coils:
 *   A4988 1A & 1B  -> Motor Coil Phase A (e.g., Black & Green wires)
 *   A4988 2A & 2B  -> Motor Coil Phase B (e.g., Red & Blue wires)
 * 
 * Test Cycle:
 *   1. Rotate Clockwise (200 steps = 1 full motor revolution)
 *   2. Pause 2 seconds (coils disabled for cooling)
 *   3. Rotate Counter-Clockwise (200 steps = 1 full motor revolution)
 *   4. Pause 2 seconds (coils disabled for cooling)
 * ==============================================================================
 */

#include <Arduino.h>

// Pin Definitions
#define PIN_STEP   8
#define PIN_DIR    9
#define PIN_ENABLE 7

// Motor Specs
const int stepsPerRevolution = 200; // 1.8 degree step angle = 200 steps/rev in full step mode

// Speed configuration:
// 1000 microseconds between pulses = 500 Hz = 2.5 revolutions per second (moderate safe speed)
const int stepPulseDelayUs = 1000;

void stepMotor(int steps, bool clockwise) {
  // Set Direction
  digitalWrite(PIN_DIR, clockwise ? HIGH : LOW);

  // Enable driver coils (Active LOW)
  digitalWrite(PIN_ENABLE, LOW);
  delayMicroseconds(50); // Small settling time

  // Generate Step Pulses
  for (int i = 0; i < steps; i++) {
    digitalWrite(PIN_STEP, HIGH);
    delayMicroseconds(stepPulseDelayUs);
    digitalWrite(PIN_STEP, LOW);
    delayMicroseconds(stepPulseDelayUs);
  }

  // De-energize coils to eliminate idle heating (Power IQ power saving)
  digitalWrite(PIN_ENABLE, HIGH);
}

void setup() {
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);

  // Start with motor disabled (Cool & safe)
  digitalWrite(PIN_ENABLE, HIGH);
  digitalWrite(PIN_STEP, LOW);
  digitalWrite(PIN_DIR, LOW);

  Serial.begin(115200);
  while (!Serial && millis() < 1000);

  Serial.println();
  Serial.println(F("=========================================================="));
  Serial.println(F(" POWER IQ — Phase 1: A4988 + NEMA 17 Motor Rotation Test  "));
  Serial.println(F(" Driver: A4988 (STEP=Pin 8, DIR=Pin 9, ENABLE=Pin 7)      "));
  Serial.println(F(" Cycle: 200 Steps CW -> Pause 2s -> 200 Steps CCW -> Pause"));
  Serial.println(F("=========================================================="));
}

void loop() {
  Serial.println(F(">>> Stepping CLOCKWISE (200 steps = 1 Motor Rev)..."));
  stepMotor(stepsPerRevolution, true);
  Serial.println(F("    CW Complete. Coils disabled. Pausing 2 seconds."));
  delay(2000);

  Serial.println(F(">>> Stepping COUNTER-CLOCKWISE (200 steps = 1 Motor Rev)..."));
  stepMotor(stepsPerRevolution, false);
  Serial.println(F("    CCW Complete. Coils disabled. Pausing 2 seconds."));
  delay(2000);
}
