/**
 * ==============================================================================
 * POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 * File: kinematics.cpp
 * Purpose: Non-blocking Kinematic Engine Implementation
 * ==============================================================================
 */

#include "kinematics.h"

KinematicsEngine::KinematicsEngine()
  : _driver(nullptr),
    _currentAngle(0.0f),
    _targetAngle(0.0f),
    _stepsPerDegree(THEORETICAL_STEPS_PER_DEG),
    _stepDelayMs(DEFAULT_STEP_DELAY_MS),
    _lastStepTimeMs(0),
    _stepsRemaining(0),
    _directionCW(true),
    _isMoving(false) {}

void KinematicsEngine::init(MotorDriver* driver) {
  _driver = driver;
  _currentAngle = 0.0f;
  _targetAngle = 0.0f;
  _stepsRemaining = 0;
  _isMoving = false;
  _lastStepTimeMs = millis();
}

bool KinematicsEngine::setTargetAngle(float targetAngleDeg) {
  // Validate safety limits (-90 to +90 degrees)
  if (targetAngleDeg < MIN_TRAVEL_ANGLE_DEG || targetAngleDeg > MAX_TRAVEL_ANGLE_DEG) {
    return false;
  }

  float diff = targetAngleDeg - _currentAngle;

  // Calculate required steps with proper rounding
  long stepsRequired = (long)round(fabs(diff) * _stepsPerDegree);

  // Bug fix: If delta is effectively zero, do not generate false steps
  if (stepsRequired == 0) {
    _targetAngle = targetAngleDeg;
    _currentAngle = targetAngleDeg;
    _stepsRemaining = 0;
    _isMoving = false;
    if (AUTO_DEENERGIZE_COILS && _driver) {
      _driver->powerOff();
    }
    return true;
  }

  _targetAngle = targetAngleDeg;
  _directionCW = (diff > 0.0f);
  _stepsRemaining = stepsRequired;
  _isMoving = true;
  _lastStepTimeMs = millis();

  return true;
}

void KinematicsEngine::update() {
  if (!_isMoving || _stepsRemaining <= 0 || _driver == nullptr) {
    return;
  }

  unsigned long currentMs = millis();
  if (currentMs - _lastStepTimeMs >= _stepDelayMs) {
    _lastStepTimeMs = currentMs;

    // Execute single step
    _driver->step(_directionCW);
    _stepsRemaining--;

    // Update real-time angular position estimation
    float stepIncrement = (1.0f / _stepsPerDegree);
    if (_directionCW) {
      _currentAngle += stepIncrement;
      if (_currentAngle > _targetAngle) _currentAngle = _targetAngle;
    } else {
      _currentAngle -= stepIncrement;
      if (_currentAngle < _targetAngle) _currentAngle = _targetAngle;
    }

    // Motion completion check
    if (_stepsRemaining <= 0) {
      _isMoving = false;
      _currentAngle = _targetAngle; // Lock exactly onto target
      _stepsRemaining = 0;

      if (AUTO_DEENERGIZE_COILS) {
        _driver->powerOff();
      }
    }
  }
}

void KinematicsEngine::emergencyStop() {
  _stepsRemaining = 0;
  _isMoving = false;
  _targetAngle = _currentAngle;
  if (_driver) {
    _driver->powerOff();
  }
}

void KinematicsEngine::setZero() {
  emergencyStop();
  _currentAngle = 0.0f;
  _targetAngle = 0.0f;
}

void KinematicsEngine::setStepsPerDegree(float stepsPerDeg) {
  if (stepsPerDeg > 0.1f) {
    _stepsPerDegree = stepsPerDeg;
  }
}

void KinematicsEngine::setStepDelayMs(unsigned long delayMs) {
  if (delayMs >= 1) {
    _stepDelayMs = delayMs;
  }
}
