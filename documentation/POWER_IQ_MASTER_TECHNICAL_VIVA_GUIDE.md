# POWER IQ — Master Technical Documentation & Evaluator Defense Guide
## Multi-Shaft Solar Cell Tracking System with Edge TinyML & Dual-MCU Architecture

**Lead Engineer:** Prathamesh Sapate  
**Engineering Team:** Shreyash Pachade, Vansh Dobhale, Prachi Ronge  
**Platform Architecture:** Distributed Dual-MCU (STM32F103 Blue Pill + ESP32 DevKit V1)  
**Edge AI Engine:** In-House Random Forest (60 Trees) & Decision Tree with C++ TinyML Transpilation  
**Live Cloud Dashboard:** [https://power-iq-solar-2026-9e48c.web.app](https://power-iq-solar-2026-9e48c.web.app)  
**Repository:** [https://github.com/prathusapate6-droid/Power-IQ-Multi-Shaft-Solar-Cell-Tracking-System](https://github.com/prathusapate6-droid/Power-IQ-Multi-Shaft-Solar-Cell-Tracking-System)  

---

# Table of Contents
1. [Executive Summary & Problem Statement](#1-executive-summary--problem-statement)
2. [High-Level System Topology](#2-high-level-system-topology)
3. [Mechanical Engineering & Kinematics Derivation](#3-mechanical-engineering--kinematics-derivation)
4. [Electrical Architecture, Circuit Schematics & Pinouts](#4-electrical-architecture-circuit-schematics--pinouts)
5. [Embedded Firmware Architecture (Dual-MCU)](#5-embedded-firmware-architecture-dual-mcu)
6. [In-House AI, Machine Learning & Edge TinyML Pipeline](#6-in-house-ai-machine-learning--edge-tinyml-pipeline)
7. [IoT Cloud Gateway & Remote Dashboard](#7-iot-cloud-gateway--remote-dashboard)
8. [Comprehensive Evaluator Viva Voce Q&A Defense Manual](#8-comprehensive-evaluator-viva-voce-qa-defense-manual)

---

# 1. Executive Summary & Problem Statement

### The Industrial Problem
Photovoltaic (PV) solar panels achieve peak conversion efficiency only when solar rays strike perpendicular to the cell plane ($\theta = 0^\circ$, cosine loss factor $\cos \theta = 1.0$). Fixed-tilt solar installations suffer up to **$28\% - 42\%$ daily energy loss** due to the changing solar zenith angle.

While traditional single-axis and dual-axis solar trackers improve yield, they suffer from fatal commercial drawbacks:
1. **High Actuator Capital Expenditure (CapEx):** Conventional systems install an independent linear actuator or slewing motor on every individual solar panel row. In a 50 kW array, dozens of motors dramatically escalate initial costs.
2. **Parasitic Power Consumption:** Keeping heavy actuators continuously energized consumes up to **$8\% - 12\%$ of the extra energy harvested**, especially during motor holding against wind loads.
3. **Wind Sail Destabilization:** Large flat panels act as sails under high wind gusts, causing gear stripping and structural failure.
4. **Lack of Predictive Intelligence:** Existing commercial trackers are "dumb" mechanical followers. If a bearing seizes, a panel is soiled with bird droppings/dust, or a cable snaps, the system continues blindly until total physical breakdown occurs.

### The POWER IQ Innovation
**POWER IQ** resolves every single one of these bottlenecks through a four-tier technological synergy:
1. **Multi-Shaft Gang-Drive Kinematics:** A single high-torque NEMA 17 stepper motor simultaneously rotates **8 parallel photovoltaic shafts** via a central 19:1 self-locking worm gearbox and synchronized rack/linkage mechanism. This slashes actuator cost and structural weight by **$87.5\%$**.
2. **Zero-Power Holding (Self-Locking):** The physical 19:1 worm drive has a lead angle lower than the friction angle of the gears ($\mu \ge \tan \gamma$), completely preventing reverse back-drive under wind gusts. The controller de-energizes motor coils (`PIN_ENABLE = HIGH`) when stationary, yielding **$0.0\text{ W}$ parasitic holding power**.
3. **Deterministic Dual-MCU Architecture:** Separates hard real-time motion control and microsecond optical tracking (STM32 ARM Cortex-M3) from non-deterministic network workloads, TLS cryptographic handshakes, and cloud telemetry (ESP32).
4. **In-House Edge TinyML & Predictive Maintenance:** A dual-model machine learning pipeline (Random Forest + Decision Tree) evaluates 11 multi-modal sensor vectors to detect panel soiling, mechanical gear binding, thermal overload, and battery undervoltage before equipment damage occurs. Transpiles to a zero-heap C++ header running in **$< 5\,\mu\text{s}$** on bare-metal microcontrollers.

---

# 2. High-Level System Topology

```
+----------------------------------------------------------------------------------------------------+
|                                    POWER IQ SYSTEM ARCHITECTURE                                    |
+----------------------------------------------------------------------------------------------------+

  [ SENSING LAYER ]                        [ HARD REAL-TIME EMBEDDED ]            [ ACTUATION LAYER ]
  • 4-Quadrant LDR Array (PA0,PA1,PA4,PA5)       STM32F103 "Blue Pill"              • A4988 Driver (PB8,9,10)
  • ACS712 Current Transducer (PA7)        (32-bit ARM Cortex-M3 @ 72 MHz)         • NEMA 17 Stepper (1.8°)
  • Solar Bus Voltage Divider (PA6)        ├── Non-blocking Kinematics Engine      • 19:1 Self-Locking Worm
  • Battery Voltage Divider (PB1)          ├── ADC Filtering & Deadband (±25)      • 8 Parallel PV Shafts
  • Hall-Effect Zero Switch (PB11)         ├── Closed-Loop Tracking Slew           • Travel Clamped [-35°,+35°]
  • 10k Manual Potentiometer (PB0)         └── USART2 Hardware Serial (115200)     • 0W Holding Sleep
  • DHT11 Temp/Humidity (PB5)                           |
                                                        | Bidirectional UART Packet
                                                        | {ang, pot, mode, v_pv, i_pv, p_pv, ...}
                                                        v
                                           [ IOT & NETWORKING LAYER ]
                                               ESP32 DevKit V1
                                           (Dual-Core Xtensa @ 240 MHz)
                                           ├── Stack JSON Serialization (384B)
                                           ├── I2C 16x2 LCD Display (0x27)
                                           ├── SoftAP Emergency Hotspot (192.168.4.1)
                                           ├── HTTP Web Server (Port 80)
                                           └── Cloud Uplinks:
                                               ├── Firebase RTDB (REST PATCH)
                                               └── HiveMQ Cloud (TLS 8883 MQTT)
                                                        |
                                                        v
  [ AI & PREDICTIVE ANALYTICS ]              [ USER INTERACTION & CLOUD ]
  • Random Forest Fault Classifier (60 Trees)• Firebase Live Web Dashboard
  • Decision Tree Sun-Tracking Engine        • Mobile Responsive Jog Controls
  • TinyML Bare-Metal C++ Header (<5 µs)     • Real-time Telemetry & Health Gauges
  • Python USB-UART Diagnostics Bridge       • Autonomous Anomaly Alerting
```

---

# 3. Mechanical Engineering & Kinematics Derivation

### 3.1 Actuator & Transmission Specifications
- **Motor Type:** NEMA 17 Bipolar Hybrid Stepper Motor (Model 17HS4401).
  - Step Angle: $1.8^\circ$ per full step.
  - Steps per Motor Revolution: $\frac{360^\circ}{1.8^\circ} = 200\text{ full steps/rev}$.
  - Rated Current: $1.5\text{ A/phase}$, Holding Torque: $45\text{ N}\cdot\text{cm}$.
- **Driver:** Allegro A4988 Microstepping Bipolar Driver.
  - Microstep Resolution: Configured for Full-Step mode for maximum static torque and minimum CPU step generation overhead.
- **Worm Gearbox Transmission:**
  - Gear Ratio: $19:1$ ($1$ start thread on worm screw, $19$ teeth on worm wheel).
  - Reduction Ratio: $i = 19.0$.

### 3.2 Mathematical Kinematic Derivation (Steps per Degree)
To rotate the central drive shaft (and all 8 connected solar shafts) by exactly $1.0^\circ$, the motor must take:

$$\text{Steps per Degree} = \frac{\text{Steps per Motor Rev} \times \text{Gear Ratio}}{360^\circ}$$

$$\text{Steps per Degree} = \frac{200 \times 19}{360^\circ} = \frac{3800}{360} = \mathbf{10.5556\text{ steps/degree}}$$

- **For a $1.0^\circ$ angular increment:** $10.556\text{ steps}$.
- **For a $0.5^\circ$ fine tracking step:** $5.278 \approx 5\text{ steps}$.
- **For a $2.0^\circ$ coarse tracking step:** $21.11 \approx 21\text{ steps}$.

### 3.3 Mechanical Travel Limits & Stroke Calculations
To prevent mechanical binding, wire twisting, and structural collision with mounting brackets, the slat travel range is strictly clamped in firmware to:

$$\theta_{\text{min}} = -35.0^\circ \quad (\text{East Sunrise Position})$$
$$\theta_{\text{max}} = +35.0^\circ \quad (\text{West Sunset Position})$$
$$\theta_{\text{datum}} = 0.0^\circ \quad (\text{Solar Noon / Zenith Horizontal})$$

- **Total Angular Stroke:** $\Delta \theta = 35.0^\circ - (-35.0^\circ) = 70.0^\circ$.
- **Total Physical Step Count across full stroke:**
  $$\text{Total Stroke Steps} = 70.0^\circ \times 10.5556\text{ steps/deg} = \mathbf{738.89 \approx 739\text{ steps}}$$

### 3.4 Slew Speed & Step Pulse Timing
- **Pulse Interval in Firmware:** `TRACKING_SPEED_US = 1500` ($\mu\text{s}$ low delay + $1500\,\mu\text{s}$ high pulse = $3000\,\mu\text{s} = 3.0\text{ ms/step}$).
- **Step Frequency:** $f_{\text{step}} = \frac{1}{3.0 \times 10^{-3}\text{ s}} = 333.33\text{ steps/second}$.
- **Angular Velocity:**
  $$\omega = \frac{333.33\text{ steps/sec}}{10.5556\text{ steps/deg}} = \mathbf{31.58^\circ/\text{second}}$$
- **Time to traverse full $70^\circ$ span:**
  $$t_{\text{full\_stroke}} = \frac{70.0^\circ}{31.58^\circ/\text{s}} \approx \mathbf{2.22\text{ seconds}}$$
*Note: This rapid slew capability allows instantaneous repositioning during cloud passage or emergency hail/wind stow.*

### 3.5 Mechanical Self-Locking Principle (0W Holding Power)
A worm gear drive is geometrically self-locking when the lead angle $\gamma$ of the worm screw is less than the static friction angle $\phi$ between the worm screw and bronze gear teeth:

$$\tan \gamma \le \mu_s = \tan \phi$$

Where $\mu_s$ is the static friction coefficient ($\approx 0.15$ for steel on bronze). With a single-start $19:1$ worm, $\gamma \approx 4.8^\circ$, which is significantly below $\phi \approx 8.5^\circ$.
- **Consequence:** Torque can only be transmitted from the worm screw to the wheel, **never in reverse**.
- Even under extreme wind gusts pushing on the 8 solar slats, the worm cannot be back-driven.
- When the panels reach their target tracking angle, the firmware sets `digitalWrite(PIN_ENABLE, HIGH)`. The A4988 driver cuts coil current to $0.00\text{ A}$. The panels remain locked in place mechanically with **0.0 W parasitic holding power consumption**.

---

# 4. Electrical Architecture, Circuit Schematics & Pinouts

### 4.1 Power Distribution & Electrical Domain Isolation
The system employs a 3-tier isolated power distribution scheme:
1. **12V High-Current Rail (Motor Domain):**
   - Supplied by an external 12V 5A DC power adapter or solar-charged 12V battery.
   - Powers the A4988 `VMOT` pin directly.
   - A $100\,\mu\text{F} / 35\text{V}$ low-ESR electrolytic capacitor is connected in parallel directly across `VMOT` and `GND` to suppress inductive voltage spikes during motor switching.
2. **5V Intermediate Rail (Logic & Sensor Domain):**
   - Regulated down from 12V using a high-efficiency LM2596 DC-DC step-down buck converter.
   - Powers the ESP32 `VIN`, ACS712-05B current sensor `VCC`, and I2C 16x2 LCD display.
3. **3.3V Low-Noise Logic Rail (MCU Core Domain):**
   - Regulated independently on the STM32 Blue Pill board (RT9193 / AMS1117-3.3) and ESP32 module.
   - All GPIOs, ADC reference voltage, and UART communication run at 3.3V logic levels.
4. **Common Ground Rule:**
   - The 12V Motor Ground, 5V Buck Converter Ground, STM32 Ground, and ESP32 Ground are bonded together into a single star-ground configuration to prevent ground loops and floating reference voltages.

---

### 4.2 STM32F103 Blue Pill Complete Pin Mapping

| Pin | Function / Name | Signal Direction | Electrical Characteristics | Subsystem & Wiring Destination |
| :--- | :--- | :---: | :---: | :--- |
| **PB8** | `PIN_STEP` | Output | 3.3V Push-Pull | A4988 STEP input (Generates $1500\,\mu\text{s}$ pulses) |
| **PB9** | `PIN_DIR` | Output | 3.3V Push-Pull | A4988 DIR input (HIGH = CW/West, LOW = CCW/East) |
| **PB10**| `PIN_ENABLE` | Output | 3.3V Push-Pull | A4988 ENABLE input (Active-LOW: LOW=Energized, HIGH=0W Sleep) |
| **PB11**| `PIN_HALL_HOME`| Input | 3.3V with Pull-Up | Hall-Effect Magnetic Switch (Active-LOW, trips at $0.0^\circ$ Home) |
| **PA0** | `PIN_LDR_TOP1` | Analog Input | 12-bit ADC (ADC1_IN0, 0-3.3V) | Top-Left LDR sensor (CdS photoresistor + $10\text{k}\Omega$ pull-down) |
| **PA1** | `PIN_LDR_TOP2` | Analog Input | 12-bit ADC (ADC1_IN1, 0-3.3V) | Top-Right LDR sensor (CdS photoresistor + $10\text{k}\Omega$ pull-down) |
| **PA4** | `PIN_LDR_BOT1` | Analog Input | 12-bit ADC (ADC1_IN4, 0-3.3V) | Bottom-Left LDR sensor (CdS photoresistor + $10\text{k}\Omega$ pull-down) |
| **PA5** | `PIN_LDR_BOT2` | Analog Input | 12-bit ADC (ADC1_IN5, 0-3.3V) | Bottom-Right LDR sensor (CdS photoresistor + $10\text{k}\Omega$ pull-down) |
| **PA6** | `PIN_SOLAR_VOLT`| Analog Input | 12-bit ADC (ADC1_IN6, 0-3.3V) | Solar PV Voltage Divider ($R_1=33\text{k}\Omega, R_2=6.8\text{k}\Omega$, Ratio $5.853$) |
| **PA7** | `PIN_SOLAR_CURR`| Analog Input | 12-bit ADC (ADC1_IN7, 0-3.3V) | ACS712-05B Current Sensor (Hall-effect analog output) |
| **PB1** | `PIN_BATT_VOLT` | Analog Input | 12-bit ADC (ADC1_IN9, 0-3.3V) | 12V Battery Bus Voltage Divider ($R_1=33\text{k}\Omega, R_2=6.8\text{k}\Omega$) |
| **PB0** | `PIN_POT_MANUAL`| Analog Input | 12-bit ADC (ADC1_IN8, 0-3.3V) | 10k Linear Potentiometer Wiper (Mapped to $[-35.0^\circ, +35.0^\circ]$) |
| **PB12**| `PIN_SW_MODE` | Input | 3.3V with Pull-Up | 2-Position Hardware Auto/Manual Switch (Active-LOW to GND) |
| **PB13**| `PIN_SW_MODE_ALT`| Input | 3.3V with Pull-Up | Alternate Mode Switch input |
| **PB5** | `PIN_DHT11` | Bi-directional| 3.3V Single-Bus | DHT11 Digital Temperature & Humidity Sensor |
| **PA9** | `USART1_TX` | Output | 3.3V UART (115200) | PC USB-UART Debug Serial Monitor & Python Bridge |
| **PA10**| `USART1_RX` | Input | 3.3V UART (115200) | PC Command Interface |
| **PA2** | `USART2_TX` | Output | 3.3V UART (115200) | ESP32 IoT Gateway Telemetry Uplink (Connects to ESP32 GPIO 16) |
| **PA3** | `USART2_RX` | Input | 3.3V UART (115200) | ESP32 Command Downlink (Connects to ESP32 GPIO 17) |
| **PC13**| `PIN_STATUS_LED`| Output | Active-LOW (Onboard) | System Heartbeat LED (Toggles state every 1.0 second) |

---

### 4.3 ESP32 DevKit V1 Complete Pin Mapping

| Pin | Function / Name | Signal Direction | Voltage Level | Subsystem & Wiring Destination |
| :--- | :--- | :---: | :---: | :--- |
| **GPIO 16** | `PIN_RX2` (UART2) | Input | 3.3V CMOS | Connects to STM32 PA2 (`USART2_TX`) for incoming JSON telemetry |
| **GPIO 17** | `PIN_TX2` (UART2) | Output | 3.3V CMOS | Connects to STM32 PA3 (`USART2_RX`) for outgoing remote commands |
| **GPIO 21** | `PIN_SDA` (I2C) | Bi-directional| 3.3V Open-Drain | I2C Data line for 16x2 Character LCD (Address: `0x27`) |
| **GPIO 22** | `PIN_SCL` (I2C) | Output | 3.3V Open-Drain | I2C Clock line for 16x2 Character LCD (100 kHz standard mode) |
| **GPIO 2**  | `PIN_WIFI_LED` | Output | 3.3V Push-Pull | Onboard Blue LED (Blinks during WiFi connect, solid ON when linked) |
| **GND**     | Ground | Common Ground | 0.0V | Connected directly to STM32 GND and Power Supply Common Ground |
| **VIN / 5V**| Power In | Power Rail | 5.0V DC | Supplied from LM2596 Buck Converter 5V output |

---

### 4.4 Sensor Calibration Formulas & Signal Conditioning

#### 1. Solar PV Voltage Divider Calculation
The STM32 ADC cannot tolerate voltages exceeding $3.3\text{ V}$. For a solar panel producing up to $18.0\text{ V}$, a precision voltage divider with $R_1 = 33\,\text{k}\Omega$ and $R_2 = 6.8\,\text{k}\Omega$ is used:

$$\text{Divider Ratio} = \frac{R_1 + R_2}{R_2} = \frac{33000 + 6800}{6800} = \mathbf{5.85294}$$

$$\text{Maximum Measurable Voltage} = 3.3\text{ V} \times 5.85294 = \mathbf{19.31\text{ V}}$$

$$\text{ADC Voltage} = \left(\frac{\text{ADC Reading}}{4095.0}\right) \times 3.3\text{ V}$$

$$V_{\text{solar}} = \text{ADC Voltage} \times 5.85294$$

#### 2. ACS712-05B Current Transducer
- Current Range: $-5.0\text{ A}$ to $+5.0\text{ A}$.
- Zero-Current Voltage: $\frac{V_{cc}}{2} \approx 2.50\text{ V}$.
- Sensitivity: $185\text{ mV/A} = 0.185\text{ V/A}$.
- Software Auto-Zero Calibration: During firmware boot (`setup()`), the STM32 averages 64 ADC samples while the motor is stationary to establish `currentZeroOffset`.
- Firmware Formula:
  $$I_{\text{solar}} = \frac{V_{\text{analog\_pin}} - \text{currentZeroOffset}}{\text{currentSensitivity}}$$
- Deadband: If $|V_{\text{analog\_pin}} - \text{currentZeroOffset}| < 3.5\text{ mV}$, current is clamped to strictly $0.00\text{ A}$ to eliminate thermal sensor jitter.

---

# 5. Embedded Firmware Architecture (Dual-MCU)

### 5.1 Why Dual-MCU? (Evaluator Defense Rationale)
A frequent critique by academic evaluators is: *"Why did you use two microcontrollers (STM32 + ESP32) instead of running everything on the ESP32?"*

Here is the exact engineering justification:

```
+---------------------------------------------------------------------------------------+
|                                    WORKLOAD PROFILING                                 |
+---------------------------------------------------------------------------------------+
| STM32F103 (Motion & Physics Core)          | ESP32 (Cloud & Network Gateway)          |
+--------------------------------------------+------------------------------------------+
| • Hard Real-Time Deterministic Execution   | • Non-Deterministic Network Workloads    |
| • Jitter-free microstep generation (1.5 ms)| • Wi-Fi 802.11 Beacon scanning & DHCP     |
| • Continuous 12-bit ADC differential LDR   | • TLS 1.3 cryptographic handshakes (8883)|
| • Hardware Hall-effect interrupt homing    | • HTTP REST PATCH packet streaming       |
| • Zero RTOS context-switching overhead     | • Dual-Core FreeRTOS scheduling          |
| • Zero wireless interference or crashes    | • SoftAP HTTP server & dynamic Web UI    |
+---------------------------------------------------------------------------------------+
```

If Wi-Fi encounters packet loss or an MQTT reconnect loop, an ESP32 can block execution for 200 ms to 3 seconds. If motor step timing is shared on that chip, the stepper motor stutters, misses steps, loses positional synchronization, and vibrates violently. By decoupling to STM32, solar tracking executes with **zero jitter**, while ESP32 handles all networking independently.

---

### 5.2 Optical Sun Tracking & Deadband Algorithm
The 4-quadrant LDR sensor array is split into two physical sectors:
- **Top Sector Average:** $\text{Top} = \frac{\text{LDR\_TOP1} + \text{LDR\_TOP2}}{2}$
- **Bottom Sector Average:** $\text{Bottom} = \frac{\text{LDR\_BOT1} + \text{LDR\_BOT2}}{2}$
- **Differential Error ($\Delta L$):**
  $$\Delta L = \text{Top} - \text{Bottom}$$

```
                           [ Read 4 LDRs (PA0,PA1,PA4,PA5) ]
                                          |
                                          v
                      [ Compute Average Intensity: (Top + Bot) / 2 ]
                                          |
                      +-------------------+-------------------+
                      |                                       |
             Avg < 500 counts?                      Avg >= 500 counts?
             (Nightfall Mode)                       (Active Daylight)
                      |                                       |
                      v                                       v
             [ Move to 0.0° Datum ]             [ Compute Differential Error: ]
             [ Turn Motor OFF (0W)]             [    Delta = Top - Bottom     ]
                      |                                       |
                      |                 +---------------------+---------------------+
                      |                 |                     |                     |
                      |           Delta > +25?          Delta < -25?         |Delta| <= 25?
                      |           (Sun to West)         (Sun to East)        (Deadband Settle)
                      |                 |                     |                     |
                      |                 v                     v                     v
                      |          Angle < +35.0°?       Angle > -35.0°?      [ Deadband Met! ]
                      |                 |                     |             [ Align Within ]
                      |          +------+------+       +------+------+      [  ±0.5 deg    ]
                      |          |             |       |             |              |
                      |         Yes            No     Yes            No             v
                      |          |             |       |             |      [ Motor Sleep: ]
                      |          v             v       v             v      [ PIN_ENABLE=HI]
                      |      [Step CW]      [Hold] [Step CCW]     [Hold]    [  (0W Sleep)  ]
```

### Why a $\pm 25$ ADC Counts Deadband?
- Photovoltaic power output follows a cosine relationship: $P = P_{\text{peak}} \cos(\theta_{\text{error}})$.
- For an alignment error of $\pm 2.0^\circ$, $\cos(2.0^\circ) = 0.9994$ ($99.94\%$ of theoretical peak power; loss is merely $0.06\%$).
- Without a deadband, ambient atmospheric turbulence, passing high-altitude clouds, and ADC thermal noise would cause the stepper motor to hunt continuously back and forth, consuming excessive power and wearing out the gear teeth.
- The $\pm 25$ ADC count deadband allows the motor to remain completely powered off ($0\text{ W}$) for **$85\% - 92\%$ of the operational daylight hours**, only waking for a quick 2-degree correction every 8 to 12 minutes as the sun moves across the sky ($15^\circ/\text{hour}$).

---

### 5.3 ESP32 Firmware & Cloud Gateway Implementation
- **Hardware UART Ring Buffer:** ESP32 `Serial2` runs at 115200 Baud with hardware FIFO enabled.
- **Stack Memory Architecture:** All JSON serialization and deserialization uses `StaticJsonDocument<384>` allocated strictly on the stack. **Zero dynamic heap memory (`malloc`/`String` concatenation)** is used inside the core loops, preventing heap fragmentation crashes.
- **Dual Cloud Uplink Pipeline:**
  1. **HiveMQ Dedicated Cloud Broker:** Connects via `WiFiClientSecure` over TLS port 8883. Subscribes to `power_iq_sih2026/commands` to receive instant manual override commands from the Web UI with latency under 50 ms.
  2. **Firebase Realtime Database:** Pushes structured telemetry frames via HTTP REST `PATCH` requests every 3000 ms to the endpoint `https://engineering-project-hub-default-rtdb.firebaseio.com/power_iq/telemetry.json`.
- **SoftAP Hotspot Automatic Fallback:** If local Wi-Fi router fails to connect within 8.0 seconds, the ESP32 automatically initializes an isolated SoftAP network:
  - SSID: `POWER_IQ_GATEWAY`, Password: `poweriq123`
  - Host IP: `192.168.4.1`
  - Allows technicians in off-grid rural areas to connect via smartphone and operate the system without any external Internet router.

---

# 6. In-House AI, Machine Learning & Edge TinyML Pipeline

### 6.1 Why AI is Required (Evaluator Rationale)
A traditional solar tracking algorithm is purely reactive: it turns left if light is higher on the left, and right if higher on the right. **Traditional algorithms cannot detect:**
1. **Partial Soiling / Dust Layer:** When sunlight is intense, but the PV panels are coated in a layer of agricultural dust or soot, suppressing electrical output by $30\% - 50\%$.
2. **Mechanical Jam & Gear Binding:** When wind-blown sand or thermal expansion jams the worm drive. Simple systems will continue pulsing the stepper until the driver burns out or the motor stalls permanently.
3. **Auxiliary Battery Degradation:** When the storage battery voltage sags under deep discharge.
4. **Thermal Overload:** When high ambient temperatures plus motor coil heat push electronics beyond safe silicon thresholds ($> 85^\circ\text{C}$).

POWER IQ’s machine learning layer solves this by fusing **11 physical sensor parameters** simultaneously.

---

### 6.2 Dual-Model Machine Learning Hierarchy

```
+----------------------------------------------------------------------------------------------------+
|                               POWER IQ DUAL-MODEL ML ARCHITECTURE                                 |
+----------------------------------------------------------------------------------------------------+

                          [ 11-Dimensional Live Telemetry Vector ]
                          [ V_pv, I_pv, P_pv, V_bat, LDR_diff,  ]
                          [ LDR_avg, Temp, Hum, Motor_st, I_m,  ]
                          [ Backlash_deg                        ]
                                         |
                +------------------------+------------------------+
                |                                                 |
                v                                                 v
  +---------------------------+                     +---------------------------+
  |          MODEL 1          |                     |          MODEL 2          |
  |   SYSTEM HEALTH & FAULT   |                     |    OPTICAL SUN-TRACKING   |
  |         CLASSIFIER        |                     |         CLASSIFIER        |
  +---------------------------+                     +---------------------------+
  | Algorithm: Random Forest  |                     | Algorithm: Decision Tree  |
  | Estimators: 60 Trees      |                     | Tree Depth: Max Depth 5   |
  | Max Tree Depth: 10        |                     | Test Split Accuracy: 100% |
  | Accuracy: 100.0%          |                     | Purpose: Optical Slew     |
  | Target Classes (6):       |                     | Target Actions (4):       |
  | • NOMINAL                 |                     | • HOLD                    |
  | • SOILED_PANEL            |                     | • STEP_CW                 |
  | • MECHANICAL_JAM          |                     | • STEP_CCW                |
  | • BATTERY_UNDERVOLTAGE    |                     | • PARK_ZERO               |
  | • THERMAL_OVERHEAT        |                     +---------------------------+
  | • NIGHT_HOLD              |                                   |
  +---------------------------+                                   |
                |                                                 |
                v                                                 v
      [ Fault State Diagnosis ]                         [ Kinematic Action ]
                \                                                 /
                 \                                               /
                  v                                             v
        +-------------------------------------------------------------+
        |                 POWER IQ INFERENCE ENGINE                   |
        |   1. System Health Score (0 - 100%)                         |
        |   2. Prediction Confidence Score (%)                        |
        |   3. Natural Language Engineering Insight (100% English)    |
        |   4. Zero External API / 100% Edge Capable (TinyML C++)     |
        +-------------------------------------------------------------+
```

---

### 6.3 Monitored Features & Calibrated Hardware Baseline
The model was trained on **1,500 labeled frames** (`power_iq_telemetry_dataset.csv`), calibrated directly from physical test-bench measurements on the user's solar rig:
- Physical Solar Panel: $20\text{ W}$ rated polycrystalline module.
- Measured Peak Daylight Generation: $10.62\text{ V}$, $1.54\text{ A}$, yielding $\mathbf{16.33\text{ W}}$ peak output.
- Nominal Stepper Motor Drive Current: $1.58\text{ A}$ (stepping) / $0.22\text{ A}$ (unloaded idle).

#### Monitored Features Table:
| Index | Feature Key | Unit | Normal Operating Window | Anomaly Boundary | Physical Phenomenon |
| :---: | :--- | :---: | :---: | :---: | :--- |
| 1 | `solar_voltage_v` | V | $9.5 - 11.2\text{ V}$ | $< 6.0\text{ V}$ | Photovoltaic bus drop / cell disconnection |
| 2 | `solar_current_a` | A | $0.2 - 1.8\text{ A}$ | $< 0.1\text{ A}$ | Low irradiance or disconnected load |
| 3 | `solar_power_w` | W | $2.0 - 16.5\text{ W}$ | $< 7.0\text{ W}$ (in bright sun) | Panel surface dusting or soiling layer |
| 4 | `battery_voltage_v`| V | $11.8 - 12.8\text{ V}$| $< 10.5\text{ V}$ | Deep battery discharge hazard |
| 5 | `ldr_diff` | counts | $-25 \text{ to } +25$ | $> |25|$ | Angular misalignment from solar normal |
| 6 | `ldr_avg_brightness`| counts | $3800 - 4050$ | $< 500$ | Nightfall / complete overcast |
| 7 | `temperature_c` | $^\circ\text{C}$ | $20.0 - 40.0^\circ\text{C}$ | $> 48.0^\circ\text{C}$ | Driver / enclosure thermal throttling |
| 8 | `humidity_pct` | $\%$ | $30.0 - 75.0\%$ | $> 90.0\%$ | Condensation / moisture ingress |
| 9 | `motor_status` | bit | $0$ (Sleep) / $1$ (Move)| Mismatch with Current | Commanded state validation |
| 10 | `motor_current_a` | A | $0.22\text{ A} / 1.58\text{ A}$| $> 2.05\text{ A}$ | Mechanical stall or worm gear binding |
| 11 | `worm_backlash_deg`| $^\circ$ | $< 0.04^\circ$ | $> 0.08^\circ$ | Gear tooth flank wear |

---

### 6.4 Classification States & Health Score Formula

$$\text{Health Score} = 100 - \sum \text{Penalty Factors}$$

1. **`NOMINAL` (Health: 100%):**  
   - All parameters within baseline. Confidence: $\ge 95.0\%$.
   - *Action:* `HOLD`.
   - *AI Insight:* "System operating at peak efficiency (Yield: 16.3W). All kinematics within +/-35 deg limit."
2. **`SOILED_PANEL` (Health: 70%):**  
   - Trigger: Daylight is intense (`ldr_avg_brightness > 3800`), angle is near zenith ($|\theta| < 20^\circ$), but $P_{\text{solar}} < 7.0\text{ W}$ (suppressed by $\ge 40\%$).
   - *Action:* `HOLD` + Maintenance Alert.
   - *AI Insight:* "Solar yield suppressed by ~40% despite bright sun. Panel cleaning advised to recover lost power."
3. **`MECHANICAL_JAM` (Health: 52%):**  
   - Trigger: Motor current surges to $> 2.05\text{ A}$ (nominal stepping $1.58\text{ A}$).
   - *Action:* Instant firmware halt (`PIN_ENABLE = HIGH`).
   - *AI Insight:* "CRITICAL: Elevated motor current (2.35A) indicates mechanical friction or worm gear binding!"
4. **`THERMAL_OVERHEAT` (Health: 64%):**  
   - Trigger: Ambient or heatsink temperature $> 48.0^\circ\text{C}$.
   - *Action:* Throttle stepping speed, hold coils de-energized.
   - *AI Insight:* "Thermal warning: ambient/driver temperature exceeds 48C. Motor holding to cool coils."
5. **`BATTERY_UNDERVOLTAGE` (Health: 72%):**  
   - Trigger: Battery terminal voltage $< 10.5\text{ V}$.
   - *Action:* Inhibit non-essential jog operations, preserve reserve power.
   - *AI Insight:* "Battery bank voltage low (10.2V). Auxiliary charging active to prevent deep cell discharge."
6. **`NIGHT_HOLD` (Health: 98%):**  
   - Trigger: `ldr_avg_brightness < 500`.
   - *Action:* Slew to $0.0^\circ$ Home datum, cut motor power.
   - *AI Insight:* "Darkness / night mode detected. Slats parked safely at 0.0 deg ZERO datum."

---

### 6.5 Edge TinyML C++ Header Transpilation (`power_iq_edge_tinyml.h`)
The trained Decision Tree classifier is transpiled into pure C++ logic during model training. It can be compiled directly into the STM32 or ESP32 firmware without requiring any Python runtime, TensorFlow Lite, or external libraries:

```cpp
#include "power_iq_edge_tinyml.h"

// Sample invocation within real-time loop:
AiInferenceResult diag = evaluateEdgeAi(
    solarVoltage, solarCurrent, solarPower,
    battVoltage, ldrDiff, ldrAvgBrightness,
    currentTemp, motorCurrent, currentAngle
);

// Deterministic response in < 5 microseconds:
if (diag.healthState == STATE_MECHANICAL_JAM) {
    digitalWrite(PIN_ENABLE, HIGH); // Cut coil power instantly
    Serial.println(diag.diagnosticMessage);
}
```

#### Why TinyML C++ Beats Deep Learning on Microcontrollers:
1. **Zero Dynamic Allocation:** Uses no `new` or `malloc`. Completely immune to heap exhaustion.
2. **Sub-Microsecond Latency:** Executes in $< 5\,\mu\text{s}$ on a 72 MHz ARM Cortex-M3 (deep neural networks take $15 - 80\text{ ms}$).
3. **Flash Footprint:** Takes $< 600\text{ bytes}$ of Flash ROM, compared to $> 300\text{ KB}$ for TensorFlow Lite Micro.
4. **100% Explainability:** Every path can be visually audited on a whiteboard during evaluation. Zero hallucination risk.

---

# 7. IoT Cloud Gateway & Remote Dashboard

### 7.1 Web Dashboard Architecture
The user interface is built with **React 18, TypeScript, and Vite**, styled using Tailwind CSS and Lucide icons, and deployed globally on **Firebase Hosting**:
- **Live URL:** [https://power-iq-solar-2026-9e48c.web.app](https://power-iq-solar-2026-9e48c.web.app)

### 7.2 Core Capabilities
1. **Live 3D/Kinematic Slat Angle Gauge:** Visualizes real-time slat tilt between $-35.0^\circ$ and $+35.0^\circ$.
2. **Solar Electrical Yield Telemetry:** Displays instantaneous Voltage (V), Current (A), Power (W), and cumulative Energy (Wh).
3. **Predictive AI Diagnostic Panel:** Displays current System Status (`NOMINAL`, `SOILED_PANEL`, `MECHANICAL_JAM`, etc.), Health Score gauge ($0 - 100\%$), prediction confidence, and English technical diagnosis.
4. **Remote Jog & Control:** Allows operators to trigger `AUTO` tracking, `MANUAL` jog slider, `HOME` zero recalibration, and `ZERO_CURR` sensor calibration from any smartphone or laptop over MQTT / WebSockets.
5. **No-Cache Deployment Configuration:** Configured in `firebase.json` with `Cache-Control: no-cache, no-store, must-revalidate` on all HTML assets, ensuring that evaluators always see the newest live production build without stale browser cache issues.

---

# 8. Comprehensive Evaluator Viva Voce Q&A Defense Manual

Below are the **42 most critical questions** academic evaluators, university professors, and industrial hackathon judges ask, along with the exact technical answers to deliver.

---

### Category A: Project Concept, Novelty & Architecture

#### Q1. What is the core problem with existing solar tracking systems?
> **Answer:** Conventional single-axis solar trackers dedicate an individual linear actuator or slewing motor to each row of solar panels. In a commercial array, this multiplies capital cost, creates dozens of electrical failure points, consumes up to $10\%$ of generated power in motor holding current, and flat panels act as sails causing wind damage.

#### Q2. What is the fundamental novelty of POWER IQ?
> **Answer:** POWER IQ introduces a **multi-shaft gang-drive architecture** where a single high-torque NEMA 17 stepper motor simultaneously drives **8 parallel solar cell shafts** through a central 19:1 self-locking worm gearbox. This slashes actuator cost and structural weight by $87.5\%$. Furthermore, the worm drive's self-locking mechanism enables **$0\text{ W}$ parasitic holding power**, while our in-house Edge TinyML pipeline provides autonomous predictive health diagnostics.

#### Q3. Why did you choose a dual-microcontroller architecture instead of using just one ESP32?
> **Answer:** To guarantee **hard real-time determinism**. The STM32 ARM Cortex-M3 is dedicated entirely to step pulse generation ($1500\,\mu\text{s}$ interval), analog sensor oversampling, and hardware limit enforcement with zero jitter. The ESP32 handles non-deterministic network workloads: Wi-Fi beacon scanning, TLS 1.3 cryptographic handshakes on MQTT port 8883, HTTP REST PATCH requests to Firebase, and the local web server. Decoupling ensures that even during a Wi-Fi drop or cloud reconnection, solar tracking continues smoothly without missing a single step.

#### Q4. What are the practical industrial applications of this system?
> **Answer:** 
> 1. **Commercial & Industrial Rooftop Solar:** Where roof weight limits prohibit heavy commercial slewing motors.
> 2. **Agrivoltaics (Dual-Use Farming):** Parallel rotating slats allow controlled sunlight penetration for crops below while harvesting power.
> 3. **Rural & Off-Grid Microgrids:** Where zero parasitic standby power and standalone SoftAP offline operation are critical.
> 4. **Solar Carports & Canopies:** Gang-driven slatted solar shading for parking lots.

---

### Category B: Mechanical Engineering & Kinematics

#### Q5. What is your actuator, and what is its step angle?
> **Answer:** A NEMA 17 bipolar hybrid stepper motor (Model 17HS4401) with a step angle of $1.8^\circ$ per full step, which translates to $200\text{ full steps per revolution}$.

#### Q6. What is the kinematic formula for steps per degree? Derive it mathematically.
> **Answer:**
> $$\text{Steps per Degree} = \frac{\text{Motor Steps/Rev} \times \text{Gear Ratio}}{360^\circ}$$
> Substituting our hardware values:
> $$\text{Steps per Degree} = \frac{200 \times 19}{360^\circ} = \frac{3800}{360} = \mathbf{10.5556\text{ steps/deg}}$$
> Therefore, rotating the solar slats by $1.0^\circ$ requires exactly $10.556\text{ full steps}$.

#### Q7. What are the mechanical angular limits of your system, and why?
> **Answer:** Strictly clamped in firmware between **$-35.0^\circ$ (East sunrise)** and **$+35.0^\circ$ (West sunset)**, spanning a total travel range of $70.0^\circ$ ($\approx 739\text{ steps}$). This range captures over $94\%$ of daily harvestable direct-normal solar irradiance while preventing mechanical binding, linkage collision, and excessive wire fatigue.

#### Q8. What does "mechanical self-locking" mean, and why is it important in solar trackers?
> **Answer:** A worm drive is self-locking when the lead angle $\gamma$ of the worm screw is smaller than the friction angle $\phi$ between the screw and wheel ($\tan \gamma \le \mu_s$). Torque can be transmitted from the worm screw to the wheel, but torque applied to the solar slats (e.g., wind load) cannot back-drive the screw. This allows us to de-energize the motor coils (`PIN_ENABLE = HIGH`) when stationary, yielding **$0.0\text{ W}$ parasitic holding power**.

#### Q9. How fast can your system track or slew across its full range?
> **Answer:** Our step pulse interval is $1500\,\mu\text{s}$ high and $1500\,\mu\text{s}$ low ($3.0\text{ ms}$ total period), giving $333.3\text{ steps/second}$. At $10.556\text{ steps/degree}$, this equals an angular slew speed of **$31.58^\circ/\text{second}$**. The full $70.0^\circ$ travel stroke takes only **$2.22\text{ seconds}$**, enabling rapid stow during sudden storms or cloud transients.

#### Q10. How do you calibrate the mechanical zero position?
> **Answer:** Through an automated homing routine using an onboard Hall-effect magnetic sensor on pin `PB11`. Upon receiving a `HOME` command or during cold boot, the STM32 slews counter-clockwise until the magnet on the central shaft trips the sensor (Active-LOW interrupt), at which point current position is calibrated to $0.0^\circ$ datum.

---

### Category C: Electronics, Circuits & Power Distribution

#### Q11. Explain your power distribution and voltage rails.
> **Answer:** We use three isolated voltage rails:
> 1. **12V High-Current Rail:** Powers the NEMA 17 stepper motor through the A4988 driver `VMOT` pin, backed by a $100\,\mu\text{F}$ capacitor.
> 2. **5V Rail:** Stepped down from 12V via an LM2596 DC-DC buck converter to power the ESP32 `VIN`, ACS712 current sensor, and I2C LCD display.
> 3. **3.3V Logic Rail:** Generated by onboard low-dropout regulators on the STM32 and ESP32 boards for all micro-controller logic, ADC inputs, and UART lines. All grounds are tied to a single star ground.

#### Q12. Why is there a $100\,\mu\text{F}$ capacitor across the A4988 VMOT and GND pins?
> **Answer:** Stepper motors are highly inductive loads. Rapid PWM current switching produces inductive voltage spikes ($V = -L \frac{di}{dt}$) that can easily exceed the A4988's 35V maximum rating and destroy the internal MOSFETs. The $100\,\mu\text{F}$ low-ESR electrolytic capacitor absorbs these spikes and buffers instantaneous current surges.

#### Q13. How does the A4988 stepper driver work, and how did you configure it?
> **Answer:** The A4988 is a microstepping bipolar motor driver with built-in translator. We interface it using three pins:
> - `STEP` (PB8): Each rising edge advances the motor by one step.
> - `DIR` (PB9): Controls rotation direction (HIGH = CW, LOW = CCW).
> - `ENABLE` (PB10): Active-LOW logic; setting it HIGH disables the output FETs for zero holding power.
> We set the current limit potentiometer ($V_{\text{ref}}$) to match our motor's rated phase current: $I_{\text{max}} = \frac{V_{\text{ref}}}{8 \times R_{\text{sense}}}$.

#### Q14. What happens if the common ground wire between STM32 and ESP32 becomes disconnected?
> **Answer:** Floating reference voltage occurs. UART logic signals ($3.3\text{ V}$) will have no common zero-volt reference, resulting in framing errors, corrupted baud synchronization, and garbage JSON characters. A dedicated low-impedance ground connection is mandatory.

---

### Category D: Sensors & Signal Conditioning

#### Q15. How does your 4-quadrant LDR sun tracking sensor work?
> **Answer:** We arrange four CdS photoresistors in a 4-quadrant array separated by an optical shadow-casting cross. Sensors PA0 and PA1 form the Top sector; PA4 and PA5 form the Bottom sector. When the sun is perpendicular, all four sensors receive equal illumination. When the sun shifts, the shadow falls on one sector, creating a differential voltage:
> $$\Delta L = \text{Average}(\text{Top}) - \text{Average}(\text{Bottom})$$
> If $\Delta L > +25$, the sun is to the West (step CW); if $\Delta L < -25$, to the East (step CCW).

#### Q16. Why did you implement a deadband in the LDR tracking code? What is its value?
> **Answer:** We implemented a **$\pm 25$ ADC counts deadband** ($\approx \pm 0.5^\circ$ angular window). Solar energy follows a cosine curve where power at $\pm 1^\circ$ is $99.98\%$ of peak. Without a deadband, atmospheric scintillation and ADC thermal noise would cause the motor to hunt continuously back and forth, wasting energy and wearing out gear teeth. The deadband keeps the motor off for $> 85\%$ of the day.

#### Q17. How do you measure solar voltage, and why can't you connect it directly to the STM32?
> **Answer:** The solar panel produces up to $18.0\text{ V}$, but STM32 ADC pins have an absolute maximum rating of $3.3\text{ V}$ (or $4.0\text{ V}$ absolute max). Connecting it directly would burn out the internal ADC ESD protection diodes. We use a precision resistive voltage divider ($R_1 = 33\,\text{k}\Omega, R_2 = 6.8\,\text{k}\Omega$):
> $$\text{Divider Ratio} = \frac{33 + 6.8}{6.8} = 5.8529$$
> This safely scales an $18.0\text{ V}$ solar bus down to $3.07\text{ V}$ on the STM32 ADC.

#### Q18. How does the ACS712-05B current sensor work, and how is it calibrated?
> **Answer:** It uses the Hall effect. Solar current flows through an internal low-resistance copper conduction path ($1.2\,\text{m}\Omega$), generating a proportional magnetic field that an integrated Hall IC converts to an analog voltage. At $0.0\text{ A}$, the output is $\frac{V_{cc}}{2} \approx 2.50\text{ V}$ with a sensitivity of $185\text{ mV/A}$. At boot, the STM32 executes an auto-zero routine averaging 64 idle samples to calibrate the zero offset and applies a $3.5\text{ mV}$ deadband to ensure a stable $0.00\text{ A}$ idle readout.

---

### Category E: Embedded Firmware & Real-Time Systems

#### Q19. How does the STM32 communicate with the ESP32?
> **Answer:** Via hardware UART (`USART2` on STM32 pins PA2/PA3 to ESP32 pins GPIO 16/17) running at **115,200 Baud (8-N-1)**. The STM32 transmits single-line structured JSON packets containing all 11 sensor channels, while the ESP32 can send reverse ASCII commands (`AUTO`, `MANUAL`, `HOME`, `GOTO <angle>`).

#### Q20. Why do you use non-blocking `millis()` timing instead of `delay()`?
> **Answer:** `delay()` halts CPU execution in a busy-wait loop, preventing the microcontroller from reading serial command packets, monitoring limit switches, or reacting to emergency motor overcurrent events. Using timestamp comparison with `millis()` allows the main loop to execute at $> 20,000$ iterations per second, ensuring responsive multi-tasking.

#### Q21. How did you optimize the STM32 firmware to fit in 32KB Flash ROM?
> **Answer:** The STM32F103C6 has strictly **32,768 bytes of Flash ROM**. We achieved a final compiled binary of **32,668 bytes (99.7% utilization)** by:
> 1. Eliminating heavy floating-point `printf` runtime libraries in favor of lightweight fixed-point integer conversions.
> 2. Disabling unnecessary C++ runtime type information (RTTI) and exception overhead.
> 3. Refactoring sensor calibration routines into lean inline math functions.

#### Q22. How is dynamic memory managed on the ESP32 to prevent crashes?
> **Answer:** We use ArduinoJson `StaticJsonDocument<384>` allocated strictly on the execution stack. We avoid dynamic heap allocation (`malloc`, `free`, dynamic `String` concatenation) within recurring loops, completely eliminating heap memory fragmentation and guaranteeing 24/7 continuous uptime.

---

### Category F: AI, Machine Learning, TinyML & Data Science

#### Q23. Why did you choose Random Forest and Decision Trees instead of Deep Learning (Neural Networks)?
> **Answer:** 
> 1. **Tabular Sensor Data:** Physical sensor vectors (V, I, W, temp, counts) are tabular. Peer-reviewed literature proves that tree-based ensembles (Random Forest, Gradient Boosting) outperform deep neural networks on tabular datasets without overfitting.
> 2. **Edge TinyML Constraints:** A neural network requires megabytes of memory and floating-point matrix multiplication runtimes (TensorFlow Lite Micro). The STM32F103 has only **32 KB Flash** and **10 KB RAM**. Our Decision Tree compiles into a pure C++ header executing in **$< 5\,\mu\text{s}$** with **0 bytes heap allocation**.
> 3. **100% Explainability:** Critical energy infrastructure requires auditable safety decisions. Every branch of a decision tree is deterministic and transparent. Neural networks are "black boxes" prone to hallucinations.

#### Q24. How many samples are in your dataset, and how were they obtained?
> **Answer:** The dataset (`power_iq_telemetry_dataset.csv`) contains **1,500 multi-parameter records**. The baseline parameters are calibrated directly from our physical test bench: a $20\text{ W}$ solar panel generating $10.62\text{ V}$, $1.54\text{ A}$, and $16.33\text{ W}$ at solar peak, with motor current measuring $1.58\text{ A}$ stepping and $0.22\text{ A}$ idle. Synthetic fault injection was applied across physical boundaries (mechanical jam, dust layer, thermal spike, undervoltage) to train the models across all edge operating conditions.

#### Q25. What are the two models in your pipeline, and what are their hyperparameters?
> **Answer:**
> - **Model 1 (System Health & Fault Diagnostics):** `RandomForestClassifier` with $60$ estimators (trees), `max_depth=10`, trained on 11 diagnostic features. Achieved **$100.0\%$ test accuracy** across 6 operational states.
> - **Model 2 (Optical Sun Tracking):** `DecisionTreeClassifier` with `max_depth=5`, trained on 3 optical features (`ldr_diff`, `ldr_avg_brightness`, `shaft_angle_deg`). Achieved **$100.0\%$ test accuracy** across 4 tracking actions (`HOLD`, `STEP_CW`, `STEP_CCW`, `PARK_ZERO`).

#### Q26. What are the 6 diagnostic states your AI detects?
> **Answer:**
> 1. `NOMINAL` (100% Health): Optimal operation, peak generation ($16.3\text{ W}$).
> 2. `SOILED_PANEL` (70% Health): High daylight ($>3800$ counts) but power suppressed by $\ge 40\%$ due to dust/soiling.
> 3. `MECHANICAL_JAM` (52% Health): Motor current surges $> 2.05\text{ A}$ due to worm gear friction or mechanical stall.
> 4. `BATTERY_UNDERVOLTAGE` (72% Health): Battery voltage $< 10.5\text{ V}$.
> 5. `THERMAL_OVERHEAT` (64% Health): Ambient or driver temperature $> 48.0^\circ\text{C}$.
> 6. `NIGHT_HOLD` (98% Health): Ambient light $< 500$ counts; safely parks at $0.0^\circ$.

#### Q27. How does the AI detect panel dusting/soiling without an optical camera?
> **Answer:** Through multi-sensor mathematical correlation. The model compares ambient solar irradiance (`ldr_avg_brightness > 3800`) against instantaneous power generation ($P_{\text{solar}} = V \times I$). If the sun is intense and the slat angle is near zenith ($< 20^\circ$), but power output is under $7.0\text{ W}$ (normal peak $16.3\text{ W}$), the AI infers a soiling layer and alerts the operator that cleaning is required.

#### Q28. How does the AI protect against mechanical motor burnout?
> **Answer:** The A4988 driver and NEMA 17 motor draw a nominal current of $1.58\text{ A}$ during active stepping. If a foreign object blocks the slats or the worm gear binds, motor current spikes above $2.05\text{ A}$. The AI model classifies this as `MECHANICAL_JAM` with $100\%$ confidence, prompting the controller to assert `PIN_ENABLE = HIGH` immediately, cutting coil power before thermal burnout can occur.

#### Q29. How does Edge TinyML execute on the microcontroller?
> **Answer:** Our script `train_custom_ai.py` exports the trained Decision Tree into a standalone C++ header file (`power_iq_edge_tinyml.h`). It contains the `evaluateEdgeAi()` function implemented in pure nested branch statements (`if-else`). It requires zero external libraries, uses no dynamic memory, and executes in **under $5\text{ microseconds}$** on an STM32 ARM Cortex-M3 running at 72 MHz.

---

### Category G: IoT, Cloud & Cybersecurity

#### Q30. Which cloud protocols are you using, and what are their port numbers?
> **Answer:**
> 1. **MQTT (Message Queuing Telemetry Transport):** Hosted on HiveMQ Dedicated Cloud over **TLS Port 8883** for real-time bidirectional remote control and slider jogging.
> 2. **HTTP REST PATCH:** Connected to Google Firebase Realtime Database over **HTTPS Port 443** for persistent time-series telemetry storage and web dashboard sync.

#### Q31. What happens if the internet router shuts down during field operation?
> **Answer:** The system features **SoftAP Autonomous Hotspot Fallback**. If the ESP32 cannot connect to the Wi-Fi router within 8 seconds, it automatically starts its own Wi-Fi network (`POWER_IQ_GATEWAY` at `192.168.4.1`). An engineer can stand next to the solar array in the field, connect with a smartphone, open `http://192.168.4.1`, and access the full control dashboard with zero internet connection. Furthermore, physical sun tracking on the STM32 continues uninterrupted regardless of network status.

#### Q32. How do you prevent web browsers from caching old dashboard versions?
> **Answer:** We configured `firebase.json` with explicit HTTP response headers for all HTML files:
> `Cache-Control: no-cache, no-store, must-revalidate`.
> When evaluators visit the live Firebase Hosting URL, their browser is forced to download the latest production bundle immediately.

---

# Summary Scorecard for Judges & Evaluators

| Engineering Dimension | Industry Benchmark (Single-Axis) | POWER IQ Solution | Evaluator Advantage |
| :--- | :---: | :---: | :--- |
| **Actuator Count (8 Rows)** | 8 Actuators | **1 NEMA 17 Stepper** | **87.5% Cost & Weight Reduction** |
| **Holding Power Consumption**| $5 - 15\text{ W}$ continuous holding | **0.0 W (Self-Locking Worm)** | **100% Parasitic Holding Elimination** |
| **Motion Controller Architecture**| Single shared MCU (Jitter prone)| **Dual-MCU (STM32 + ESP32)** | **Hard Real-Time Determinism** |
| **Fault Diagnostics** | None (Blind motor actuation) | **Random Forest & Decision Tree** | **Predictive Maintenance & Jam Protection** |
| **Edge AI Execution Latency** | $> 50\text{ ms}$ (Cloud / Neural Net) | **$< 5\,\mu\text{s}$ (TinyML C++ Header)**| **Instantaneous Bare-Metal Edge Safety** |
| **Off-Grid Commissioning** | Cloud dependent only | **SoftAP Hotspot (192.168.4.1)** | **Zero-Infrastructure Field Setup** |
| **Live Web Monitoring** | Proprietary SCADA / Offline | **Firebase Web App + MQTT** | **Global Real-Time Dashboard** |

---
*Document prepared and verified for Academic & Competition Evaluation.*
