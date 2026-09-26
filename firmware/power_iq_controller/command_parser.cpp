/**
 * ==============================================================================
 * POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 * File: command_parser.cpp
 * Purpose: Non-blocking ASCII/JSON Serial Command Interpreter Implementation
 * ==============================================================================
 */

#include "command_parser.h"
#include <string.h>
#include <stdlib.h>

CommandParser::CommandParser()
  : _kinematics(nullptr), _driver(nullptr), _rxIndex(0) {
  _rxBuffer[0] = '\0';
}

void CommandParser::init(KinematicsEngine* kinematics, MotorDriver* driver) {
  _kinematics = kinematics;
  _driver = driver;
  _rxIndex = 0;
}

void CommandParser::printBanner() {
  Serial.println();
  Serial.println(F("=========================================================="));
  Serial.println(F(" POWER IQ — Low-Power Multi-Shaft Solar Tracking Controller"));
  Serial.println(F(" Architecture: Non-Blocking HAL (L298N / Step-Dir)        "));
  Serial.print(F(" Active Driver: "));
  Serial.println(_driver ? _driver->getDriverName() : "None");
  Serial.print(F(" Worm Ratio: 30:1 | Calibrated Steps/Deg: "));
  Serial.println(_kinematics ? _kinematics->getStepsPerDegree() : 0.0f);
  Serial.println(F(" Travel Range: -90.0 to +90.0 degrees                    "));
  Serial.println(F(" Type 'HELP' for command summary or enter angle (e.g. 45) "));
  Serial.println(F("=========================================================="));
}

void CommandParser::update() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    if (c == '\r') {
      continue; // Skip CR
    }

    if (c == '\n') {
      _rxBuffer[_rxIndex] = '\0';
      if (_rxIndex > 0) {
        _dispatchCommand(_rxBuffer);
      }
      _rxIndex = 0;
    } else {
      if (_rxIndex < (COMMAND_BUFFER_SIZE - 1)) {
        _rxBuffer[_rxIndex++] = c;
      }
    }
  }
}

void CommandParser::_dispatchCommand(char* cmd) {
  // Strip leading whitespace
  while (*cmd == ' ' || *cmd == '\t') cmd++;

  // Convert command verb to uppercase for matching
  char verb[16];
  uint8_t i = 0;
  while (cmd[i] != '\0' && cmd[i] != ' ' && i < 15) {
    verb[i] = (char)toupper(cmd[i]);
    i++;
  }
  verb[i] = '\0';

  const char* arg = cmd + i;
  while (*arg == ' ' || *arg == '\t') arg++;

  if (strcmp(verb, "STATUS") == 0) {
    bool json = (strstr(arg, "JSON") != nullptr);
    _cmdStatus(json);
  }
  else if (strcmp(verb, "MOVE") == 0) {
    _cmdMove(arg);
  }
  else if (strcmp(verb, "STOP") == 0) {
    if (_kinematics) _kinematics->emergencyStop();
    Serial.println(F("[ACK] EMERGENCY STOP TRIGGERED. Motor halted and de-energized."));
  }
  else if (strcmp(verb, "ZERO") == 0) {
    if (_kinematics) _kinematics->setZero();
    Serial.println(F("[ACK] CURRENT POSITION SET TO 0.0 DEG (Home)."));
  }
  else if (strcmp(verb, "CALIBRATE") == 0 || strcmp(verb, "CAL") == 0) {
    _cmdCalibrate(arg);
  }
  else if (strcmp(verb, "SPEED") == 0) {
    _cmdSpeed(arg);
  }
  else if (strcmp(verb, "OFF") == 0) {
    if (_driver) _driver->powerOff();
    Serial.println(F("[ACK] Motor coils de-energized (0W idle power)."));
  }
  else if (strcmp(verb, "HELP") == 0 || strcmp(verb, "?") == 0) {
    _cmdHelp();
  }
  else {
    // If command is a direct number (e.g. "45", "-30", "0")
    char* endPtr;
    float directAngle = strtod(cmd, &endPtr);
    if (endPtr != cmd) {
      _cmdMove(cmd);
    } else {
      Serial.print(F("[ERR] Unknown command: "));
      Serial.println(cmd);
      Serial.println(F("Type 'HELP' for available commands."));
    }
  }
}

