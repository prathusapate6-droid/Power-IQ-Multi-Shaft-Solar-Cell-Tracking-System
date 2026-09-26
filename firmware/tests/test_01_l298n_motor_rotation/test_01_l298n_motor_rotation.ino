/**
 * ==============================================================================
 * POWER IQ — Subsystem Test 01: L298N Motor Rotation & Coil Verification
 * File: test_01_l298n_motor_rotation.ino
 * Target: Arduino UNO + L298N + NEMA 17 Stepper Motor
 * Connections:
 *   Arduino Pin 8  -> L298N IN1
 *   Arduino Pin 9  -> L298N IN2
 *   Arduino Pin 10 -> L298N IN3
 *   Arduino Pin 11 -> L298N IN4
 *   Arduino GND    -> L298N GND (Common Ground Essential!)
 *   12V DC Supply  -> L298N 12V & GND
 * ==============================================================================
 */

#include <Arduino.h>

#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11

const int stepDelay = 5; // Milliseconds per phase (5ms = smooth torque for NEMA 17)

void motorOff() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void stepPhase(uint8_t phase) {
  switch (phase) {
    case 0:
      digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
      break;
    case 1:
      digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
      break;
    case 2:
      digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
      break;
    case 3:
      digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
      break;
  }
  delay(stepDelay);
}

void stepCW(long steps) {
  for (long i = 0; i < steps; i++) {
    stepPhase(i % 4);
  }
  motorOff();
}

void stepCCW(long steps) {
  for (long i = 0; i < steps; i++) {
    stepPhase(3 - (i % 4));
  }
  motorOff();
}

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  motorOff();

  Serial.begin(115200);
  while (!Serial && millis() < 1000);

  Serial.println();
  Serial.println(F("=================================================="));
  Serial.println(F(" POWER IQ — Test 01: L298N Motor Rotation Test    "));
  Serial.println(F(" Cycle: 200 steps CW -> Pause 2s -> 200 CCW -> Pause"));
  Serial.println(F(" Coils de-energize during pause (0W idle heat).   "));
  Serial.println(F("=================================================="));
}

void loop() {
  Serial.println(F(">>> Stepping CLOCKWISE (200 steps = 1 Motor Rev)..."));
  stepCW(200);
  Serial.println(F("    Target reached. Coils OFF. Pausing 2 seconds."));
  delay(2000);

  Serial.println(F(">>> Stepping COUNTER-CLOCKWISE (200 steps = 1 Motor Rev)..."));
  stepCCW(200);
  Serial.println(F("    Target reached. Coils OFF. Pausing 2 seconds."));
  delay(2000);
}
