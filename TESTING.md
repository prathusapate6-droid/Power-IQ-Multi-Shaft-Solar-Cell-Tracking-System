# POWER IQ: Subsystem Verification & Testing Log (TESTING.md)

**Project:** Low-Power Multi-Shaft Solar Cell Tracking System  
**Lead Engineer:** Prathamesh Sapate  
**Team Members:** Shreyash Pachade, Vansh Dobhale, Prachi Ronge  
**Target Hardware:** Arduino UNO (Validation) $\rightarrow$ STM32 (Final Controller)  
**Actuator:** NEMA 17 Bipolar Stepper Motor  
**Motor Driver:** A4988 Stepper Driver (Step/Dir)  
**Position Reference:** Magnet + Hall-Effect Sensor ($0^\circ$ Home Datum)  
**Sun Sensor:** 4-Quadrant GL5528 5mm LDR Array with Shadow Cross Divider  
**Software Limits:** $-40.0^\circ \le \theta \le +40.0^\circ$ (Total $80.0^\circ$ Slat Travel)  

---

## Testing Protocol & Tracking Table

| Test ID | Phase | Objective | Hardware / Pins | Status | Notes |
| :--- | :--- | :--- | :--- | :---: | :--- |
| **TEST-01** | Phase 1 | A4988 Basic Motor Rotation (CW & CCW) | Arduino UNO + A4988 + NEMA 17 | **PASSED** | STEP/DIR pulses & coil wiring verified on workbench |
| **TEST-02** | Phase 1 | Fixed Steps & Speed Control | Arduino UNO + A4988 + NEMA 17 | **PASSED** | 200 steps (1 rev), 2500µs pulse delay verified |
| **TEST-03** | Phase 1 | Angular Movement ($-40^\circ$ to $+40^\circ$) | Arduino UNO + A4988 + NEMA 17 + Worm | **PASSED** | Calibrated to 10.556 steps/deg, silent holding verified |
| **TEST-04** | Phase 2 | Hall Sensor Magnet Detection | Arduino UNO + Hall Sensor Module | **PLANNED** | Digital transition on D2 (Active LOW/HIGH) |
| **TEST-05** | Phase 2 | Automated Homing Sequence to $0^\circ$ | Arduino + A4988 + Hall Sensor | **PLANNED** | Startup home search, step counter reset |
| **TEST-06** | Phase 3 | 4-LDR Sensor Analog Characterization | Arduino UNO / STM32 + 4x GL5528 | **IN PROGRESS** | Live 4-quadrant differential tracking test |
| **TEST-07** | Phase 3 | LDR Differential & Deadband Filtering | Arduino UNO + 4x GL5528 | **PLANNED** | Direction error, hysteresis, jitter filter |
| **TEST-08** | Phase 4 | Full Arduino Closed-Loop Tracker | Arduino + A4988 + Hall + 4-LDR | **PLANNED** | Closed-loop sun seeking within $\pm 40^\circ$ |
| **TEST-09** | Phase 5 | STM32 Motor & Driver Porting | STM32 + A4988 + NEMA 17 | **IN PROGRESS** | PB8, PB9, PB10 connected, ready for test |
| **TEST-10** | Phase 6 | STM32 USB-Serial Telemetry & Shell | STM32 USB CDC / UART | **PLANNED** | Interactive command parser & status |
| **TEST-11** | Phase 7 | STM32 $\leftrightarrow$ ESP UART Link | STM32 UART $\leftrightarrow$ ESP8266/ESP32 | **PLANNED** | Structured JSON telemetry & ACK |
| **TEST-12** | Phase 8 | ESP Cloud Ingestion & WiFi Uplink | ESP $\rightarrow$ MQTT / HTTP Cloud | **PLANNED** | Field telemetry streaming |

---

## Detailed Test Logs

### TEST-01: A4988 Basic Motor Rotation (CW & CCW)
- **Status:** **PASSED**
- **Date:** 2026-09-27
- **Objective:** Validate basic pulse generation, STEP & DIR signals, and bidirectional rotation of NEMA 17 using A4988 driver.
- **Wiring Setup:**
  - Arduino Pin 8 $\rightarrow$ A4988 `STEP`
  - Arduino Pin 9 $\rightarrow$ A4988 `DIR`
  - Arduino Pin 7 $\rightarrow$ A4988 `ENABLE` (Active LOW)
  - Arduino 5V $\rightarrow$ A4988 `VDD`
  - Arduino GND $\rightarrow$ A4988 `GND` (Logic Ground)
  - Jumper $\rightarrow$ A4988 `RESET` connected to `SLEEP`
  - External 12V DC (+) $\rightarrow$ A4988 `VMOT` (with $100\mu\text{F}$ capacitor across `VMOT` and `GND`)
  - External 12V DC (-) $\rightarrow$ A4988 `GND` (Motor Ground)
  - Motor Coil A $\rightarrow$ A4988 `1A` & `1B`
  - Motor Coil B $\rightarrow$ A4988 `2A` & `2B`
