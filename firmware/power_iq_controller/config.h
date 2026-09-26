/**
 * ==============================================================================
 * POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 * File: config.h
 * Purpose: Central Hardware Configuration, Pin Mapping & Kinematic Constants
 * Team: Prathamesh Sapate (Lead), Shreyash Pachade, Vansh Dobhale, Prachi Ronge
 * Target MCU: Arduino UNO R3 (ATmega328P, 16MHz) / ESP32 / STM32
 * ==============================================================================
 */

#ifndef POWER_IQ_CONFIG_H
#define POWER_IQ_CONFIG_H

#include <Arduino.h>

// ==============================================================================
// 1. DRIVER HARDWARE ABSTRACTION LAYER (HAL) SELECTION
// ==============================================================================
// Set ACTIVE_DRIVER to:
//   DRIVER_TYPE_L298N    : 4-wire Dual H-Bridge (Current physical workbench bench setup)
//   DRIVER_TYPE_STEP_DIR : 2-wire Step/Dir driver (A4988 / TMC2209 / DRV8825)
#define DRIVER_TYPE_L298N    1
#define DRIVER_TYPE_STEP_DIR 2

#define ACTIVE_DRIVER DRIVER_TYPE_L298N

// ==============================================================================
// 2. PIN ALLOCATION (ARDUINO UNO R3)
// ==============================================================================

#if (ACTIVE_DRIVER == DRIVER_TYPE_L298N)
  // L298N Dual H-Bridge 4-wire control pins
  #define PIN_MOTOR_IN1 8   // Coil Phase A+
  #define PIN_MOTOR_IN2 9   // Coil Phase A-
  #define PIN_MOTOR_IN3 10  // Coil Phase B+
  #define PIN_MOTOR_IN4 11  // Coil Phase B-
#elif (ACTIVE_DRIVER == DRIVER_TYPE_STEP_DIR)
  // Step / Direction driver pins (A4988 / TMC2209)
  #define PIN_STEP   8      // Step Pulse
  #define PIN_DIR    9      // Direction
  #define PIN_ENABLE 7      // Active-LOW Enable (optional)
#endif

// Onboard Heartbeat & Status Indicator
#define PIN_STATUS_LED 13

// Sensors & Electrical Telemetry Pins (Planned / Subsystem Benches)
#define PIN_LDR_EAST       A0   // East light sensor (Analog divider)
#define PIN_LDR_WEST       A1   // West light sensor (Analog divider)
#define PIN_LDR_REF        A2   // Ambient reference sensor
#define PIN_ACS712_CURRENT A3   // ACS712 motor current analog output
#define PIN_I2C_SDA        A4   // INA219 SDA (Solar bus telemetry)
#define PIN_I2C_SCL        A5   // INA219 SCL (Solar bus telemetry)

// Hardware Limit / Homing Interrupt Pins (Optional Safety Endstops)
#define PIN_LIMIT_HOME     2    // INT0
#define PIN_LIMIT_MAX      3    // INT1

// ==============================================================================
// 3. MECHANICAL & KINEMATIC SPECIFICATIONS
// ==============================================================================
// Stepper Motor Specs (NEMA 17 Standard)
#define MOTOR_FULL_STEPS_PER_REV  200     // 1.8 deg per step
#define MICROSTEPPING_MULTIPLIER  1       // 1 for L298N full-stepping, 16 for A4988

// Worm Gear Ratio (Central Worm Shaft to Shaft Worm Wheels)
// Change WORM_GEAR_TEETH to 30 or 40 depending on your 3D printed wheel:
#define WORM_GEAR_TEETH           30      // Default: 30:1 reduction ratio
// #define WORM_GEAR_TEETH        40      // Uncomment if using 40T wheel (A_wheel_01_x10.stl)

// Number of Parallel Rotating PV Shafts
#define NUM_SHAFTS                10      // 10 shafts x 10 cells = 100 PV cells

// Kinematic Calculation:
// Motor steps per 360 deg shaft rotation = MOTOR_FULL_STEPS_PER_REV * WORM_GEAR_TEETH * MICROSTEPPING_MULTIPLIER
// Steps per degree = (MOTOR_FULL_STEPS_PER_REV * WORM_GEAR_TEETH * MICROSTEPPING_MULTIPLIER) / 360.0f
// For 30:1 ratio & full-step: (200 * 30) / 360 = 16.6667 steps / degree
// For 40:1 ratio & full-step: (200 * 40) / 360 = 22.2222 steps / degree
#define THEORETICAL_STEPS_PER_DEG ((float)(MOTOR_FULL_STEPS_PER_REV * WORM_GEAR_TEETH * MICROSTEPPING_MULTIPLIER) / 360.0f)

// Default step delay in milliseconds (for L298N torque stability)
#define DEFAULT_STEP_DELAY_MS     5

// Mechanical Slat Travel Limits (Degrees)
#define MIN_TRAVEL_ANGLE_DEG      -90.0f
#define MAX_TRAVEL_ANGLE_DEG      90.0f

// Power Conservation: Automatically de-energize coils when stationary
// (Worm gear self-locking prevents back-drive, reducing idle power to 0W)
#define AUTO_DEENERGIZE_COILS     true

// ==============================================================================
// 4. COMMUNICATION & PROTOCOL
// ==============================================================================
#define SERIAL_BAUD_RATE          115200  // High-speed UART for real-time telemetry
#define COMMAND_BUFFER_SIZE       64

#endif // POWER_IQ_CONFIG_H
