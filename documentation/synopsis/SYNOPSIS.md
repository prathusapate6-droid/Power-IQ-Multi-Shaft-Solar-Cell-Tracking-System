# Design and Development of a Multi-Shaft Solar Cell Tracking System

## Project Synopsis & Technical Specification

---

## 1. Project Title
**Design and Development of a Multi-Shaft Solar Cell Tracking System Using a Single Worm Gear Drive**  
*Project Code:* POWER IQ  

---

## 2. Abstract
The increasing demand for renewable energy has created the need for more efficient and energy-conscious solar tracking systems. Conventional solar trackers rotate the entire photovoltaic panel, which increases the overall moving mass, structural complexity, and actuator power consumption. These limitations reduce the practical feasibility and overall efficiency of compact and rooftop solar installations.

This project proposes the **Design and Development of a Low-Power Multi-Shaft Solar Cell Tracking System Using a Single Worm Gear Drive**. Instead of rotating the complete solar panel, the proposed system rotates only the individual rows of solar cells mounted on multiple parallel shafts. A single NEMA 17 stepper motor drives a long worm shaft, which transmits motion to multiple worm gears connected to the solar cell shafts, ensuring synchronized angular movement with minimal energy consumption.

The system utilizes light sensors (LDRs) and an embedded tracking algorithm to continuously orient the solar cells toward maximum sunlight throughout the day. By reducing the moving mass and employing a compact worm gear transmission mechanism, the design minimizes power consumption, improves mechanical efficiency, and reduces structural stress. The prototype is fabricated using 3D-printed mechanical components, making it lightweight, cost-effective, and easy to manufacture.

---

## 3. Problem Statement & Motivation

### 3.1 Background of the Problem
Traditional solar tracking systems require heavy support structures and high-torque motors because the complete solar panel is rotated. This leads to:
1. Increased installation and structural reinforcement cost.
2. Higher energy consumption for continuous tracking.
3. Greater mechanical wear on heavy pivot bearings.
4. Vulnerability to high wind shear on large planar surfaces.

### 3.2 Proposed Innovation: "Rotate the Cells, Not the Panel"
By rotating only the individual rows of PV cells using multiple synchronized shafts driven by a single motor:
- The heavy support chassis remains stationary.
- Moving mass is reduced by over $65\%$.
- A single motor replaces multiple independent actuators.
- Wind torque is distributed across narrow slats, reducing aerodynamic moment.

---

## 4. Objectives of the Project

1. Design and develop an innovative multi-shaft solar cell tracking mechanism.
2. Rotate only the solar cell rows instead of the entire solar panel to reduce moving mass.
3. Synchronize multiple rotating shafts using a single stepper motor and common worm-gear transmission.
4. Maximize solar energy harvesting by maintaining optimum solar incidence throughout the day.
5. Reduce actuator power consumption to less than $1\%$ of generated solar energy.
6. Minimize mechanical complexity and structural load compared to conventional trackers.
7. Develop a compact, lightweight, and cost-effective prototype using 3D-printed components.
8. Evaluate performance and feasibility for rooftop, commercial, off-grid, and EV charging applications.

---

## 5. System Architecture & Working Principle

```
Sunlight
   ↓
LDR Optical Sensors (Light Differential Detection)
   ↓
STM32 / ESP32 Controller (Angular Correction Algorithm)
   ↓
Stepper Motor Driver (A4988 / TMC2209 - 16x Microstepping)
   ↓
Single NEMA 17 Stepper Motor
   ↓
Continuous Central Worm Shaft
   ↓
8x Synchronized Worm Gears (40:1 Ratio)
   ↓
Multiple Parallel PV Cell Shafts
   ↓
Synchronized Solar Cell Row Rotation (+30% Energy Capture)
```

### Electrical Telemetry Pipeline:
```
Voltage & Current Sensors (INA219 / ACS712)
   ↓
Microcontroller ADC & I2C Bus
   ↓
IoT Telemetry Gateway (WiFi / MQTT)
   ↓
POWER IQ Live Dashboard
   ↓
AI Predictive Maintenance & Fault Detection Analysis
```

---

## 6. Key Components & Specifications

| Component | Specification | Function |
| :--- | :--- | :--- |
| **Microcontroller** | STM32F401RE / ESP32 | Main controller, sensor processing & step generation |
| **Actuator** | NEMA 17 Stepper Motor ($1.8^\circ$, $1.5\text{ A}$) | Drives central worm gear shaft |
| **Driver** | TMC2209 / A4988 (1/16 microstepping) | Microstepping control and current regulation |
| **Transmission** | Steel Worm Shaft + 40:1 Worm Wheels | Synchronous transmission to 8 parallel shafts |
| **Bearings** | 608ZZ Radial Ball Bearings | Low-friction support for rotating shafts |
| **Sensors** | 4-Quadrant CdS LDR Sensor Array | Optical differential sun tracking |
| **Electrical Sensing** | INA219 ($I^2C$) & ACS712 | Solar power ($V \times I$) and motor current sensing |
| **Structural Frame** | 3D-Printed PETG Components | Lightweight, modular chassis |

---

## 7. Real-Life Applications & Future Potential

- **Rooftop Solar Installations:** Reduced structural weight allows installation on lightweight roofs.
- **Commercial & Agricultural Canopies:** Ideal for solar water pumping and greenhouse canopies.
- **Solar-Powered EV Charging:** Maximizes daytime charging energy capture in parking structures.
- **Off-Grid Microgrids & Research Labs:** Modular design enables scalable deployment in remote areas.

---

## 8. Document Reference Files in this Repository

- Original Word Document: [`Synopsis_Multi_Shaft_Solar_Cell_Tracking_System.docx`](Synopsis_Multi_Shaft_Solar_Cell_Tracking_System.docx)
- Original PDF Document: [`Synopsis_Multi_Shaft_Solar_Cell_Tracking_System.pdf`](Synopsis_Multi_Shaft_Solar_Cell_Tracking_System.pdf)
