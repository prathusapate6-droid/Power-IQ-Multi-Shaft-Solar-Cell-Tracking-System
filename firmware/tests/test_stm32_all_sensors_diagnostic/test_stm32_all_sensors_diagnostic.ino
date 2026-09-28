/*
================================================================================
 POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 Subsystem: Master Multi-Sensor Hardware Diagnostic & Live Telemetry
 Platform : STM32F103C8T6 "Blue Pill" (32-bit ARM Cortex-M3 @ 72 MHz)
 Author   : Prathamesh Sapate (Lead) & POWER IQ Engineering Team
 Purpose  : 100% Dedicated STM32 Hardware Diagnostic Suite (Tests ALL Sensors!)
================================================================================

 STM32 BLUE PILL PINOUT & HARDWARE WIRING:
   1. 4-LDR Sun Tracking Sensors:
      - Top-Left (TL)      -> STM32 Pin PA0 (ADC Channel 0)
      - Top-Right (TR)     -> STM32 Pin PA1 (ADC Channel 1)
      - Bottom-Left (BL)   -> STM32 Pin PA4 (ADC Channel 4)
      - Bottom-Right (BR)  -> STM32 Pin PA5 (ADC Channel 5)
      - Power: 3.3V and GND (with 10k pull-down resistors for bare LDRs)
   
   2. Zero / Home Sensor:
      - Hall-Effect Signal -> STM32 Pin PB11 (Digital Input with internal Pull-up)
      - Power: 3.3V / 5V and GND

   3. Solar PV Power Monitor:
      - PV Voltage Sensor  -> STM32 Pin PA6 (ADC Channel 6) via 5:1 Resistor Divider
      - PV Current Sensor  -> STM32 Pin PA7 (ADC Channel 7) via ACS712 / Shunt

   4. Battery Management System (BMS):
      - Battery Voltage    -> STM32 Pin PB0 (ADC Channel 8) via Resistor Divider
      - Battery Current    -> STM32 Pin PB1 (ADC Channel 9) via ACS712 / Shunt

   5. Temperature Monitor:
      - Temp Sensor Probe  -> STM32 Pin PB5 (Analog NTC / DS18B20)

   6. I2C Peripheral Bus:
      - SCL -> STM32 Pin PB6 | SDA -> STM32 Pin PB7 (INA219 / OLED / RTC)

   7. Status Heartbeat Indicator:
      - Onboard Blue LED   -> STM32 Pin PC13 (Active-LOW)

   8. Serial Communication & PC Flashing (USART1):
      - STM32 Pin PA9  (USART1_TX) -> USB-to-TTL RX
      - STM32 Pin PA10 (USART1_RX) -> USB-to-TTL TX
================================================================================
*/

#include <Arduino.h>
#include <Wire.h>

// ---------------- DEDICATED STM32 BLUE PILL PIN ALLOCATION ----------------
#define PIN_LDR_TL     PA0   // LDR Top-Left (ADC1_IN0)
#define PIN_LDR_TR     PA1   // LDR Top-Right (ADC1_IN1)
#define PIN_LDR_BL     PA4   // LDR Bottom-Left (ADC1_IN4)
#define PIN_LDR_BR     PA5   // LDR Bottom-Right (ADC1_IN5)

#define PIN_PV_VOLT    PA6   // Solar PV Voltage (ADC1_IN6)
#define PIN_PV_CURR    PA7   // Solar PV Current (ADC1_IN7)

#define PIN_BATT_VOLT  PB0   // Battery Voltage (ADC1_IN8)
#define PIN_BATT_CURR  PB1   // Battery Current (ADC1_IN9)

#define PIN_TEMP       PB5   // Temperature Probe
#define PIN_HALL_HOME  PB11  // Hall Effect Magnet Sensor (EXTI11)

#define PIN_I2C_SCL    PB6   // I2C1 Clock
#define PIN_I2C_SDA    PB7   // I2C1 Data

#define PIN_STATUS_LED PC13  // STM32 Blue Pill Onboard LED

// STM32 Native 12-bit ADC Specs (4096 resolution steps)
const float ADC_MAX_VAL = 4095.0f;
const float VREF_VOLT   = 3.3f;

// Scaling Multipliers
const float PV_VOLTAGE_DIVIDER_RATIO   = 5.0f;   // Standard 25V voltage divider module (5:1)
const float BATT_VOLTAGE_DIVIDER_RATIO = 4.0f;   // 12V Battery divider (4:1)
const float ACS712_SENSITIVITY_V_PER_A = 0.185f; // 185mV/A for ACS712-05B

// ---------------- STM32 SERIAL ROUTING ----------------
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

