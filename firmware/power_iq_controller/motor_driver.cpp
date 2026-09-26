/**
 * ==============================================================================
 * POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 * File: motor_driver.cpp
 * Purpose: Hardware Abstraction Layer (HAL) Implementation
 * ==============================================================================
 */

#include "motor_driver.h"

MotorDriver::MotorDriver() : _currentPhase(0) {}

void MotorDriver::init() {
#if (ACTIVE_DRIVER == DRIVER_TYPE_L298N)
  pinMode(PIN_MOTOR_IN1, OUTPUT);
  pinMode(PIN_MOTOR_IN2, OUTPUT);
  pinMode(PIN_MOTOR_IN3, OUTPUT);
  pinMode(PIN_MOTOR_IN4, OUTPUT);
  powerOff();
#elif (ACTIVE_DRIVER == DRIVER_TYPE_STEP_DIR)
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  #ifdef PIN_ENABLE
    pinMode(PIN_ENABLE, OUTPUT);
    digitalWrite(PIN_ENABLE, LOW); // Active-LOW enable
  #endif
  digitalWrite(PIN_STEP, LOW);
  digitalWrite(PIN_DIR, LOW);
#endif
}

void MotorDriver::step(bool clockwise) {
#if (ACTIVE_DRIVER == DRIVER_TYPE_L298N)
  if (clockwise) {
    _currentPhase = (_currentPhase + 1) & 0x03; // Advance 0 -> 1 -> 2 -> 3 -> 0
  } else {
    _currentPhase = (_currentPhase + 3) & 0x03; // Reverse 0 -> 3 -> 2 -> 1 -> 0
  }
  _applyL298NPhase(_currentPhase);

#elif (ACTIVE_DRIVER == DRIVER_TYPE_STEP_DIR)
  digitalWrite(PIN_DIR, clockwise ? HIGH : LOW);
  digitalWrite(PIN_STEP, HIGH);
  delayMicroseconds(20);
  digitalWrite(PIN_STEP, LOW);
#endif
}

void MotorDriver::_applyL298NPhase(uint8_t phase) {
#if (ACTIVE_DRIVER == DRIVER_TYPE_L298N)
  switch (phase) {
    case 0: // Phase 1: A+ HIGH, A- LOW, B+ HIGH, B- LOW
      digitalWrite(PIN_MOTOR_IN1, HIGH);
      digitalWrite(PIN_MOTOR_IN2, LOW);
      digitalWrite(PIN_MOTOR_IN3, HIGH);
      digitalWrite(PIN_MOTOR_IN4, LOW);
      break;
    case 1: // Phase 2: A+ LOW, A- HIGH, B+ HIGH, B- LOW
      digitalWrite(PIN_MOTOR_IN1, LOW);
      digitalWrite(PIN_MOTOR_IN2, HIGH);
      digitalWrite(PIN_MOTOR_IN3, HIGH);
      digitalWrite(PIN_MOTOR_IN4, LOW);
      break;
    case 2: // Phase 3: A+ LOW, A- HIGH, B+ LOW, B- HIGH
      digitalWrite(PIN_MOTOR_IN1, LOW);
      digitalWrite(PIN_MOTOR_IN2, HIGH);
      digitalWrite(PIN_MOTOR_IN3, LOW);
      digitalWrite(PIN_MOTOR_IN4, HIGH);
      break;
    case 3: // Phase 4: A+ HIGH, A- LOW, B+ LOW, B- HIGH
      digitalWrite(PIN_MOTOR_IN1, HIGH);
      digitalWrite(PIN_MOTOR_IN2, LOW);
      digitalWrite(PIN_MOTOR_IN3, LOW);
      digitalWrite(PIN_MOTOR_IN4, HIGH);
      break;
  }
#endif
}

void MotorDriver::powerOff() {
#if (ACTIVE_DRIVER == DRIVER_TYPE_L298N)
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, LOW);
  digitalWrite(PIN_MOTOR_IN3, LOW);
  digitalWrite(PIN_MOTOR_IN4, LOW);
#elif (ACTIVE_DRIVER == DRIVER_TYPE_STEP_DIR)
  #ifdef PIN_ENABLE
    digitalWrite(PIN_ENABLE, HIGH); // Disable coils
  #endif
#endif
}

const char* MotorDriver::getDriverName() const {
#if (ACTIVE_DRIVER == DRIVER_TYPE_L298N)
  return "L298N (4-Phase Bipolar H-Bridge)";
#elif (ACTIVE_DRIVER == DRIVER_TYPE_STEP_DIR)
  return "Step/Dir (A4988 / TMC2209)";
#else
  return "Unknown Driver";
#endif
}
