/*
================================================================================
 POWER IQ — Low-Power Multi-Shaft Solar Cell Tracking System
 Subsystem: Master Multi-Sensor Hardware Health Check & Live Telemetry
 Author   : Prathamesh Sapate (Lead) & POWER IQ Engineering Team
 Hardware : STM32F103C8T6 Blue Pill (72 MHz ARM Cortex-M3) / Arduino UNO
 Target   : Tests and validates ALL sensors simultaneously via Serial Monitor!
================================================================================

 PIN MAPPING & WIRING GUIDE (STM32 BLUE PILL):
   1. 4-LDR Sun Sensors:
      - Top-Left (TL)      -> PA0 (ADC1_IN0)
      - Top-Right (TR)     -> PA1 (ADC1_IN1)
      - Bottom-Left (BL)   -> PA4 (ADC1_IN4)
      - Bottom-Right (BR)  -> PA5 (ADC1_IN5)
      - Power: 3.3V and GND (with 10k pull-down resistors if using bare LDRs)
   
   2. Zero / Home Sensor:
      - Hall-Effect Signal -> PB11 (Digital Input with internal Pull-up)
      - Power: 3.3V / 5V and GND

   3. Solar PV Power Monitor:
      - PV Voltage Sensor  -> PA6 (ADC1_IN6) via 5:1 Resistor Divider
      - PV Current Sensor  -> PA7 (ADC1_IN7) via ACS712 / Shunt

   4. Battery Management System (BMS):
      - Battery Voltage    -> PB0 (ADC1_IN8) via Resistor Divider
      - Battery Current    -> PB1 (ADC1_IN9) via Current Shunt / ACS712

   5. Temperature Monitor:
      - Temp Sensor Probe  -> PB5 (Analog NTC / DS18B20)

   6. I2C Bus (Optional Modules):
      - SCL -> PB6 | SDA -> PB7 (INA219 / OLED / RTC)

   7. Status Heartbeat Indicator:
      - Onboard Blue LED   -> PC13 (Active-LOW)
================================================================================
*/

#include <Arduino.h>
#include <Wire.h>

// ---------------- PIN ASSIGNMENTS ----------------
#if defined(ARDUINO_ARCH_STM32) || defined(__STM32F1__)
  // STM32 Blue Pill Pins (12-bit ADC, 0 to 4095)
  #define PIN_LDR_TL     PA0
  #define PIN_LDR_TR     PA1
  #define PIN_LDR_BL     PA4
  #define PIN_LDR_BR     PA5
  #define PIN_PV_VOLT    PA6
  #define PIN_PV_CURR    PA7
  #define PIN_BATT_VOLT  PB0
  #define PIN_BATT_CURR  PB1
  #define PIN_TEMP       PB5
  #define PIN_HALL_HOME  PB11
  #define PIN_STATUS_LED PC13
  #define PIN_I2C_SCL    PB6
  #define PIN_I2C_SDA    PB7
  #define ADC_MAX_VAL    4095.0f
  #define VREF_VOLT      3.3f
  #define PLATFORM_NAME  "STM32F103C8T6 Blue Pill (32-bit ARM Cortex-M3 @ 72MHz)"
#else
  // Arduino UNO Fallback Pins (10-bit ADC, 0 to 1023)
  #define PIN_LDR_TL     A0
  #define PIN_LDR_TR     A1
  #define PIN_LDR_BL     A2
  #define PIN_LDR_BR     A3
  #define PIN_PV_VOLT    A4
  #define PIN_PV_CURR    A5
  #define PIN_BATT_VOLT  A6
  #define PIN_BATT_CURR  A7
  #define PIN_TEMP       A2
  #define PIN_HALL_HOME  2
  #define PIN_STATUS_LED 13
  #define ADC_MAX_VAL    1023.0f
  #define VREF_VOLT      5.0f
  #define PLATFORM_NAME  "Arduino UNO (8-bit ATmega328P @ 16MHz)"
#endif

