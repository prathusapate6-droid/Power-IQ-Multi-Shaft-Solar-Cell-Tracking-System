/**
 * ==============================================================================
 * POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 * File: power_iq_controller.ino
 * Purpose: Production Embedded Controller Main Loop & Subsystem Coordinator
 * Team: Prathamesh Sapate (Lead), Shreyash Pachade, Vansh Dobhale, Prachi Ronge
 * Institution: Smart India Hackathon 2026 / Final-Year Engineering Project
 * ==============================================================================
 */

#include <Arduino.h>
#include "config.h"
#include "motor_driver.h"
#include "kinematics.h"
#include "command_parser.h"

// Subsystem Instances
MotorDriver motorDriver;
KinematicsEngine kinematics;
CommandParser commandParser;

// Heartbeat LED timing
unsigned long lastHeartbeatMs = 0;
bool heartbeatState = false;

void setup() {
  // Initialize Serial Interface
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial && millis() < 1500) {
    // Wait briefly for USB enumeration on native USB boards
  }

  // Initialize Status LED
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);

  // Initialize Subsystems
  motorDriver.init();
  kinematics.init(&motorDriver);
  commandParser.init(&kinematics, &motorDriver);

  // Print Welcome Banner
  commandParser.printBanner();
}

void loop() {
  // 1. Service Incoming Serial Commands (Non-blocking)
  commandParser.update();

  // 2. Service Stepper Motor Motion Cadence (Non-blocking)
  kinematics.update();

  // 3. Heartbeat LED Indicator:
  // Fast blink (100ms) when stepping; gentle pulse (500ms) when stationary/holding
  unsigned long currentMs = millis();
  unsigned long heartbeatInterval = kinematics.isMoving() ? 100 : 500;

  if (currentMs - lastHeartbeatMs >= heartbeatInterval) {
    lastHeartbeatMs = currentMs;
    heartbeatState = !heartbeatState;
    digitalWrite(PIN_STATUS_LED, heartbeatState ? HIGH : LOW);
  }
}
