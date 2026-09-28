/*
================================================================================
 POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 Subsystem: 4-Quadrant GL5528 Sun Tracking Sensor Test Bench
 Author   : Prathamesh Sapate (Lead) & POWER IQ Engineering Team
 Compatibility: Works on both Arduino UNO and STM32F103C8T6 Blue Pill!
================================================================================

 SENSOR CROSS-DIVIDER ARRANGEMENT (Looking at Front of Sensor):
           TOP-LEFT (TL)     |     TOP-RIGHT (TR)
         --------------------+---------------------   <-- Shadow Cross Divider
        BOTTOM-LEFT (BL)     |    BOTTOM-RIGHT (BR)

 WIRING CONNECTIONS:
   If using STM32 Blue Pill:
     - LDR Top-Left (TL)     -> STM32 Pin PA0 (ADC1_IN0)
     - LDR Top-Right (TR)    -> STM32 Pin PA1 (ADC1_IN1)
     - LDR Bottom-Left (BL)  -> STM32 Pin PA4 (ADC1_IN4)
     - LDR Bottom-Right (BR) -> STM32 Pin PA5 (ADC1_IN5)
     - LDR VCC               -> STM32 3.3V
     - LDR GND               -> STM32 GND
   
   If using Arduino UNO:
     - LDR Top-Left (TL)     -> Arduino Pin A0
     - LDR Top-Right (TR)    -> Arduino Pin A1
     - LDR Bottom-Left (BL)  -> Arduino Pin A2
     - LDR Bottom-Right (BR) -> Arduino Pin A3
     - LDR VCC               -> Arduino 5V
     - LDR GND               -> Arduino GND

 CIRCUIT DIAGRAM (Voltage Divider per LDR):
   VCC (3.3V / 5V) ---> [ GL5528 LDR ] ---> Analog Pin (A0 / PA0) ---> [ 10kΩ Resistor ] ---> GND
   (When light increases, LDR resistance drops, and Analog Voltage RISES!)
================================================================================
*/

#include <Arduino.h>

// ---------------- PIN DEFINITIONS (AUTO-DETECT PLATFORM) ----------------
#if defined(ARDUINO_ARCH_STM32) || defined(__STM32F1__)
  // STM32 Blue Pill 12-bit ADC Pins
  #define PIN_LDR_TL     PA0   // Top-Left
  #define PIN_LDR_TR     PA1   // Top-Right
  #define PIN_LDR_BL     PA4   // Bottom-Left
  #define PIN_LDR_BR     PA5   // Bottom-Right
  #define ADC_RESOLUTION 4095.0f
  #define VREF_VOLTAGE   3.3f
  #define BOARD_NAME     "STM32F103 Blue Pill (12-bit ADC)"
#else
  // Arduino UNO 10-bit ADC Pins
  #define PIN_LDR_TL     A0    // Top-Left
  #define PIN_LDR_TR     A1    // Top-Right
  #define PIN_LDR_BL     A2    // Bottom-Left
  #define PIN_LDR_BR     A3    // Bottom-Right
  #define ADC_RESOLUTION 1023.0f
  #define VREF_VOLTAGE   5.0f
  #define BOARD_NAME     "Arduino UNO (10-bit ADC)"
#endif

// ---------------- TRACKING THRESHOLDS ----------------
// Deadband prevents motor from jittering/hunting for tiny light differences
const int DEADBAND_ADC = (int)(ADC_RESOLUTION * 0.03f); // 3% deadband margin

// Minimum light threshold: below this, it's night or deep indoor darkness
const int NIGHT_THRESHOLD_ADC = (int)(ADC_RESOLUTION * 0.10f); // 10% threshold

// Universal Serial definition
#if defined(Serial1) && (defined(ARDUINO_ARCH_STM32) || defined(__STM32F1__))
  #define HAS_SERIAL1 1
#else
  #define HAS_SERIAL1 0
#endif

void printOut(const String& str) {
  Serial.print(str);
  #if HAS_SERIAL1
    Serial1.print(str);
  #endif
}

void printlnOut(const String& str = "") {
  Serial.println(str);
  #if HAS_SERIAL1
    Serial1.println(str);
  #endif
}

