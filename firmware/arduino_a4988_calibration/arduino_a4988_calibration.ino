/*
====================================================================
POWER IQ — Multi-Shaft Solar Cell Tracking System
Firmware: Arduino UNO + A4988 + NEMA 17 Live Calibration Bench
Function: Interactive Angle & Gear Ratio Calibration via Serial Monitor
Author  : Prathamesh Sapate (Lead) & POWER IQ Team
====================================================================
*/

#include <Arduino.h>

#define STEP_PIN   8
#define DIR_PIN    9
#define ENABLE_PIN 7

// 2500us = Proven smooth torque on bench
const int stepPulseDelayUs = 2500;

// Dynamic Calibration Factor (Steps per degree of SLAT rotation)
// You can change this live from Serial Monitor using: CAL <number>
// Example:
//   If Worm Gear has 30 teeth: (200 * 30) / 360 = 16.67 steps/deg
//   If Worm Gear has 40 teeth: (200 * 40) / 360 = 22.22 steps/deg
//   If directly testing motor shaft without gear: 200 / 360 = 0.556 steps/deg
// Confirmed bench calibration: 200 steps / 360 deg = 0.5556 steps/deg
float stepsPerDegree = 10.556;

float currentAngle = 0.0;

void stepPulse() {
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(stepPulseDelayUs);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(stepPulseDelayUs);
}

void moveSteps(long steps, bool cw) {
  digitalWrite(ENABLE_PIN, LOW); // Awake driver coils
  delay(5);
  digitalWrite(DIR_PIN, cw ? HIGH : LOW);

  for (long i = 0; i < steps; i++) {
    stepPulse();
  }

  // De-energize coils: Eliminates buzzing/hissing noise & heating (0W power)
  digitalWrite(ENABLE_PIN, HIGH);
}

void moveToAngle(float target) {
  float diff = target - currentAngle;
  if (fabs(diff) < 0.01) {
    Serial.println("Already at target angle.");
    return;
  }

  long totalSteps = (long)round(fabs(diff) * stepsPerDegree);
  float motorRevs = (float)totalSteps / 200.0;

  Serial.println("-------------------------------------------------");
  Serial.print("Target Slat Angle      : "); Serial.print(target, 1); Serial.println(" deg");
  Serial.print("Current Slat Angle     : "); Serial.print(currentAngle, 1); Serial.println(" deg");
  Serial.print("Angular Difference     : "); Serial.print(diff, 1); Serial.println(" deg");
  Serial.print("Steps to Execute       : "); Serial.println(totalSteps);
  Serial.print("Motor Shaft Rotations  : "); Serial.print(motorRevs, 2); Serial.println(" full revolutions");
  Serial.println("-------------------------------------------------");

  bool cw = (diff > 0);
  moveSteps(totalSteps, cw);

  currentAngle = target;
  Serial.print(">>> Target Reached. Current Angle: ");
  Serial.print(currentAngle, 1);
  Serial.println(" deg\n");
}

void setup() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);

  // Start with motor coils de-energized (100% silent & cool standby)
  digitalWrite(ENABLE_PIN, HIGH);
  digitalWrite(DIR_PIN, HIGH);
  digitalWrite(STEP_PIN, LOW);

  Serial.begin(9600);
  while (!Serial && millis() < 1000);

  Serial.println();
  Serial.println("=================================================");
  Serial.println(" POWER IQ — LIVE ANGLE & GEAR CALIBRATION BENCH  ");
  Serial.println("=================================================");
  Serial.print(" Active Steps Per Degree: ");
  Serial.println(stepsPerDegree, 3);
  Serial.println();
  Serial.println(" COMMANDS:");
  Serial.println("   <angle>          -> e.g. 10, 20, 40, -20, 0");
  Serial.println("   CAL <value>      -> e.g. CAL 0.556");
  Serial.println("   REV <turns>      -> e.g. REV 1");
  Serial.println("   ZERO             -> Reset current angle to 0 deg");
  Serial.println("   OFF              -> De-energize coils (Silent & Cool)");
  Serial.println("   STATUS           -> Show current angle & calibration");
  Serial.println("=================================================");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() == 0) return;

    if (input.equalsIgnoreCase("OFF")) {
      digitalWrite(ENABLE_PIN, HIGH);
      Serial.println("[ACK] Motor coils de-energized. 100% Silent & Cool (0W).");
      return;
    }

    if (input.equalsIgnoreCase("STATUS")) {
      Serial.print("Current Angle: "); Serial.print(currentAngle, 1); Serial.println(" deg");
      Serial.print("Steps/Degree : "); Serial.println(stepsPerDegree, 4);
      return;
    }

    if (input.equalsIgnoreCase("ZERO")) {
      currentAngle = 0.0;
      Serial.println("[ACK] Current position calibrated as 0.0 deg (Home).");
      return;
    }

    // REV command: Turn motor exactly N revolutions to check steps
    if (input.startsWith("REV ") || input.startsWith("rev ")) {
      float revs = input.substring(4).toFloat();
      long steps = (long)round(fabs(revs) * 200.0);
      bool cw = (revs >= 0);
      Serial.print(">>> Rotating Motor exactly ");
      Serial.print(revs);
      Serial.print(" revs (");
      Serial.print(steps);
      Serial.println(" steps)...");
      moveSteps(steps, cw);
      Serial.println(">>> Done.");
      return;
    }

    // CAL command: update calibration factor live
    if (input.startsWith("CAL ") || input.startsWith("cal ")) {
      float newCal = input.substring(4).toFloat();
      if (newCal > 0.01) {
        stepsPerDegree = newCal;
        Serial.print("[ACK] Steps per degree updated to: ");
        Serial.println(stepsPerDegree, 4);
      } else {
        Serial.println("[ERR] Invalid calibration number.");
      }
      return;
    }

    // Angle command
    float target = input.toFloat();
    moveToAngle(target);
  }
}
