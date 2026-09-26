# Hardware Firmware: Arduino UNO + L298N Stepper Control

## Overview

This directory contains the physical prototype firmware for the **POWER IQ Multi-Shaft Solar Cell Tracking System**. It controls the NEMA 17 stepper motor coupled to the 30:1 common worm gear transmission via an L298N Dual H-Bridge motor driver.

---

## Hardware Pinout & Wiring

| Arduino UNO Pin | L298N Driver Pin | Stepper Motor Coil | Notes |
| :---: | :---: | :---: | :--- |
| **Pin 8** | `IN1` | Coil A+ | Phase 1 & 4 High |
| **Pin 9** | `IN2` | Coil A- | Phase 2 & 3 High |
| **Pin 10** | `IN3` | Coil B+ | Phase 1 & 2 High |
| **Pin 11** | `IN4` | Coil B- | Phase 3 & 4 High |
| **GND** | `GND` | Common Ground | Connected to common system ground |
| **External 12V** | `12V / VMS` | Motor Power Bus | Independent 12V DC power supply |

---

## Kinematic Settings

- **Stepper Motor:** NEMA 17 (Bipolar, $1.8^\circ$ step angle, 200 full steps per revolution).
- **Worm Gear Transmission Ratio:** $30:1$.
- **Step Delay:** $5\text{ ms}$ between phase transitions (configurable for speed vs torque).
- **Steps Per Degree:** Calibrated at $3.5\text{ steps/degree}$ of solar cell shaft rotation.
- **Self-Locking Hold:** `motorOff()` de-energizes all four coils immediately after positioning. The worm drive mechanism locks the shafts in place mechanically, cutting idle power draw to zero.

---

## Serial Monitor Control Instructions

1. Open Arduino IDE and select **Arduino UNO** board and correct COM/Serial port (`/dev/cu.usbserial-...`).
2. Open the **Serial Monitor** at baud rate **`9600`** with **`Newline`** line ending.
3. Send desired angular commands:
   - `45` $\rightarrow$ Rotates shafts to $+45^\circ$ (Westward morning/afternoon tilt).
   - `-30` $\rightarrow$ Rotates shafts to $-30^\circ$ (Eastward morning tilt).
   - `0` $\rightarrow$ Returns all shafts to horizontal stow position ($0^\circ$).
   - `STATUS` $\rightarrow$ Displays current calibrated shaft angle.
