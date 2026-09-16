# Hardware Circuit Architecture & Schematic

## Circuit Overview

The **POWER IQ** electrical control circuit integrates sensing, processing, motor driving, and telemetry into a compact, low-power architecture.

![Circuit Diagram](circuit_diagram.png)

---

## Key Hardware Modules & Wiring Connections

### 1. Microcontroller Unit (MCU)
- **Primary Controller:** STMicroelectronics STM32F401RE / ESP32 NodeMCU / Arduino Uno.
- **Clock Speed:** 84 MHz (STM32) / 240 MHz (ESP32).
- **Functions:** Reads optical sensor differentials, computes optimal sun elevation/azimuth, generates precise STEP/DIR pulses, and calculates real-time power metrics.

### 2. Light Sensing Array (LDR Sensors)
- **Sensors:** 4-quadrant Cadmium Sulfide (CdS) Light Dependent Resistors separated by a cross-baffle shadow collimator.
- **Connection:** Voltage divider network with $10\text{ k}\Omega$ pull-down precision resistors connected to ADC pins (`A0`, `A1`, `A2`, `A3`).
- **Logic:** Compares Left vs Right sensor voltages to determine azimuthal sun tracking error. When $|\Delta V| > V_{\text{threshold}}$, the motor is commanded to microstep toward the brighter sector.

### 3. Stepper Motor Drive Subsystem
- **Motor Driver:** Allegro A4988 / Trinamic TMC2209 SilentStepStick.
- **Motor:** NEMA 17 Bipolar Stepper Motor ($1.8^\circ/\text{step}$, $200\text{ steps/rev}$, rated current $1.5\text{ A}$).
- **Microstepping:** Configured for 1/16 microstep resolution ($3,200\text{ microsteps/rev}$) via MS1, MS2, MS3 pins for smooth, vibration-free worm gear engagement.
- **Control Lines:**
  - `STEP` $\rightarrow$ MCU GPIO (PWM/Timer output)
  - `DIR` $\rightarrow$ MCU GPIO (Direction control: High = Clockwise, Low = Counter-Clockwise)
  - `ENABLE` $\rightarrow$ MCU GPIO (Active low; disabled during hold state to save energy)

### 4. Power Measurement & Telemetry
- **Voltage & Current Sensor:** INA219 High-Side DC Sensor ($I^2C$ interface) measuring array bus voltage ($0\text{–}60\text{ V}$) and current ($0\text{–}50\text{ A}$).
- **Motor Current Sensing:** ACS712-05B Hall-effect current sensor measuring drive current ($0\text{–}5\text{ A}$) to detect mechanical drag or binding.

### 5. Power Supply Distribution
- **Motor Power Bus:** $12\text{V–}24\text{V}$ DC supply to driver `VMOT` with a $100\mu\text{F}$ decoupling electrolytic capacitor.
- **Logic Bus:** LM2596 high-efficiency step-down buck converter stepping $24\text{V}$ down to clean $5\text{V}$ and $3.3\text{V}$ logic rails for MCU and sensor nodes.
