/**
 * ==============================================================================
 * POWER IQ — Subsystem Test 02: Stepper Pulse & Step Accuracy Verification
 * File: test_02_step_accuracy.ino
 * Target: Arduino UNO + L298N + NEMA 17 Stepper Motor
 * Purpose: Allows user to command precise step counts (+200, +400, -200, etc.)
 *          via the Serial Monitor to verify that physical rotation matches commanded steps.
 * ==============================================================================
 */

#include <Arduino.h>

#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11

const int stepDelay = 5;
long totalAccumulatedSteps = 0;
uint8_t currentPhaseIndex = 0;

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

void stepMotor(long steps) {
  bool cw = (steps > 0);
  long count = abs(steps);

  for (long i = 0; i < count; i++) {
    if (cw) {
      currentPhaseIndex = (currentPhaseIndex + 1) & 0x03;
    } else {
      currentPhaseIndex = (currentPhaseIndex + 3) & 0x03;
    }
    applyPhase(currentPhaseIndex);
    delay(stepDelay);
  }

  totalAccumulatedSteps += steps;
  motorOff(); // Power saving & thermal protection
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
  Serial.println(F(" POWER IQ — Test 02: Step Counting & Accuracy     "));
  Serial.println(F(" Commands:                                        "));
  Serial.println(F("   Enter integer steps to move:                   "));
  Serial.println(F("     200  -> 1 full revolution CW                 "));
  Serial.println(F("    -200  -> 1 full revolution CCW                "));
  Serial.println(F("     400  -> 2 full revolutions CW                "));
  Serial.println(F("     ZERO -> Reset step accumulator to 0          "));
  Serial.println(F("=================================================="));
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.equalsIgnoreCase("ZERO")) {
      totalAccumulatedSteps = 0;
      Serial.println(F("[ACK] Step accumulator reset to 0."));
      return;
    }

    long steps = input.toInt();
    if (steps == 0 && input != "0") {
      Serial.println(F("[ERR] Please enter a valid step count (e.g. 200, -200)"));
      return;
    }

    Serial.print(F(">>> Executing "));
    Serial.print(steps);
    Serial.println(F(" steps..."));

    stepMotor(steps);

    Serial.print(F(">>> Done. Net Accumulated Steps from Home: "));
    Serial.print(totalAccumulatedSteps);
    Serial.print(F(" ("));
    Serial.print((float)totalAccumulatedSteps / 200.0f, 2);
    Serial.println(F(" motor revolutions)"));
  }
}
