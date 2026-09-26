/**
 * ==============================================================================
 * POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 * File: motor_driver.h
 * Purpose: Hardware Abstraction Layer (HAL) for Stepper Motor Drivers
 *          Supports L298N 4-phase sequence and A4988/TMC2209 Step/Dir
 * ==============================================================================
 */

#ifndef POWER_IQ_MOTOR_DRIVER_H
#define POWER_IQ_MOTOR_DRIVER_H

#include "config.h"

class MotorDriver {
public:
  MotorDriver();
  
  // Initialize GPIO pins according to active HAL driver
  void init();

  // Execute a single step in the requested direction
  void step(bool clockwise);

  // De-energize all coils to eliminate idle heating and power draw
  void powerOff();

  // Return the active driver name
  const char* getDriverName() const;

private:
  uint8_t _currentPhase;
  void _applyL298NPhase(uint8_t phase);
};

#endif // POWER_IQ_MOTOR_DRIVER_H
