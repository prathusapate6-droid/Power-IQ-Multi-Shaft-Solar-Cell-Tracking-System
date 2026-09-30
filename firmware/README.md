# Embedded Firmware Architecture: POWER IQ Dual-MCU Controller

**Project:** Low-Power Multi-Shaft Solar Cell Tracking System (POWER IQ)  
**Lead Engineer:** Prathamesh Sapate  
**Team Members:** Shreyash Pachade, Vansh Dobhale, Prachi Ronge  
**Platform Architecture:** Dual-Microcontroller Distributed Embedded System  
- **Primary Real-Time Motion Controller:** STM32F103C8T6 / C6T6 "Blue Pill" (32-bit ARM Cortex-M3 @ 72 MHz)
- **IoT Cloud Gateway & Display Controller:** ESP32 DevKit V1 (Dual-Core 32-bit Xtensa LX6 @ 240 MHz)
- **Host Diagnostics & AI Bridge:** Python 3 USB-UART Telemetry Bridge with In-House Random Forest / Decision Tree Inference

---

## 1. Directory Structure

```
firmware/
├── stm32_ldr_sun_tracking/
│   └── stm32_ldr_sun_tracking.ino    # Real-time kinematic controller, LDR sun tracking, ACS712 & telemetry
├── esp32_iot_gateway/
│   └── esp32_iot_gateway.ino         # Dual-MCU UART bridge, Firebase RTDB, HiveMQ MQTT (TLS), I2C LCD
├── serial_telemetry_bridge.py        # Python USB-UART bridge linking live hardware to in-house AI engine
└── README.md                         # Comprehensive firmware documentation & viva explanation guide
```

---

## 2. Distributed Dual-MCU Architecture

```
   +--------------------------------------------------------------------------+
   |                        POWER IQ SYSTEM TOPOLOGY                          |
   +--------------------------------------------------------------------------+
                                       |
    [ 4-Quadrant LDR Array ]           |         [ I2C 16x2 LCD Display (0x27) ]
    [ Hall Effect Home Switch ]        |         [ Onboard Status LED (GPIO 2)  ]
    [ ACS712 Solar Current ]           |                        ^
    [ 33k/6.8k Voltage Divider ]       |                        |
    [ Manual 10k Potentiometer ]       |                        |
    [ 2-Pos Mode Switch ]              |                        |
               |                       |                        |
               v                       |                        v
   +------------------------+  UART2 (115200)   +-----------------------------+
   |   STM32F103 BluePill   | <==============> |      ESP32 IoT Gateway      |
   |   (ARM Cortex-M3)      |  PA2(TX)/PA3(RX) |  (Xtensa Dual-Core 240MHz)  |
   +------------------------+  GPIO16/GPIO17   +-----------------------------+
         |            |                                |             |
         v            v                                v             v
    [ A4988 ]    [ USART1 TX/RX ]                 [ WiFi STA / ]  [ HiveMQ MQTT ]
    [ Driver ]   (PA9/PA10 @ 115200)              [ SoftAP Hotspot] (TLS 8883)   ]
         |            |                                |             |
         v            v                                v             v
    [ NEMA 17 ]  [ Host PC Python Bridge ]       [ Firebase ]  [ Web Dashboard ]
    [ 19:1 Worm ] (serial_telemetry_bridge.py)   [ Cloud RTDB] [ Control UI    ]
    [ 8 Shafts ]      |
                      v
             [ In-House ML AI ]
             (Random Forest & TinyML)
```

### Why Dual-MCU? (Common Evaluator Question)
- **Deterministic Hard Real-Time Execution:** The STM32 ARM Cortex-M3 is dedicated entirely to micro-stepping pulse generation, sub-millisecond ADC sampling, and hardware limit enforcement. It has zero network latency, zero RTOS jitter, and cannot be interrupted by Wi-Fi handshakes or TLS cryptographic computations.
- **Network & Cryptographic Isolation:** The ESP32 handles the memory-intensive TCP/IP stack, TLS 1.3 handshakes for MQTT (HiveMQ Cloud port 8883), HTTP REST PATCH requests to Firebase, and HTTP serving for the local dashboard. Even if Wi-Fi disconnects or reconnects, solar tracking on the STM32 remains 100% uninterrupted.

