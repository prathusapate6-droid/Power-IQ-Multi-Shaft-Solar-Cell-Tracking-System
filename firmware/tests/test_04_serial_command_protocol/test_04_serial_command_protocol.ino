/**
 * ==============================================================================
 * POWER IQ — Subsystem Test 04: Serial Protocol & Telemetry Benchmark
 * File: test_04_serial_command_protocol.ino
 * Target: Arduino UNO (Standalone or with L298N)
 * Purpose: Verifies ASCII & JSON command parsing, error trapping, and state reporting.
 * ==============================================================================
 */

#include <Arduino.h>

float simulatedAngle = 0.0f;
float simulatedTarget = 0.0f;
float stepsPerDegree = 16.6667f;
bool isMoving = false;
unsigned long moveStartTime = 0;
unsigned long moveDurationMs = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 1000);

  Serial.println();
  Serial.println(F("=========================================================="));
  Serial.println(F(" POWER IQ — Test 04: Serial Command Protocol Benchmark    "));
  Serial.println(F(" Commands to test:                                        "));
  Serial.println(F("   MOVE 45         - Move to +45 degrees                  "));
  Serial.println(F("   -30             - Direct angle input                   "));
  Serial.println(F("   STATUS          - Human readable telemetry             "));
  Serial.println(F("   STATUS JSON     - Machine JSON telemetry               "));
  Serial.println(F("   CALIBRATE 22.2  - Modify steps per degree              "));
  Serial.println(F("   STOP            - Emergency halt                       "));
  Serial.println(F("   ZERO            - Reset zero datum                     "));
  Serial.println(F("=========================================================="));
}

void loop() {
  // Simulate motion progression
  if (isMoving) {
    if (millis() - moveStartTime >= moveDurationMs) {
      simulatedAngle = simulatedTarget;
      isMoving = false;
      Serial.print(F("[EVENT] Target reached: "));
      Serial.print(simulatedAngle);
      Serial.println(F(" deg. Coils locked."));
    }
  }

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() == 0) return;

    if (cmd.equalsIgnoreCase("STATUS JSON")) {
      Serial.print(F("{\"angle\":")); Serial.print(simulatedAngle, 2);
      Serial.print(F(",\"target\":")); Serial.print(simulatedTarget, 2);
      Serial.print(F(",\"moving\":")); Serial.print(isMoving ? F("true") : F("false"));
      Serial.print(F(",\"steps_per_deg\":")); Serial.print(stepsPerDegree, 2);
      Serial.println(F("}"));
      return;
    }

    if (cmd.equalsIgnoreCase("STATUS")) {
      Serial.println(F("--- STATUS REPORT ---"));
      Serial.print(F("  Current Angle : ")); Serial.print(simulatedAngle); Serial.println(F(" deg"));
      Serial.print(F("  Target Angle  : ")); Serial.print(simulatedTarget); Serial.println(F(" deg"));
      Serial.print(F("  Motion Status : ")); Serial.println(isMoving ? F("MOVING") : F("IDLE"));
      Serial.print(F("  Steps / Deg   : ")); Serial.println(stepsPerDegree, 4);
      Serial.println(F("---------------------"));
      return;
    }

    if (cmd.equalsIgnoreCase("STOP")) {
      isMoving = false;
      simulatedTarget = simulatedAngle;
      Serial.println(F("[ACK] EMERGENCY STOP: Motion halted immediately."));
      return;
    }

    if (cmd.equalsIgnoreCase("ZERO")) {
      isMoving = false;
      simulatedAngle = 0.0f;
      simulatedTarget = 0.0f;
      Serial.println(F("[ACK] ZERO DATUM SET to 0.0 deg."));
      return;
    }

    if (cmd.startsWith("CALIBRATE ") || cmd.startsWith("calibrate ")) {
      float spd = cmd.substring(10).toFloat();
      if (spd > 0.5f) {
        stepsPerDegree = spd;
        Serial.print(F("[ACK] Steps per degree updated to: "));
        Serial.println(stepsPerDegree, 4);
      } else {
        Serial.println(F("[ERR] Invalid calibration parameter."));
      }
      return;
    }

    float target = 0.0f;
    if (cmd.startsWith("MOVE ") || cmd.startsWith("move ")) {
      target = cmd.substring(5).toFloat();
    } else {
      target = cmd.toFloat();
    }

    if (target < -90.0f || target > 90.0f) {
      Serial.println(F("[ERR] Target exceeds safe mechanical limit (-90 to +90 deg)."));
      return;
    }

    simulatedTarget = target;
    float delta = fabs(target - simulatedAngle);
    long steps = (long)round(delta * stepsPerDegree);
    moveDurationMs = steps * 5; // 5ms per step
    moveStartTime = millis();
    isMoving = true;

    Serial.print(F("[ACK] Moving to "));
    Serial.print(target);
    Serial.print(F(" deg (Delta: "));
    Serial.print(delta);
    Serial.print(F(" deg, "));
    Serial.print(steps);
    Serial.print(F(" steps, Est: "));
    Serial.print(moveDurationMs / 1000.0f, 1);
    Serial.println(F(" s)"));
  }
}