- **Code:** [`firmware/tests/test_a4988_hardware_diag/test_a4988_hardware_diag.ino`](firmware/tests/test_a4988_hardware_diag/test_a4988_hardware_diag.ino)
- **Expected Result:** Motor rotates continuously at controlled speed with no stalling.
- **Actual Result:** Verified! Motor rotates smoothly with $2500\mu\text{s}$ step delay. Electrical wiring and driver connections confirmed functional.
- **Root Cause & Fix Applied:** Jumper applied between `RESET` and `SLEEP`, step delay calibrated to $2500\mu\text{s}$ for reliable starting torque.

### TEST-02 & TEST-03: Kinematic Angular Calibration & Silent Holding
- **Status:** **PASSED**
- **Date:** 2026-09-27
- **Objective:** Calibrate exact `stepsPerDegree` to map commanded degrees to physical rotation, and eliminate motor coil hissing/buzzing noise when stationary.
- **Final Confirmed Bench Calibration:**
  $$\mathbf{stepsPerDegree = 10.556} \quad (\approx 19:1\text{ worm reduction})$$
  - Command $10^\circ \rightarrow 106\text{ steps}$ ($10.0^\circ$ physical shaft rotation)
  - Command $40^\circ \rightarrow 422\text{ steps}$ ($40.0^\circ$ physical shaft rotation)
  - Command $-40^\circ \rightarrow 422\text{ steps}$ reverse
  - Command $0^\circ \rightarrow$ Returns to datum
- **Noise & Heat Fix:** Integrated automatic coil de-energization (`ENABLE` pin pulled `HIGH` after motion completes). The motor becomes **100% silent and cold (0W idle power)** while remaining mechanically locked via the worm gear transmission.
- **Code:** [`firmware/arduino_a4988_angle_control/arduino_a4988_angle_control.ino`](firmware/arduino_a4988_angle_control/arduino_a4988_angle_control.ino) and [`firmware/power_iq_motor_controller/power_iq_motor_controller.ino`](firmware/power_iq_motor_controller/power_iq_motor_controller.ino)

### TEST-09: STM32 Blue Pill A4988 Stepper Control
- **Status:** **IN PROGRESS (Ready for Hardware Bench Run)**
- **Date:** 2026-09-27
- **Objective:** Port validated motor control and kinematic calibration ($10.556\text{ steps/deg}$) from Arduino to STM32F103C8T6 Blue Pill.
- **Wiring Setup:**
  - STM32 `PB8` $\rightarrow$ A4988 `STEP`
  - STM32 `PB9` $\rightarrow$ A4988 `DIR`
  - STM32 `PB10` $\rightarrow$ A4988 `ENABLE` (Active LOW)
  - STM32 `3.3V` $\rightarrow$ A4988 `VDD` (Logic Power)
  - STM32 `GND` $\rightarrow$ A4988 `GND` (Logic Ground)
  - USB-to-TTL `TX` $\rightarrow$ STM32 `PA10` (USART1_RX)
  - USB-to-TTL `RX` $\rightarrow$ STM32 `PA9` (USART1_TX)
  - External 12V DC $\rightarrow$ A4988 `VMOT` / `GND` (with $100\mu\text{F}$ capacitor & common GND)
- **Code:** [`firmware/stm32_a4988_motor_controller/stm32_a4988_motor_controller.ino`](firmware/stm32_a4988_motor_controller/stm32_a4988_motor_controller.ino)
- **Expected Result:** STM32 Blue Pill commands NEMA 17 via A4988 with identical accuracy and silent idle hold.

### TEST-06: 4-Quadrant GL5528 Sun Sensor Analog Characterization
- **Status:** **IN PROGRESS (Ready for Hardware Bench Run)**
- **Date:** 2026-09-27
- **Objective:** Read all 4 GL5528 LDRs, verify individual and averaged readings, calculate differential tracking error (Horizontal Left-Right and Vertical Top-Bottom), and validate deadband filtering.
- **Wiring Setup:**
  - If Arduino UNO: `A0` (TL), `A1` (TR), `A2` (BL), `A3` (BR), `5V` (VCC), `GND` (GND)
  - If STM32: `PA0` (TL), `PA1` (TR), `PA4` (BL), `PA5` (BR), `3.3V` (VCC), `GND` (GND)
- **Code:** [`firmware/tests/test_03_four_ldr_sun_sensor/test_03_four_ldr_sun_sensor.ino`](firmware/tests/test_03_four_ldr_sun_sensor/test_03_four_ldr_sun_sensor.ino)
- **Expected Result:**
  - In ambient light: All 4 LDRs report approximately balanced readings with "BALANCED" status.
  - When shining torch on left: `avgLeft > avgRight`, error positive, reports "SUN ON LEFT -> Stepper moves Positive (+)".
  - When shining torch on right: `avgRight > avgLeft`, error negative, reports "SUN ON RIGHT -> Stepper moves Negative (-)".
  - When covered / darkness: Reports "NIGHT / INSUFFICIENT SUNLIGHT (Tracker Sleeps)".






