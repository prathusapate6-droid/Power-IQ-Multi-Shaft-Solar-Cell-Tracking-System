/*
====================================================================
POWER IQ — Multi-Shaft Solar Cell Tracking System
Firmware: Arduino UNO + A4988 Stepper Driver + NEMA 17 Stepper Motor
Function: Precise Slat Angular Rotation via Serial Monitor Commands
Range   : -40° to +40° (Safe mechanical range for Multi-Shaft System)
Author  : Prathamesh Sapate (Lead) & POWER IQ Team
====================================================================

Wiring Connection (Arduino UNO -> A4988):
  Arduino Pin 8  -> A4988 STEP
  Arduino Pin 9  -> A4988 DIR
  Arduino Pin 7  -> A4988 ENABLE (LOW = Motor ON, HIGH = Motor OFF)
  Arduino 5V     -> A4988 VDD
  Arduino GND    -> A4988 GND (Logic GND)
  Jumper Wire    -> A4988 RESET pin connected to SLEEP pin
  12V DC Supply  -> A4988 VMOT (+) and GND (-) [With 100uF Capacitor]
  NEMA 17 Motor  -> A4988 1A, 1B (Coil A) and 2A, 2B (Coil B)
====================================================================
*/

#include <Arduino.h>

// ---------------- PIN DEFINITIONS ----------------

#define STEP_PIN   8
#define DIR_PIN    9
#define ENABLE_PIN 7

// ---------------- MECHANICAL & STEPPER SETTINGS ----------------

const int motorStepsPerRev = 200;      // NEMA 17: 1.8 deg per step (200 steps/rev in Full Step)
const int wormRatio = 30;             // 30:1 Worm Gear Reduction Ratio

// Confirmed Bench Calibration: 10.556 steps per degree
const float stepsPerDegree = 10.556;

// Speed: Pulse delay in microseconds (2500us = smooth torque, high reliability)
const int stepPulseDelayUs = 2500;

// Travel Safety Limits (Degrees)
const int minAngle = -40;
const int maxAngle = 40;

int currentAngle = 0;                 // Slat orientation tracker (-40 to +40 deg)

// Function prototypes
void moveToAngle(int target);
void stepPulse();
void motorOn();
void motorOff();

// ---------------- SETUP ----------------

void setup() {

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);

  // Start with motor coils de-energized (100% silent & cool standby)
  motorOff();


  Serial.begin(9600);

  Serial.println();
  Serial.println("=================================================");
  Serial.println(" POWER IQ — Multi-Shaft Solar Tracking System   ");
  Serial.println(" Hardware Controller: Arduino UNO + A4988 Driver");
  Serial.println(" Range: -40 to +40 degrees                      ");
  Serial.println(" Commands:                                      ");
  Serial.println("   Enter integer angle: e.g. 20, -30, 0         ");
  Serial.println("   Type 'STATUS' to read current shaft angle    ");
  Serial.println("=================================================");
}

// ---------------- MAIN LOOP ----------------

void loop() {

  if (Serial.available()) {

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.length() == 0) return;

    if (cmd.equalsIgnoreCase("STATUS")) {

      Serial.print("Current Shaft Angle : ");
      Serial.print(currentAngle);
      Serial.println(" deg");
      return;
    }

    if (cmd.equalsIgnoreCase("ZERO")) {

      currentAngle = 0;
      Serial.println("Current position set to 0 deg (Home).");
      return;
    }

    if (cmd.equalsIgnoreCase("OFF")) {

      motorOff();
      Serial.println("Motor coils de-energized. 100% Silent & Cool (0W).");
      return;
    }

    int targetAngle = cmd.toInt();

    if (targetAngle > maxAngle || targetAngle < minAngle) {

      Serial.print("Error: Invalid Angle. Allowed range is ");
      Serial.print(minAngle);
      Serial.print(" to +");
      Serial.print(maxAngle);
      Serial.println(" degrees.");
      return;
    }

    moveToAngle(targetAngle);
  }
}

//============================================
// Motor Kinematic Control Function
//============================================

void moveToAngle(int target) {

  int diff = target - currentAngle;

  if (diff == 0) {
    Serial.println("Already at target angle.");
    return;
  }

  long totalSteps = round(abs(diff) * stepsPerDegree);

  Serial.print("Moving Shafts To : ");
  Serial.print(target);
  Serial.print(" deg (Steps: ");
  Serial.print(totalSteps);
  Serial.println(")");

  // Enable driver coils
  motorOn();

  // Set direction: positive diff = Clockwise, negative diff = Counter-Clockwise
  if (diff > 0) {
    digitalWrite(DIR_PIN, HIGH); // CW
  } else {
    digitalWrite(DIR_PIN, LOW);  // CCW
  }

  // Generate required step pulses
  for (long i = 0; i < totalSteps; i++) {
    stepPulse();
  }

  currentAngle = target;

  // Power Saving: Cut coil current after movement
  // Worm gear self-locking prevents back-drive, cutting idle heat & power to 0W
  motorOff();

  Serial.print("Target Reached. Current Angle : ");
  Serial.print(currentAngle);
  Serial.println(" deg. Transmission Locked.");
}

//============================================
// A4988 Single Step Pulse Generation
//============================================

void stepPulse() {

  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(stepPulseDelayUs);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(stepPulseDelayUs);
}

//============================================
// Driver Enable / Power Saving Controls
//============================================

void motorOn() {

  digitalWrite(ENABLE_PIN, LOW); // A4988 ENABLE is Active-LOW
  delay(5);                      // Driver charge pump & wake-up settling time
}

void motorOff() {

  digitalWrite(ENABLE_PIN, HIGH); // Disables MOSFETs, 0W idle power
  digitalWrite(STEP_PIN, LOW);
}
