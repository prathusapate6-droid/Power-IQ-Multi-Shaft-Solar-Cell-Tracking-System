/*
====================================================================
POWER IQ — Multi-Shaft Solar Cell Tracking System
Firmware: Arduino UNO + L298N Dual H-Bridge + NEMA 17 Stepper Motor
Function: Precise Slat Angular Rotation & Calibration via Serial Commands
Range   : -90° to +90°
Team    : Prathamesh Sapate (Lead), Shreyash Pachade, Vansh Dobhale, Prachi Ronge
====================================================================
*/

#include <Arduino.h>

#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11

// ---------------- MECHANICAL & STEPPER SETTINGS ----------------

const int motorStepsPerRev = 200;      // NEMA 17 standard: 1.8 deg per full step (200 steps/rev)
const int wormRatio = 30;             // 30:1 Worm Gear Reduction Ratio

// Calibrated steps per degree of shaft rotation:
const float stepsPerDegree = 3.5;
// Theoretical calculation: (float)(motorStepsPerRev * wormRatio) / 360.0;

const int stepDelay = 5;              // Milliseconds per step phase transition

int currentAngle = 0;                 // Slat orientation tracker (-90 to +90 deg)

// Function prototypes
void moveToAngle(int target);
void stepCW();
void stepCCW();
void phase1();
void phase2();
void phase3();
void phase4();
void motorOff();

// ---------------------------------------------------------------

void setup() {

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  motorOff();

  Serial.begin(9600);

  Serial.println();
  Serial.println("=================================================");
  Serial.println(" POWER IQ — Multi-Shaft Solar Tracking System   ");
  Serial.println(" Hardware Controller: Arduino UNO + L298N Driver");
  Serial.println(" Range: -90 to +90 degrees                      ");
  Serial.println(" Commands:                                      ");
  Serial.println("   Enter integer angle: e.g. 45, -30, 0         ");
  Serial.println("   Type 'STATUS' to read current shaft angle    ");
  Serial.println("=================================================");
}

void loop() {

  if (Serial.available()) {

    String cmd = Serial.readStringUntil('\n');

    cmd.trim();

    if (cmd == "STATUS") {

      Serial.print("Current Shaft Angle : ");
      Serial.print(currentAngle);
      Serial.println(" deg");
      return;
    }

    int targetAngle = cmd.toInt();

    if (targetAngle > 90 || targetAngle < -90) {

      Serial.println("Error: Invalid Angle. Allowed range is -90 to +90 degrees.");
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

  long totalSteps = round(abs(diff) * stepsPerDegree);

  Serial.print("Moving Shafts To : ");
  Serial.print(target);
  Serial.println(" deg");

  if (diff >= 0) {

    for (long i = 0; i <= totalSteps; i++) {

      stepCW();
    }

  }

  else if (diff <= 0) {

    for (long i = 0; i <= totalSteps; i++) {

      stepCCW();
    }
  }

  currentAngle = target;

  motorOff(); // De-energize coils to save power and prevent thermal accumulation

  Serial.print("Target Reached. Current Angle : ");
  Serial.print(currentAngle);
  Serial.println(" deg. Transmission Locked.");
}

//============================================
// Stepper Step Sequences (Full Step 4-Phase)
//============================================

void stepCW() {

  phase1();
  phase2();
  phase3();
  phase4();
}

void stepCCW() {

  phase4();
  phase3();
  phase2();
  phase1();
}

//============================================
// Individual Coil Energization Phases
//============================================

void phase1() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  delay(stepDelay);
}

void phase2() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  delay(stepDelay);
}

void phase3() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  delay(stepDelay);
}

void phase4() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  delay(stepDelay);
}

//============================================
// Power Saving: Cut Coil Holding Current
// (Worm gear self-locking prevents back-drive)
//============================================

void motorOff() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