// ---------------- I2C HARDWARE BUS SCANNER ----------------
String scanI2CBus() {
  String foundDevices = "";
  byte count = 0;
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    if (error == 0) {
      if (count > 0) foundDevices += ", ";
      foundDevices += "0x";
      if (address < 16) foundDevices += "0";
      foundDevices += String(address, HEX);
      if (address == 0x40) foundDevices += " (INA219 Power Monitor)";
      if (address == 0x3C) foundDevices += " (OLED Display)";
      if (address == 0x68) foundDevices += " (RTC DS3231)";
      count++;
    }
  }
  if (count == 0) return "No I2C devices detected (Bus Ready)";
  return foundDevices;
}

// ---------------- SETUP ----------------
void setup() {
  // Configure Analog Input Pins
  pinMode(PIN_LDR_TL, INPUT);
  pinMode(PIN_LDR_TR, INPUT);
  pinMode(PIN_LDR_BL, INPUT);
  pinMode(PIN_LDR_BR, INPUT);
  pinMode(PIN_PV_VOLT, INPUT);
  pinMode(PIN_PV_CURR, INPUT);
  pinMode(PIN_BATT_VOLT, INPUT);
  pinMode(PIN_BATT_CURR, INPUT);
  pinMode(PIN_TEMP, INPUT);

  // Configure Hall Effect Sensor with Internal Pull-up (Active LOW on magnet)
  pinMode(PIN_HALL_HOME, INPUT_PULLUP);

  // Configure Status LED
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW); // Turn ON LED at startup

  // Initialize USART1 Serial Ports (9600 Baud)
  Serial.begin(9600);
  #if HAS_SERIAL1
    Serial1.begin(9600);
  #endif

  // Initialize Hardware I2C (PB6=SCL, PB7=SDA)
  Wire.begin();

  delay(1000);

  printlnOut();
  printlnOut(F("=========================================================================="));
  printlnOut(F(" POWER IQ — STM32F103C8T6 MASTER HARDWARE & SENSOR HEALTH CHECK          "));
  printlnOut(F(" Architecture: 32-bit ARM Cortex-M3 @ 72 MHz (100% Pure STM32 Firmware)   "));
  printlnOut(F(" Telemetry   : 4x LDRs, Hall Effect, Solar PV, Battery BMS, Temp, I2C     "));
  printlnOut(F("=========================================================================="));
  printlnOut();
}

unsigned long frameCounter = 0;

