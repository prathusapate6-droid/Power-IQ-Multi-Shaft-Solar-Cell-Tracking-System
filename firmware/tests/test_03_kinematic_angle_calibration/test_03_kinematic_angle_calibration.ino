/**
 * ==============================================================================
 * POWER IQ — Subsystem Test 03: Kinematic Angular Positioning & Calibration
 * File: test_03_kinematic_angle_calibration.ino
 * Target: Arduino UNO + L298N + NEMA 17 Stepper Motor + Worm Gear Shafts
 * Purpose: Moves the solar slat shafts to commanded angles (-90° to +90°) and
 *          enables dynamic calibration of steps/degree with a physical protractor.
 * ==============================================================================
 */

#include <Arduino.h>

#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11

const int stepDelay = 5;

// Default theoretical value for 30:1 reduction (200 * 30 / 360 = 16.6667)
float stepsPerDegree = 16.6667f;

float currentAngle = 0.0f;
uint8_t currentPhase = 0;

void motorOff() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void applyPhase(uint8_t phase) {
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
}

void stepSingle(bool cw) {
  if (cw) {
    currentPhase = (currentPhase + 1) & 0x03;
  } else {
    currentPhase = (currentPhase + 3) & 0x03;
  }
  applyPhase(currentPhase);
  delay(stepDelay);
}

void moveToAngle(float targetAngle) {
  if (targetAngle < -90.0f || targetAngle > 90.0f) {
    Serial.println(F("[ERR] Angle outside safe mechanical limit (-90 to +90 deg)."));
    return;
  }

  float diff = targetAngle - currentAngle;
  long steps = (long)round(fabs(diff) * stepsPerDegree);

  if (steps == 0) {
    Serial.println(F("[INFO] Already at target angle."));
    currentAngle = targetAngle;
    return;
  }

  bool cw = (diff > 0.0f);
  Serial.print(F(">>> Moving from "));
  Serial.print(currentAngle);
  Serial.print(F(" deg to "));
  Serial.print(targetAngle);
  Serial.print(F(" deg ("));
  Serial.print(steps);
  Serial.println(F(" steps)..."));

  for (long i = 0; i < steps; i++) {
    stepSingle(cw);
  }

  currentAngle = targetAngle;
  motorOff(); // Power down coils to eliminate heating

  Serial.print(F(">>> Movement complete. Current Slat Angle: "));
  Serial.print(currentAngle);
  Serial.println(F(" deg. Transmission locked."));
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
  Serial.println(F("=========================================================="));
  Serial.println(F(" POWER IQ — Test 03: Kinematic Angular Calibration Bench  "));
  Serial.print(F(" Current Steps per Degree: "));
  Serial.println(stepsPerDegree, 4);
  Serial.println(F(" Commands:                                                "));
  Serial.println(F("   <angle>           : Move to angle (e.g. 45, -30, 0)    "));
  Serial.println(F("   CAL <steps_deg>   : Update steps/degree (e.g. CAL 16.6)"));
  Serial.println(F("   ZERO              : Define current position as 0 deg   "));
  Serial.println(F("   STATUS            : Print current angle & calibration  "));
  Serial.println(F("=========================================================="));
}

void loop() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.equalsIgnoreCase("STATUS")) {
      Serial.print(F("Current Angle: ")); Serial.print(currentAngle); Serial.println(F(" deg"));
      Serial.print(F("Steps/Degree : ")); Serial.println(stepsPerDegree, 4);
      return;
    }

    if (line.equalsIgnoreCase("ZERO")) {
      currentAngle = 0.0f;
      Serial.println(F("[ACK] Current position calibrated as 0.0 deg."));
      return;
    }

    if (line.startsWith("CAL") || line.startsWith("cal")) {
      String valStr = line.substring(3);
      valStr.trim();
      float val = valStr.toFloat();
      if (val > 0.5f) {
        stepsPerDegree = val;
        Serial.print(F("[ACK] Steps per degree updated to: "));
        Serial.println(stepsPerDegree, 4);
      } else {
        Serial.println(F("[ERR] Invalid calibration value."));
      }
      return;
    }

    float target = line.toFloat();
    moveToAngle(target);
  }
}
