/**
 * ==============================================================================
 * POWER IQ — A4988 Hardware Diagnostic & Continuous Motion Test
 * File: test_a4988_hardware_diag.ino
 * Purpose: Simple, slow continuous stepping to troubleshoot why motor is not rotating.
 * ==============================================================================
 */

#include <Arduino.h>

#define STEP_PIN   8
#define DIR_PIN    9
#define ENABLE_PIN 7

// 2000 microseconds = 2ms per pulse = High starting torque, no stalling
const int stepDelayUs = 2500;

void setup() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);

  // A4988 ENABLE pin is ACTIVE LOW. Setting LOW permanently enables driver.
  digitalWrite(ENABLE_PIN, LOW);
  digitalWrite(DIR_PIN, HIGH);
  digitalWrite(STEP_PIN, LOW);

  Serial.begin(9600);
  while (!Serial && millis() < 1000);

  Serial.println();
  Serial.println("=================================================");
  Serial.println(" POWER IQ — A4988 Continuous Diagnostic Test    ");
  Serial.println("=================================================");
  Serial.println("Motor should be slowly turning right now.");
  Serial.println("Check:");
  Serial.println("1. Are RESET and SLEEP pins connected with jumper?");
  Serial.println("2. Is external 12V DC turned on?");
  Serial.println("3. Is Arduino GND connected to A4988 GND?");
  Serial.println("=================================================");
}

long stepCount = 0;

void loop() {
  // Step pulse
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(stepDelayUs);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(stepDelayUs);

  stepCount++;

  if (stepCount % 200 == 0) {
    Serial.print("Steps sent: ");
    Serial.print(stepCount);
    Serial.println(" (1 Full Revolution Completed)");
  }
}