// ---------------- MAIN LOOP ----------------
void loop() {
  frameCounter++;

  // 1. READ 4-QUADRANT LDR SUN SENSORS (12-bit ADC with 4x oversampling)
  long sumTL = 0, sumTR = 0, sumBL = 0, sumBR = 0;
  for (int i = 0; i < 4; i++) {
    sumTL += analogRead(PIN_LDR_TL);
    sumTR += analogRead(PIN_LDR_TR);
    sumBL += analogRead(PIN_LDR_BL);
    sumBR += analogRead(PIN_LDR_BR);
    delayMicroseconds(250);
  }
  int ldrTL = sumTL / 4;
  int ldrTR = sumTR / 4;
  int ldrBL = sumBL / 4;
  int ldrBR = sumBR / 4;

  int avgLeft   = (ldrTL + ldrBL) / 2;
  int avgRight  = (ldrTR + ldrBR) / 2;
  int avgTotal  = (avgLeft + avgRight) / 2;
  int hDelta    = avgLeft - avgRight; // Positive = Sun Left, Negative = Sun Right
  float sunPct  = (avgTotal / ADC_MAX_VAL) * 100.0f;

  String trackingDecision;
  if (sunPct < 8.0f) {
    trackingDecision = "NIGHT / DARKNESS (Tracker Sleeps)";
  } else if (abs(hDelta) <= (int)(ADC_MAX_VAL * 0.035f)) {
    trackingDecision = "BALANCED (Within Deadband -> Locked)";
  } else if (hDelta > 0) {
    trackingDecision = "SUN ON LEFT -> Stepper moves Positive (+)";
  } else {
    trackingDecision = "SUN ON RIGHT -> Stepper moves Negative (-)";
  }

  // 2. READ HALL-EFFECT ZERO/HOME SENSOR
  int hallState = digitalRead(PIN_HALL_HOME);
  String hallStatus = (hallState == LOW) ? "[ TRIGGERED: 0.0 deg HOME DATUM DETECTED! ]" : "[ OPEN: In Rotation / No Magnet ]";

  // 3. READ SOLAR PV MONITOR (Voltage, Current, Power)
  int rawPvVolt = analogRead(PIN_PV_VOLT);
  int rawPvCurr = analogRead(PIN_PV_CURR);
  float pvAdcVolt = (rawPvVolt / ADC_MAX_VAL) * VREF_VOLT;
  float pvActualVolt = pvAdcVolt * PV_VOLTAGE_DIVIDER_RATIO;

  float currAdcVolt = (rawPvCurr / ADC_MAX_VAL) * VREF_VOLT;
  float pvActualCurr = fabs(currAdcVolt - (VREF_VOLT / 2.0f)) / ACS712_SENSITIVITY_V_PER_A;
  if (pvActualCurr < 0.05f) pvActualCurr = 0.0f; // Noise filter
  float pvPowerWatts = pvActualVolt * pvActualCurr;

  // 4. READ BATTERY MANAGEMENT (BMS) SENSORS
  int rawBattVolt = analogRead(PIN_BATT_VOLT);
  float battAdcVolt = (rawBattVolt / ADC_MAX_VAL) * VREF_VOLT;
  float battActualVolt = battAdcVolt * BATT_VOLTAGE_DIVIDER_RATIO;
  float battPowerWatts = battActualVolt * pvActualCurr;

  // Estimated State of Charge (SoC %) for 12V Battery Pack
  float battSoCPct = 0.0f;
  if (battActualVolt >= 12.8f) battSoCPct = 100.0f;
  else if (battActualVolt <= 10.5f) battSoCPct = 0.0f;
  else battSoCPct = ((battActualVolt - 10.5f) / (12.8f - 10.5f)) * 100.0f;

  // 5. READ TEMPERATURE PROBE
  int rawTemp = analogRead(PIN_TEMP);
  float tempVolts = (rawTemp / ADC_MAX_VAL) * VREF_VOLT;
  float tempCelsius = tempVolts * 30.0f + 15.0f; // Scaled temperature estimation

  // 6. SCAN I2C BUS
  String i2cDevices = scanI2CBus();

  // 7. PRINT COMPREHENSIVE INDUSTRIAL TELEMETRY DASHBOARD
  printlnOut(F("=========================================================================="));
  printOut(F(" POWER IQ STM32 SENSOR TELEMETRY FRAME #")); printlnOut(String(frameCounter));
  printlnOut(F("=========================================================================="));

  // [1] LDR
  printlnOut(F(" [1] 4-QUADRANT SUN SENSORS (LDRs):"));
  printOut(F("     Top-Left  (PA0): ")); printOut(String(ldrTL));
  printOut(F(" (")); printOut(String((ldrTL / ADC_MAX_VAL) * VREF_VOLT, 2));
  printOut(F("V) | Top-Right   (PA1): ")); printOut(String(ldrTR));
  printOut(F(" (")); printOut(String((ldrTR / ADC_MAX_VAL) * VREF_VOLT, 2)); printlnOut(F("V)"));

  printOut(F("     Bot-Left  (PA4): ")); printOut(String(ldrBL));
  printOut(F(" (")); printOut(String((ldrBL / ADC_MAX_VAL) * VREF_VOLT, 2));
  printOut(F("V) | Bot-Right   (PA5): ")); printOut(String(ldrBR));
  printOut(F(" (")); printOut(String((ldrBR / ADC_MAX_VAL) * VREF_VOLT, 2)); printlnOut(F("V)"));

  printOut(F("     Avg Intensity : ")); printOut(String(sunPct, 1));
  printOut(F("% | H-Delta (L-R): ")); printOut(String(hDelta));
  printOut(F(" | Decision: ")); printlnOut(trackingDecision);

  // [2] HALL SENSOR
  printlnOut();
  printlnOut(F(" [2] ZERO / HOME DATUM SENSOR (Hall Effect - PB11):"));
  printOut(F("     Sensor State  : ")); printlnOut(hallStatus);

  // [3] SOLAR PV
  printlnOut();
  printlnOut(F(" [3] SOLAR PHOTOVOLTAIC ELECTRICAL YIELD (PA6, PA7):"));
  printOut(F("     PV Voltage    : ")); printOut(String(pvActualVolt, 2)); printOut(F(" V"));
  printOut(F("  |  PV Current    : ")); printOut(String(pvActualCurr, 2)); printOut(F(" A"));
  printOut(F("  |  Solar Power   : ")); printOut(String(pvPowerWatts, 2)); printlnOut(F(" W"));

  // [4] BATTERY BMS
  printlnOut();
  printlnOut(F(" [4] BATTERY MANAGEMENT & HEALTH (BMS - PB0, PB1):"));
  printOut(F("     Battery Volt  : ")); printOut(String(battActualVolt, 2)); printOut(F(" V"));
  printOut(F("  |  Battery Power : ")); printOut(String(battPowerWatts, 2)); printOut(F(" W"));
  printOut(F("  |  Estimated SoC : ")); printOut(String(battSoCPct, 0)); printlnOut(F(" %"));

  // [5] TEMPERATURE
  printlnOut();
  printlnOut(F(" [5] TEMPERATURE MONITORING (PB5):"));
  printOut(F("     System / Battery Temp : ")); printOut(String(tempCelsius, 1)); printlnOut(F(" deg C"));

  // [6] I2C BUS
  printlnOut();
  printlnOut(F(" [6] I2C HARDWARE BUS SCANNER (PB6-SCL, PB7-SDA):"));
  printOut(F("     Detected Devices      : ")); printlnOut(i2cDevices);

  printlnOut(F("=========================================================================="));
  printlnOut();

  // Toggle STM32 Blue Pill onboard LED (PC13) to indicate live execution
  digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));

  delay(1000); // 1-second update cycle
}
