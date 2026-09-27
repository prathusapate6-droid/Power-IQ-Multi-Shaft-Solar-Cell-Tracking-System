/*
================================================================================
 POWER IQ — STM32F103C8T6 Blue Pill LDR Sensor Diagnostic Test
 Target  : STM32F103C8T6 Blue Pill (72 MHz ARM Cortex-M3)
 Author  : Prathamesh Sapate (Lead) & POWER IQ Engineering Team
================================================================================

 WIRING TO STM32 BLUE PILL:
   - LDR 1 Signal -> STM32 Pin PA0 (ADC1_IN0)
   - LDR 2 Signal -> STM32 Pin PA1 (ADC1_IN1)
   - LDR 3 Signal -> STM32 Pin PA4 (ADC1_IN4) [If connected]
   - LDR 4 Signal -> STM32 Pin PA5 (ADC1_IN5) [If connected]
   - LDR VCC      -> STM32 3.3V
   - LDR GND      -> STM32 GND

 NOTE ON LDR TYPES:
   Case 1 (Ready-made 3-pin LDR Module with chip & blue screw):
     - VCC -> STM32 3.3V
     - GND -> STM32 GND
     - AO (Analog Out) or DO (Digital Out) -> STM32 Pin PA0 / PA1

   Case 2 (Simple 2-pin bare LDR component):
     - One leg -> 3.3V
     - Other leg -> STM32 Pin PA0 AND a 10kΩ resistor to GND (Voltage Divider)
================================================================================
*/

#include <Arduino.h>

// Pins configured on STM32 Blue Pill
#define LDR1_PIN PA0
#define LDR2_PIN PA1
#define LDR3_PIN PA4
#define LDR4_PIN PA5

#define STATUS_LED PC13 // Onboard Blue LED

// Serial Interface for USB-to-TTL on PA9/PA10
#if defined(Serial1)
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
  pinMode(LDR1_PIN, INPUT);
  pinMode(LDR2_PIN, INPUT);
  pinMode(LDR3_PIN, INPUT);
  pinMode(LDR4_PIN, INPUT);
  pinMode(STATUS_LED, OUTPUT);

  digitalWrite(STATUS_LED, LOW); // LED ON

  Serial.begin(9600);
  #if HAS_SERIAL1
    Serial1.begin(9600);
  #endif

  delay(1000);

  printlnOut();
  printlnOut(F("=========================================================="));
  printlnOut(F(" POWER IQ — STM32 Blue Pill LDR Sensor Live Monitor       "));
  printlnOut(F(" Pins Monitored: PA0 (LDR1), PA1 (LDR2), PA4, PA5         "));
  printlnOut(F("=========================================================="));
  printlnOut(F(" Move your hand over LDR or shine phone torch to test!    "));
  printlnOut();
}

void loop() {
  // Read Analog values (STM32 12-bit ADC: 0 to 4095)
  int a1 = analogRead(LDR1_PIN);
  int a2 = analogRead(LDR2_PIN);
  int a3 = analogRead(LDR3_PIN);
  int a4 = analogRead(LDR4_PIN);

  // Read Digital logic values (HIGH / LOW)
  int d1 = digitalRead(LDR1_PIN);
  int d2 = digitalRead(LDR2_PIN);

  // Calculate approximate voltage on 3.3V scale
  float v1 = (a1 / 4095.0f) * 3.3f;
  float v2 = (a2 / 4095.0f) * 3.3f;

  // Print Live Readings
  printOut(F("LDR-1 (PA0): ADC=")); printOut(String(a1));
  printOut(F(" ["));                 printOut(String(v1, 2));
  printOut(F("V, Dig="));             printOut(String(d1));
  printOut(F("]  |  LDR-2 (PA1): ADC=")); printOut(String(a2));
  printOut(F(" ["));                 printOut(String(v2, 2));
  printOut(F("V, Dig="));             printOut(String(d2));
  printlnOut(F("]"));

  // Toggle onboard LED on every reading to confirm STM32 is alive
  digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));

  delay(500); // 2 readings per second
}