void CommandParser::_cmdStatus(bool jsonFormat) {
  if (!_kinematics) return;

  float cur = _kinematics->getCurrentAngle();
  float tgt = _kinematics->getTargetAngle();
  bool mov = _kinematics->isMoving();
  long rem = _kinematics->getRemainingSteps();
  float spd = _kinematics->getStepsPerDegree();

  if (jsonFormat) {
    Serial.print(F("{\"angle\":"));
    Serial.print(cur, 2);
    Serial.print(F(",\"target\":"));
    Serial.print(tgt, 2);
    Serial.print(F(",\"moving\":"));
    Serial.print(mov ? F("true") : F("false"));
    Serial.print(F(",\"remaining_steps\":"));
    Serial.print(rem);
    Serial.print(F(",\"steps_per_deg\":"));
    Serial.print(spd, 2);
    Serial.print(F(",\"driver\":\""));
    Serial.print(_driver ? _driver->getDriverName() : "Unknown");
    Serial.println(F("\"}"));
  } else {
    Serial.println(F("--- POWER IQ TELEMETRY STATUS ---"));
    Serial.print(F("  Current Slat Angle : ")); Serial.print(cur, 2); Serial.println(F(" deg"));
    Serial.print(F("  Target Slat Angle  : ")); Serial.print(tgt, 2); Serial.println(F(" deg"));
    Serial.print(F("  Motion State       : ")); Serial.println(mov ? F("MOVING") : F("HOLDING (Locked)"));
    Serial.print(F("  Remaining Steps    : ")); Serial.println(rem);
    Serial.print(F("  Steps per Degree   : ")); Serial.println(spd, 4);
    Serial.print(F("  Active Driver      : ")); Serial.println(_driver ? _driver->getDriverName() : "None");
    Serial.println(F("---------------------------------"));
  }
}

void CommandParser::_cmdMove(const char* arg) {
  if (!_kinematics) return;

  float target = atof(arg);
  if (target < MIN_TRAVEL_ANGLE_DEG || target > MAX_TRAVEL_ANGLE_DEG) {
    Serial.print(F("[ERR] Target "));
    Serial.print(target);
    Serial.print(F(" deg out of travel range ["));
    Serial.print(MIN_TRAVEL_ANGLE_DEG);
    Serial.print(F(" to "));
    Serial.print(MAX_TRAVEL_ANGLE_DEG);
    Serial.println(F("]."));
    return;
  }

  bool accepted = _kinematics->setTargetAngle(target);
  if (accepted) {
    Serial.print(F("[ACK] Moving to "));
    Serial.print(target);
    Serial.print(F(" deg (Delta: "));
    Serial.print(target - _kinematics->getCurrentAngle());
    Serial.print(F(" deg, Steps: "));
    Serial.print(_kinematics->getRemainingSteps());
    Serial.println(F(")"));
  }
}

void CommandParser::_cmdCalibrate(const char* arg) {
  if (!_kinematics) return;

  float newSpd = atof(arg);
  if (newSpd < 0.5f || newSpd > 200.0f) {
    Serial.println(F("[ERR] Steps per degree must be between 0.5 and 200.0"));
    return;
  }

  _kinematics->setStepsPerDegree(newSpd);
  Serial.print(F("[ACK] Steps per degree updated to: "));
  Serial.println(newSpd, 4);
}

void CommandParser::_cmdSpeed(const char* arg) {
  if (!_kinematics) return;

  unsigned long ms = strtoul(arg, nullptr, 10);
  if (ms < 1 || ms > 100) {
    Serial.println(F("[ERR] Step delay must be between 1 and 100 ms."));
    return;
  }

  _kinematics->setStepDelayMs(ms);
  Serial.print(F("[ACK] Step delay updated to: "));
  Serial.print(ms);
  Serial.println(F(" ms"));
}

void CommandParser::_cmdHelp() {
  Serial.println();
  Serial.println(F("=== POWER IQ SERIAL COMMAND GUIDE ==="));
  Serial.println(F("  <angle>          : Move directly to angle (e.g. '45', '-30', '0')"));
  Serial.println(F("  MOVE <angle>     : Move to target angle (-90 to +90 degrees)"));
  Serial.println(F("  STATUS           : Human-readable telemetry report"));
  Serial.println(F("  STATUS JSON      : JSON formatted telemetry for dashboard/bridge"));
  Serial.println(F("  STOP             : Immediate emergency stop"));
  Serial.println(F("  ZERO             : Define current physical orientation as 0.0 deg"));
  Serial.println(F("  CALIBRATE <val>  : Update steps/degree (e.g. 'CALIBRATE 16.6667')"));
  Serial.println(F("  SPEED <ms>       : Adjust step delay in ms (default 5ms)"));
  Serial.println(F("  OFF              : De-energize motor coils"));
  Serial.println(F("  HELP             : Show this command menu"));
  Serial.println(F("====================================="));
}
