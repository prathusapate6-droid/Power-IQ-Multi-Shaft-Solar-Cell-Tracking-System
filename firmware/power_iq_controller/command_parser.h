/**
 * ==============================================================================
 * POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 * File: command_parser.h
 * Purpose: Non-blocking ASCII/JSON Serial Command Interpreter
 * ==============================================================================
 */

#ifndef POWER_IQ_COMMAND_PARSER_H
#define POWER_IQ_COMMAND_PARSER_H

#include "config.h"
#include "kinematics.h"
#include "motor_driver.h"

class CommandParser {
public:
  CommandParser();

  // Initialize with references to subsystems
  void init(KinematicsEngine* kinematics, MotorDriver* driver);

  // Poll serial port non-blockingly and process complete lines
  void update();

  // Print startup welcome banner and help
  void printBanner();

private:
  KinematicsEngine* _kinematics;
  MotorDriver* _driver;
  char _rxBuffer[COMMAND_BUFFER_SIZE];
  uint8_t _rxIndex;

  void _dispatchCommand(char* cmd);
  void _cmdStatus(bool jsonFormat);
  void _cmdMove(const char* arg);
  void _cmdCalibrate(const char* arg);
  void _cmdSpeed(const char* arg);
  void _cmdHelp();
};

#endif // POWER_IQ_COMMAND_PARSER_H
