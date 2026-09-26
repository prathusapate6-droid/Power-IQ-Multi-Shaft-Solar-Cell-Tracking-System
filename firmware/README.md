# Embedded Firmware Architecture: POWER IQ Controller

**Project:** Low-Power Multi-Shaft Solar Cell Tracking System  
**Lead Engineer:** Prathamesh Sapate  
**Team Members:** Shreyash Pachade, Vansh Dobhale, Prachi Ronge  
**Target Hardware:** Arduino UNO R3 (ATmega328P) / ESP32 / STM32  
**Actuator:** NEMA 17 Stepper Motor ($1.8^\circ$, 200 steps/rev)  
**Default Driver:** L298N Dual H-Bridge (with HAL switch to A4988 / TMC2209 Step/Dir)  

---

## Directory Overview

```
firmware/
├── power_iq_controller/            # Production modular firmware
│   ├── config.h                    # Central pinouts, ratios, & travel limits
│   ├── motor_driver.h / .cpp       # Hardware Abstraction Layer (L298N & Step/Dir)
│   ├── kinematics.h / .cpp         # Non-blocking angle calculation & step generator
│   ├── command_parser.h / .cpp     # Interactive ASCII / JSON serial protocol parser
│   └── power_iq_controller.ino     # Main loop & status LED coordinator
├── tests/                          # Standalone bench verification sketches
│   ├── test_01_l298n_motor_rotation/
│   ├── test_02_step_accuracy/
│   ├── test_03_kinematic_angle_calibration/
│   └── test_04_serial_command_protocol/
├── serial_telemetry_bridge.py      # Python USB-UART bridge with AI health integration
├── arduino_stepper_control/        # Original single-file sketch (kept for reference)
└── README.md
```

---

## Hardware Pinout & Wiring

| Subsystem | Signal | Arduino UNO Pin | Destination / Driver Pin | Notes |
| :--- | :--- | :---: | :---: | :--- |
| **Stepper Motor (L298N)** | Phase A+ | **Pin 8** | `IN1` | 4-phase cyclic stepping |
| | Phase A- | **Pin 9** | `IN2` | 4-phase cyclic stepping |
| | Phase B+ | **Pin 10** | `IN3` | 4-phase cyclic stepping |
| | Phase B- | **Pin 11** | `IN4` | 4-phase cyclic stepping |
| **Driver HAL (Alternative)**| STEP / DIR / EN | **Pins 8, 9, 7** | Step/Dir Drivers | Set `#define ACTIVE_DRIVER DRIVER_TYPE_STEP_DIR` in `config.h` |
| **Power Supply** | Motor 12V DC | External V+ | `12V / VMS` | Do NOT power motor from Arduino 5V! |
| | Logic Ground | **GND** | `GND` | **Must share common ground with Arduino** |
| **Status Indicator** | Heartbeat LED | **Pin 13** | Onboard LED | Fast blink when moving, slow pulse when idle |
| **Sensors (Future)** | East / West LDR | **A0 / A1** | CdS Divider | Differential solar tracking |
| | Solar Bus Monitor | **A4 (SDA), A5 (SCL)** | INA219 I2C | Voltage, Current, Power telemetry |

---

## Kinematics & Calibration Formula

For a $1.8^\circ$ stepper (200 full steps/rev) driving a central worm gear coupled to rotating PV shaft gears:

$$\text{Steps per Degree} = \frac{\text{motorStepsPerRev} \times \text{wormRatio} \times \text{microstepping}}{360^\circ}$$

- **30:1 Worm Gear Ratio (Full Stepping):**
  $$\frac{200 \times 30 \times 1}{360} = 16.6667\text{ steps/deg}$$
- **40:1 Worm Gear Ratio (Full Stepping):**
  $$\frac{200 \times 40 \times 1}{360} = 22.2222\text{ steps/deg}$$

> [!NOTE]
> In `config.h`, `WORM_GEAR_TEETH` can be toggled between 30 and 40. Furthermore, steps per degree can be adjusted live from the Serial Monitor at any time using `CALIBRATE <val>`.

---

## Serial Protocol Command Set

The controller runs at **`115200` baud** (Newline line ending) with non-blocking reception:

| Command | Syntax Example | Function |
| :--- | :--- | :--- |
| **Direct Angle** | `45` or `-30` | Commands slats directly to target angle ($-90^\circ$ to $+90^\circ$) |
| **MOVE** | `MOVE 60` | Commands slats to $+60^\circ$ |
| **STATUS** | `STATUS` | Prints human-readable status, angle, motion state, driver |
| **STATUS JSON** | `STATUS JSON` | Outputs single-line JSON payload for Web UI & telemetry bridges |
| **ZERO** | `ZERO` | Sets current physical orientation as calibrated $0.0^\circ$ |
| **STOP** | `STOP` | Instant emergency halt; cuts motor coil power |
| **CALIBRATE** | `CALIBRATE 16.67` | Overwrites active steps/degree without reflashing code |
| **SPEED** | `SPEED 5` | Sets phase step delay in milliseconds |
| **OFF** | `OFF` | De-energizes all 4 H-bridge coils immediately |
| **HELP** | `HELP` | Prints quick-reference manual |

---

## Subsystem Bench Testing Guide

Before full mechanical assembly, verify individual functions using the isolated test sketches in `tests/`:

1. **`test_01_l298n_motor_rotation.ino`:**
   - Verifies coil sequence wiring.
   - Rotates motor 200 steps CW, de-energizes coils for 2s, then 200 steps CCW.
2. **`test_02_step_accuracy.ino`:**
   - Type `200` to verify exactly 1 full rotation of the motor shaft.
   - Type `-200` to verify reverse rotation with zero backlash error.
3. **`test_03_kinematic_angle_calibration.ino`:**
   - Type angles (`15`, `30`, `45`) and measure slat angle with a protractor.
   - Use `CAL <val>` to calibrate exact degrees.
4. **`test_04_serial_command_protocol.ino`:**
   - Test all serial parser commands without needing the motor physically connected.

---

## Python Telemetry Bridge & Live AI Monitor

To stream live data from your Arduino to the AI engine or test in mock mode:

```bash
# Auto-detects connected Arduino and streams 1 Hz telemetry + AI health diagnostics:
python3 firmware/serial_telemetry_bridge.py

# Or run in simulated mock mode without hardware:
python3 firmware/serial_telemetry_bridge.py --mock
```