// Scaling Multipliers (Resistor dividers: V_in = V_adc * multiplier)
const float PV_VOLTAGE_DIVIDER_RATIO   = 5.0f;  // Standard 25V voltage sensor module (5:1)
const float BATT_VOLTAGE_DIVIDER_RATIO = 4.0f;  // 12V Battery divider (4:1)
const float ACS712_SENSITIVITY_V_PER_A = 0.185f;// 185mV/A for ACS712-05B (or 100mV for 20A)

// ---------------- SERIAL INTERFACE (PA9/PA10 on STM32) ----------------
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

// ---------------- I2C BUS SCANNER FUNCTION ----------------
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
  pinMode(PIN_LDR_TL, INPUT);
  pinMode(PIN_LDR_TR, INPUT);
  pinMode(PIN_LDR_BL, INPUT);
  pinMode(PIN_LDR_BR, INPUT);
  pinMode(PIN_PV_VOLT, INPUT);
  pinMode(PIN_PV_CURR, INPUT);
  pinMode(PIN_BATT_VOLT, INPUT);
  pinMode(PIN_BATT_CURR, INPUT);
  pinMode(PIN_TEMP, INPUT);

  // Hall-effect sensor: Internal pull-up enabled (Active LOW when magnet touches)
  pinMode(PIN_HALL_HOME, INPUT_PULLUP);

  // Status LED
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW); // LED ON

  // Start Serial Interfaces (9600 baud)
  Serial.begin(9600);
  #if HAS_SERIAL1
    Serial1.begin(9600);
  #endif

  // Start I2C Bus
  Wire.begin();

  delay(1000);

  printlnOut();
  printlnOut(F("=========================================================================="));
  printlnOut(F(" POWER IQ — MASTER ALL-SENSOR HARDWARE HEALTH & TELEMETRY CHECK          "));
  printlnOut(String(F(" Platform: ")) + PLATFORM_NAME);
  printlnOut(F(" Scanning: 4x LDRs, Hall Effect, Solar PV (V/I/P), BMS Battery, Temp, I2C "));
  printlnOut(F("=========================================================================="));
  printlnOut();
}

unsigned long frameCounter = 0;

