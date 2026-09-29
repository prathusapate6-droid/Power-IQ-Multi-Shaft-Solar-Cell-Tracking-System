/*
================================================================================
 POWER IQ — STM32 4-QUADRANT LDR SENSOR DIAGNOSTIC TESTER
 Subsystem: LDR Hardware Live Testing & Polarity Analyzer
 Platform : STM32F103C8T6 / STM32F103C6 "Blue Pill" @ 72 MHz
 Baud Rate: 115200 Baud (USART1 on PA9/PA10)
 Pin Mapping:
   - PA0 -> LDR TOP 1 (ADC1_IN0)
   - PA1 -> LDR TOP 2 (ADC1_IN1)
   - PA4 -> LDR BOT 1 (ADC1_IN4)
   - PA5 -> LDR BOT 2 (ADC1_IN5)
   - PC13-> On-board Status LED (Blinks on every sample)
================================================================================
*/

#include <Arduino.h>

#define PIN_LDR_TOP1 PA0
#define PIN_LDR_TOP2 PA1
#define PIN_LDR_BOT1 PA4
#define PIN_LDR_BOT2 PA5
#define PIN_STATUS_LED PC13

const float VREF = 3.3f;
unsigned long lastPrintMs = 0;
int sampleDelayMs = 300; // 300ms live stream refresh
unsigned long sampleCount = 0;

// Read ADC with 16-sample averaging for clean noise-free values
int readAveragedADC(uint8_t pin) {
  long sum = 0;
  for (int i = 0; i < 16; i++) {
    sum += analogRead(pin);
    delayMicroseconds(50);
  }
  return (int)(sum / 16);
}

// Generate simple ASCII bar for quick visual level check
void printAsciiBar(int val, int maxVal = 4095) {
  int bars = (val * 10) / maxVal;
  if (bars > 10) bars = 10;
  if (bars < 0) bars = 0;
  Serial1.print(F("["));
  for (int i = 0; i < 10; i++) {
    if (i < bars) Serial1.print(F("="));
    else Serial1.print(F("."));
  }
  Serial1.print(F("] "));
  int pct = (val * 100) / maxVal;
  if (pct < 10) Serial1.print(F(" "));
  if (pct < 100) Serial1.print(F(" "));
  Serial1.print(pct);
  Serial1.print(F("%"));
}

void setup() {
  // Initialize Serial1 (PA9=TX, PA10=RX @ 115200 Baud)
  Serial1.begin(115200);
  Serial.begin(115200); // USB CDC fallback if available

  pinMode(PIN_LDR_TOP1, INPUT);
  pinMode(PIN_LDR_TOP2, INPUT);
  pinMode(PIN_LDR_BOT1, INPUT);
  pinMode(PIN_LDR_BOT2, INPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);

  // Triple blink on start
  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_STATUS_LED, LOW);
    delay(100);
    digitalWrite(PIN_STATUS_LED, HIGH);
    delay(100);
  }

  Serial1.println();
  Serial1.println(F("================================================================================"));
  Serial1.println(F("           POWER IQ — STM32 LDR 4-CHANNEL SENSOR DIAGNOSTIC TESTER              "));
  Serial1.println(F("================================================================================"));
  Serial1.println(F(" Pins Monitored: PA0 (TOP1), PA1 (TOP2), PA4 (BOT1), PA5 (BOT2) @ 115200 Baud   "));
  Serial1.println(F(" TEST INSTRUCTIONS:                                                             "));
  Serial1.println(F(" 1. Shine light / flashlight on TOP LDRs -> Watch TOP ADC & Voltage rise/fall.  "));
  Serial1.println(F(" 2. Shine light on BOTTOM LDRs -> Watch BOT ADC & Voltage rise/fall.           "));
  Serial1.println(F(" 3. Cover each LDR with your finger -> Watch individual channel response.       "));
  Serial1.println(F("================================================================================\n"));
  Serial1.flush();
}