void setup() {
  pinMode(PIN_LDR_TL, INPUT);
  pinMode(PIN_LDR_TR, INPUT);
  pinMode(PIN_LDR_BL, INPUT);
  pinMode(PIN_LDR_BR, INPUT);

  Serial.begin(9600);
  #if HAS_SERIAL1
    Serial1.begin(9600);
  #endif

  delay(1000);

  printlnOut();
  printlnOut(F("=========================================================="));
  printlnOut(F(" POWER IQ — 4-Quadrant GL5528 Sun Sensor Test Bench       "));
  printlnOut(String(F(" Platform: ")) + BOARD_NAME);
  printlnOut(F(" Layout  : TL (Top-Left)    | TR (Top-Right)              "));
  printlnOut(F("           BL (Bottom-Left) | BR (Bottom-Right)           "));
  printlnOut(F("=========================================================="));
  printlnOut(F(" Tip: Shine your phone torch or cover individual LDRs     "));
  printlnOut(F("      to verify that each quadrant responds properly!     "));
  printlnOut();
}

void loop() {
  // 1. Read Raw ADC Values (Oversampled 4 times for noise filtering)
  long rawTL = 0, rawTR = 0, rawBL = 0, rawBR = 0;
  for (int i = 0; i < 4; i++) {
    rawTL += analogRead(PIN_LDR_TL);
    rawTR += analogRead(PIN_LDR_TR);
    rawBL += analogRead(PIN_LDR_BL);
    rawBR += analogRead(PIN_LDR_BR);
    delayMicroseconds(250);
  }
  int valTL = rawTL / 4;
  int valTR = rawTR / 4;
  int valBL = rawBL / 4;
  int valBR = rawBR / 4;

  // 2. Invert ADC Readings (Active-LOW: 4095 = Pitch Dark, ~600 = Direct Sunlight)
  int lightTL = ADC_RESOLUTION - valTL;
  int lightTR = ADC_RESOLUTION - valTR;
  int lightBL = ADC_RESOLUTION - valBL;
  int lightBR = ADC_RESOLUTION - valBR;

  // 3. Calculate Sector Light Intensities
  int avgLeft   = (lightTL + lightBL) / 2;
  int avgRight  = (lightTR + lightBR) / 2;
  int avgTop    = (lightTL + lightTR) / 2;
  int avgBottom = (lightBL + lightBR) / 2;
  int avgTotal  = (avgLeft + avgRight) / 2;

  // 4. Calculate Differential Tracking Errors
  int errorHorizontal = avgLeft - avgRight; // Positive = Sun on Left, Negative = Sun on Right
  int errorVertical   = avgTop - avgBottom; // Positive = Sun is High, Negative = Sun is Low

  // Calculate percentage (0% = Dark, 100% = Full Light)
  float pctTotal = (avgTotal / (float)ADC_RESOLUTION) * 100.0f;

  // 5. Determine Sun Tracking Direction
  String sunDirection = "";
  if (avgTotal < 150) {
    sunDirection = "NIGHT / INSUFFICIENT SUNLIGHT (Tracker Sleeps)";
  } else if (abs(errorHorizontal) <= DEADBAND_ADC) {
    sunDirection = "BALANCED (In Deadband -> Solar Shafts Locked)";
  } else if (errorHorizontal > DEADBAND_ADC) {
    sunDirection = "SUN ON LEFT -> Stepper moves Positive (+)";
  } else {
    sunDirection = "SUN ON RIGHT -> Stepper moves Negative (-)";
  }


  // 5. Print Clean Live Telemetry Dashboard
  printlnOut(F("----------------------------------------------------------"));
  printOut(F(" [LDR RAW READINGS]   TL: ")); printOut(String(valTL));
  printOut(F(" | TR: "));               printOut(String(valTR));
  printOut(F(" | BL: "));               printOut(String(valBL));
  printOut(F(" | BR: "));               printlnOut(String(valBR));

  printOut(F(" [SECTOR AVERAGES]   LEFT: "));  printOut(String(avgLeft));
  printOut(F(" | RIGHT: "));                  printOut(String(avgRight));
  printOut(F(" | INTENSITY: "));              printOut(String(pctTotal, 1));
  printlnOut(F("%"));

  printOut(F(" [DIFFERENTIAL ERR]  H-Delta: ")); printOut(String(errorHorizontal));
  printOut(F(" (Deadband: +/-"));             printOut(String(DEADBAND_ADC));
  printlnOut(F(")"));

  printOut(F(" [TRACKING STATUS]   >>> "));   printlnOut(sunDirection);
  printlnOut();

  delay(600); // Update cadence
}