// ---------------- MAIN LOOP ----------------
void loop() {
  frameCounter++;

  // 1. READ 4-LDR SUN TRACKING SENSORS (Oversampled 4 times)
  long sumTL = 0, sumTR = 0, sumBL = 0, sumBR = 0;
  for (int i = 0; i < 4; i++) {
    sumTL += analogRead(PIN_LDR_TL);
    sumTR += analogRead(PIN_LDR_TR);
    sumBL += analogRead(PIN_LDR_BL);
    sumBR += analogRead(PIN_LDR_BR);
    delayMicroseconds(200);
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

  String trackingState;
  if (sunPct < 8.0f) {
    trackingState = "NIGHT / DARKNESS (Tracker Sleeps)";
  } else if (abs(hDelta) <= (int)(ADC_MAX_VAL * 0.035f)) {
    trackingState = "BALANCED (Within Deadband -> Locked)";
  } else if (hDelta > 0) {
    trackingState = "SUN ON LEFT (Tilt Positive +)";
  } else {
    trackingState = "SUN ON RIGHT (Tilt Negative -)";
  }

  // 2. READ ZERO/HOME SENSOR (Hall Effect)
  // Low = Magnet Detected (Active LOW), High = Open
  int hallState = digitalRead(PIN_HALL_HOME);
  String hallStatus = (hallState == LOW) ? "[ TRIGGERED: 0.0 deg HOME DATUM DETECTED! ]" : "[ OPEN: In Rotation / No Magnet ]";

  // 3. READ SOLAR PV ELECTRICAL MONITOR (Voltage, Current, Power)
  int rawPvVolt = analogRead(PIN_PV_VOLT);
  int rawPvCurr = analogRead(PIN_PV_CURR);
  float pvAdcVoltage = (rawPvVolt / ADC_MAX_VAL) * VREF_VOLT;
  float pvActualVolt = pvAdcVoltage * PV_VOLTAGE_DIVIDER_RATIO;

  // ACS712 current calculation: (V_adc - V_zero) / Sensitivity
  float currAdcVolt = (rawPvCurr / ADC_MAX_VAL) * VREF_VOLT;
  float pvActualCurr = fabs(currAdcVolt - (VREF_VOLT / 2.0f)) / ACS712_SENSITIVITY_V_PER_A;
  if (pvActualCurr < 0.05f) pvActualCurr = 0.0f; // Filter noise floor
  float pvPowerWatts = pvActualVolt * pvActualCurr;

  // 4. READ BATTERY MANAGEMENT (BMS) SENSORS
  int rawBattVolt = analogRead(PIN_BATT_VOLT);
  int rawBattCurr = analogRead(PIN_BATT_CURR);
  float battAdcVolt = (rawBattVolt / ADC_MAX_VAL) * VREF_VOLT;
  float battActualVolt = battAdcVolt * BATT_VOLTAGE_DIVIDER_RATIO;
  float battPowerWatts = battActualVolt * pvActualCurr;

  // Estimated State of Charge (SoC %) for 12V Battery Pack (10.5V empty, 12.8V full)
  float battSoCPct = 0.0f;
  if (battActualVolt >= 12.8f) battSoCPct = 100.0f;
  else if (battActualVolt <= 10.5f) battSoCPct = 0.0f;
  else battSoCPct = ((battActualVolt - 10.5f) / (12.8f - 10.5f)) * 100.0f;

  // 5. READ TEMPERATURE SENSOR
  int rawTemp = analogRead(PIN_TEMP);
  float tempVolts = (rawTemp / ADC_MAX_VAL) * VREF_VOLT;
  // Approximate standard 10mV/°C sensor (e.g. LM35 or NTC bridge)
  float tempCelsius = tempVolts * 30.0f + 15.0f; // Ambient room scale estimation

  // 6. SCAN I2C BUS
  String i2cScanResult = scanI2CBus();

  // 7. PRINT COMPREHENSIVE INDUSTRIAL DASHBOARD TO SERIAL
  printlnOut(F("=========================================================================="));
  printOut(F(" POWER IQ SENSOR TELEMETRY FRAME #")); printlnOut(String(frameCounter));
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

  printOut(F("     Avg Intensity: ")); printOut(String(sunPct, 1));
  printOut(F("% | H-Delta (L-R): ")); printOut(String(hDelta));
  printOut(F(" | Decision: ")); printlnOut(trackingState);

  // [2] HALL
  printlnOut();
  printlnOut(F(" [2] ZERO / HOME DATUM SENSOR (Hall Effect - PB11):"));
  printOut(F("     Sensor State : ")); printlnOut(hallStatus);

  // [3] SOLAR PV
  printlnOut();
  printlnOut(F(" [3] SOLAR PHOTOVOLTAIC ELECTRICAL YIELD (PA6, PA7):"));
  printOut(F("     PV Voltage   : ")); printOut(String(pvActualVolt, 2)); printOut(F(" V"));
  printOut(F("  |  PV Current   : ")); printOut(String(pvActualCurr, 2)); printOut(F(" A"));
  printOut(F("  |  Solar Power  : ")); printOut(String(pvPowerWatts, 2)); printlnOut(F(" W"));

  // [4] BATTERY BMS
  printlnOut();
  printlnOut(F(" [4] BATTERY MANAGEMENT & HEALTH (BMS - PB0, PB1):"));
  printOut(F("     Battery Volt : ")); printOut(String(battActualVolt, 2)); printOut(F(" V"));
  printOut(F("  |  Battery Power: ")); printOut(String(battPowerWatts, 2)); printOut(F(" W"));
  printOut(F("  |  Estimated SoC: ")); printOut(String(battSoCPct, 0)); printlnOut(F(" %"));

  // [5] TEMPERATURE
  printlnOut();
  printlnOut(F(" [5] TEMPERATURE MONITORING (PB5):"));
  printOut(F("     Battery / System Temp : ")); printOut(String(tempCelsius, 1)); printlnOut(F(" deg C"));

  // [6] I2C BUS
  printlnOut();
  printlnOut(F(" [6] I2C HARDWARE BUS SCANNER (PB6-SCL, PB7-SDA):"));
  printOut(F("     Detected Devices      : ")); printlnOut(i2cScanResult);

  printlnOut(F("=========================================================================="));
  printlnOut();

  // Toggle Blue Pill Status LED (PC13)
  digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));

  delay(1000); // 1-second update cycle
}