void loop() {
  unsigned long now = millis();
  if (now - lastPrintMs >= sampleDelayMs) {
    lastPrintMs = now;
    sampleCount++;

    // Toggle heartbeat LED
    digitalWrite(PIN_STATUS_LED, (sampleCount % 2 == 0) ? LOW : HIGH);

    // Read all 4 LDR channels
    int rawT1 = readAveragedADC(PIN_LDR_TOP1);
    int rawT2 = readAveragedADC(PIN_LDR_TOP2);
    int rawB1 = readAveragedADC(PIN_LDR_BOT1);
    int rawB2 = readAveragedADC(PIN_LDR_BOT2);

    // Calculate real voltages
    float vT1 = (rawT1 * VREF) / 4095.0f;
    float vT2 = (rawT2 * VREF) / 4095.0f;
    float vB1 = (rawB1 * VREF) / 4095.0f;
    float vB2 = (rawB2 * VREF) / 4095.0f;

    // Inverted values (4095 - raw)
    int invT1 = 4095 - rawT1;
    int invT2 = 4095 - rawT2;
    int invB1 = 4095 - rawB1;
    int invB2 = 4095 - rawB2;

    int avgTopRaw = (rawT1 + rawT2) / 2;
    int avgBotRaw = (rawB1 + rawB2) / 2;
    int diffRaw = avgTopRaw - avgBotRaw;

    int avgTopInv = (invT1 + invT2) / 2;
    int avgBotInv = (invB1 + invB2) / 2;
    int diffInv = avgTopInv - avgBotInv;

    // Print Clean Formatted Telemetry Block
    Serial1.println(F("--------------------------------------------------------------------------------"));
    Serial1.print(F("Sample #"));
    Serial1.print(sampleCount);
    Serial1.print(F(" | Active Readings (Ref: "));
    Serial1.print(VREF, 1);
    Serial1.println(F("V):"));

    // Channel 1: TOP 1 (PA0)
    Serial1.print(F(" [1. TOP 1 - PA0] Raw ADC: "));
    if (rawT1 < 1000) Serial1.print(F(" "));
    if (rawT1 < 100) Serial1.print(F(" "));
    Serial1.print(rawT1);
    Serial1.print(F(" ("));
    Serial1.print(vT1, 2);
    Serial1.print(F("V) | 4095-Raw: "));
    if (invT1 < 1000) Serial1.print(F(" "));
    if (invT1 < 100) Serial1.print(F(" "));
    Serial1.print(invT1);
    Serial1.print(F(" | Bar: "));
    printAsciiBar(rawT1);
    Serial1.println();

    // Channel 2: TOP 2 (PA1)
    Serial1.print(F(" [2. TOP 2 - PA1] Raw ADC: "));
    if (rawT2 < 1000) Serial1.print(F(" "));
    if (rawT2 < 100) Serial1.print(F(" "));
    Serial1.print(rawT2);
    Serial1.print(F(" ("));
    Serial1.print(vT2, 2);
    Serial1.print(F("V) | 4095-Raw: "));
    if (invT2 < 1000) Serial1.print(F(" "));
    if (invT2 < 100) Serial1.print(F(" "));
    Serial1.print(invT2);
    Serial1.print(F(" | Bar: "));
    printAsciiBar(rawT2);
    Serial1.println();

    // Channel 3: BOT 1 (PA4)
    Serial1.print(F(" [3. BOT 1 - PA4] Raw ADC: "));
    if (rawB1 < 1000) Serial1.print(F(" "));
    if (rawB1 < 100) Serial1.print(F(" "));
    Serial1.print(rawB1);
    Serial1.print(F(" ("));
    Serial1.print(vB1, 2);
    Serial1.print(F("V) | 4095-Raw: "));
    if (invB1 < 1000) Serial1.print(F(" "));
    if (invB1 < 100) Serial1.print(F(" "));
    Serial1.print(invB1);
    Serial1.print(F(" | Bar: "));
    printAsciiBar(rawB1);
    Serial1.println();

    // Channel 4: BOT 2 (PA5)
    Serial1.print(F(" [4. BOT 2 - PA5] Raw ADC: "));
    if (rawB2 < 1000) Serial1.print(F(" "));
    if (rawB2 < 100) Serial1.print(F(" "));
    Serial1.print(rawB2);
    Serial1.print(F(" ("));
    Serial1.print(vB2, 2);
    Serial1.print(F("V) | 4095-Raw: "));
    if (invB2 < 1000) Serial1.print(F(" "));
    if (invB2 < 100) Serial1.print(F(" "));
    Serial1.print(invB2);
    Serial1.print(F(" | Bar: "));
    printAsciiBar(rawB2);
    Serial1.println();

    // Comparison Summary
    Serial1.print(F(" >>> SECTOR SUMMARY: TOP Avg: "));
    Serial1.print(avgTopRaw);
    Serial1.print(F(" ("));
    Serial1.print(((vT1 + vT2) / 2.0f), 2);
    Serial1.print(F("V) | BOT Avg: "));
    Serial1.print(avgBotRaw);
    Serial1.print(F(" ("));
    Serial1.print(((vB1 + vB2) / 2.0f), 2);
    Serial1.print(F("V) | DIFF (TOP-BOT): "));
    if (diffRaw >= 0) Serial1.print(F("+"));
    Serial1.println(diffRaw);

    // Analysis Guidance
    if (abs(diffRaw) <= 150) {
      Serial1.println(F(" >>> STATUS: [LIGHT BALANCED] Top and Bottom sensors receive equal illumination."));
    } else if (diffRaw > 150) {
      Serial1.println(F(" >>> STATUS: [LIGHT BIAS -> TOP] Stronger light on TOP sector (PA0/PA1)."));
    } else {
      Serial1.println(F(" >>> STATUS: [LIGHT BIAS -> BOTTOM] Stronger light on BOTTOM sector (PA4/PA5)."));
    }

    Serial1.flush();
  }

  // Serial input commands
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == 'f' || c == 'F') {
      sampleDelayMs = 150;
      Serial1.println(F("\n[CMD] Fast mode active (150ms)\n"));
    } else if (c == 's' || c == 'S') {
      sampleDelayMs = 500;
      Serial1.println(F("\n[CMD] Slow mode active (500ms)\n"));
    }
  }
}