---

## 3. Mathematical Kinematics & Mechanical Derivation

### Stepper Motor and Worm Transmission
- **Actuator:** NEMA 17 Stepper Motor ($1.8^\circ$ step angle $\rightarrow$ $200\text{ full steps / revolution}$).
- **Gear Reduction:** Central high-precision $19:1$ worm gearbox driving a synchronized rack or linkage that rotates 8 parallel solar cell shafts.
- **Kinematic Formula:**
  $$\text{Steps per Degree} = \frac{\text{Motor Steps per Rev} \times \text{Gear Ratio}}{360^\circ} = \frac{200 \times 19}{360} = \frac{3800}{360} \approx 10.556\text{ steps/deg}$$
- **Angular Travel Range:** Strictly clamped in firmware to $[-35.0^\circ, +35.0^\circ]$ ($70.0^\circ$ total mechanical travel):
  $$\text{Total Travel Steps} = 70.0^\circ \times 10.5556\text{ steps/deg} \approx 739\text{ full steps}$$
- **Zero-Power Mechanical Holding (Self-Locking):**
  Worm gear transmissions exhibit inherent mechanical self-locking when the lead angle is less than the friction angle of the mating materials ($\mu \ge \tan \gamma$). Consequently, wind loads applied to the solar slats cannot back-drive the stepper motor. When the target angle is reached, the firmware asserts `PIN_ENABLE = HIGH`, cutting coil current to $0.0\text{ A}$ (0W holding power), drastically reducing parasitic standby consumption.

---

## 4. Hardware Pin Mapping & Electrical Connections

### STM32F103 Blue Pill (Motion & Sensor Controller)
| Component | Function | STM32 Pin | Logic Level | Electrical Notes |
| :--- | :--- | :---: | :---: | :--- |
| **A4988 Driver** | STEP (Pulse) | **PB8** | 3.3V CMOS | $1500\,\mu\text{s}$ pulse interval for smooth, high-torque slew |
| | DIR (Direction) | **PB9** | 3.3V CMOS | HIGH = Clockwise (West), LOW = Counter-Clockwise (East) |
| | ENABLE | **PB10** | 3.3V CMOS | Active-LOW (LOW = Motor energized, HIGH = Sleep/0W) |
| **Homing Sensor** | Hall Effect Switch | **PB11** | 3.3V Input | Active-LOW with internal pull-up resistor; triggers at calibrated $0.0^\circ$ |
| **4-Quadrant LDRs** | Top Sector 1 | **PA0** | 12-bit ADC | Connected via CdS voltage divider ($10\text{k}\Omega$ pull-down) |
| | Top Sector 2 | **PA1** | 12-bit ADC | Connected via CdS voltage divider ($10\text{k}\Omega$ pull-down) |
| | Bottom Sector 1| **PA4** | 12-bit ADC | Connected via CdS voltage divider ($10\text{k}\Omega$ pull-down) |
| | Bottom Sector 2| **PA5** | 12-bit ADC | Connected via CdS voltage divider ($10\text{k}\Omega$ pull-down) |
| **Power Monitors** | Solar Voltage | **PA6** | 12-bit ADC | Divider: $R_1=33\,\text{k}\Omega$, $R_2=6.8\,\text{k}\Omega$ (Divider ratio: $5.853$) |
| | Solar Current | **PA7** | 12-bit ADC | ACS712-05B Hall IC; baseline $V_{cc}/2$, auto-zeroed on boot |
| | Battery Voltage| **PB1** | 12-bit ADC | Monitored through resistor divider |
| **User Controls** | Manual Potentiometer | **PB0** | 12-bit ADC | 10k linear pot mapped to $[-35.0^\circ, +35.0^\circ]$ |
| | Auto/Manual Switch | **PB12 / PB13** | Digital In | Active-LOW switch to select Tracking vs. Manual Jog |
| **Environment** | DHT11 Sensor | **PB5** | Bi-directional | Ambient temperature ($^\circ\text{C}$) and relative humidity ($\%$) |
| **Serial Links** | USART1 (Host PC) | **PA9 (TX) / PA10 (RX)** | 3.3V UART | 115200 Baud; interactive console & Python telemetry bridge |
| | USART2 (ESP32) | **PA2 (TX) / PA3 (RX)** | 3.3V UART | 115200 Baud; structured JSON streaming to ESP32 gateway |
| **Status LED** | Heartbeat Indicator | **PC13** | Digital Out | Active-LOW onboard green LED; toggles every second |

