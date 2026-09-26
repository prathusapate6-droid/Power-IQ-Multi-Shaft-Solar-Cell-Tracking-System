/**
 * ==============================================================================
 * POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 * File: kinematics.h
 * Purpose: Non-blocking Kinematic Engine for Angular Positioning & Calibration
 * ==============================================================================
 */

#ifndef POWER_IQ_KINEMATICS_H
#define POWER_IQ_KINEMATICS_H

#include "config.h"
#include "motor_driver.h"

class KinematicsEngine {
public:
  KinematicsEngine();

  // Initialize with motor driver reference
  void init(MotorDriver* driver);

  // Command slats to target angle (-90 to +90 deg). Returns false if out of bounds.
  bool setTargetAngle(float targetAngleDeg);

  // Non-blocking loop update. Generates steps at configured cadence.
  void update();

  // Instant emergency halt
  void emergencyStop();

  // Set current shaft position as calibrated 0.0 degrees
  void setZero();

  // Calibrate steps per degree dynamically
  void setStepsPerDegree(float stepsPerDeg);

  // Getters
  float getCurrentAngle() const { return _currentAngle; }
  float getTargetAngle() const { return _targetAngle; }
  float getStepsPerDegree() const { return _stepsPerDegree; }
  bool isMoving() const { return _isMoving; }
  long getRemainingSteps() const { return _stepsRemaining; }
  unsigned long getStepDelayMs() const { return _stepDelayMs; }

  // Set step delay (speed control)
  void setStepDelayMs(unsigned long delayMs);

private:
  MotorDriver* _driver;
  float _currentAngle;
  float _targetAngle;
  float _stepsPerDegree;
  unsigned long _stepDelayMs;
  unsigned long _lastStepTimeMs;
  long _stepsRemaining;
  bool _directionCW;
  bool _isMoving;
};

#endif // POWER_IQ_KINEMATICS_H