### ESP32 DevKit V1 (IoT Cloud Gateway)
| Component | Function | ESP32 Pin | Electrical Notes |
| :--- | :--- | :---: | :--- |
| **STM32 Link** | UART2 RX2 | **GPIO 16** | Connects to STM32 PA2 (USART2_TX) |
| | UART2 TX2 | **GPIO 17** | Connects to STM32 PA3 (USART2_RX) |
| | Ground | **GND** | **Common Ground with STM32 is mandatory** |
| **I2C LCD Display**| SDA (Data) | **GPIO 21** | I2C address `0x27`, 16x2 character display |
| | SCL (Clock) | **GPIO 22** | Hardware I2C clock line |
| **Status Indicator**| Onboard LED | **GPIO 2** | Blue LED; solid ON when connected to Wi-Fi / MQTT |

---

## 5. Serial Telemetry Protocol (115200 Baud)

The STM32 streams formatted single-line JSON packets to the ESP32 and host PC at regular intervals:

```json
{
  "ang": 12.5,
  "pot": 0.0,
  "mode": "AUTO",
  "homed": 1,
  "v_pv": 10.62,
  "i_pv": 1.54,
  "p_pv": 16.33,
  "v_bat": 12.45,
  "temp": 37.2,
  "hum": 49.0
}
```

### Interactive Serial Commands (USART1 / USART2 / Web UI / MQTT)
- `AUTO` — Switches system into autonomous optical closed-loop sun tracking.
- `MANUAL` / `MAN` — Pauses tracking; allows potentiometer or remote slider positioning.
- `HOME` — Initiates homing sequence: rotates until Hall switch PB11 trips, then resets calibrated datum to $0.0^\circ$.
- `GOTO <angle>` — Drives slats directly to specified angle within $[-35.0^\circ, +35.0^\circ]$ (e.g., `GOTO 20.0` or `GOTO -15.5`).
- `ZERO_CURR` — Re-samples ACS712 zero-current baseline offset.
- `SPEED <us>` — Adjusts step pulse interval in microseconds (default: $1500\,\mu\text{s}$).

---

## 6. How to Build, Compile & Deploy

### Prerequisites
- Install `arduino-cli` with STM32duino and ESP32 board packages:
  ```bash
  # STM32 Core:
  arduino-cli core install stm32duino:STM32F1

  # ESP32 Core:
  arduino-cli core install esp32:esp32
  ```

### Compiling STM32 Firmware
```bash
arduino-cli compile --clean \
  --fqbn "stm32duino:STM32F1:genericSTM32F103C6:upload_method=serialMethod" \
  firmware/stm32_ldr_sun_tracking
```
> [!NOTE]
> The compiled binary size is optimized to fit within the strict 32KB Flash ROM limit of the STM32F103C6 micro-controller (~32,668 bytes utilized).

### Running the Python Live AI Telemetry Bridge
```bash
# Auto-detects connected STM32 USB-UART adapter and runs live ML inference:
python3 firmware/serial_telemetry_bridge.py

# Or run in bench simulation / mock mode (no hardware required):
python3 firmware/serial_telemetry_bridge.py --mock
```
