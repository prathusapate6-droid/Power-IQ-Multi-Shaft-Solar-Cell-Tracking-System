# In-House AI & Machine Learning Architecture: POWER IQ

**Project:** Low-Power Multi-Shaft Solar Cell Tracking System  
**Lead Engineer:** Prathamesh Sapate  
**Team Members:** Shreyash Pachade, Vansh Dobhale, Prachi Ronge  
**AI System Type:** Custom In-House Supervised Learning & Predictive Maintenance Pipeline  
**Deployment Modes:** Dual-Tier (Python Real-Time Diagnostics Engine + Bare-Metal Embedded C++ TinyML)  
**Cloud Dependency:** **Zero** — 100% In-House, Offline, Self-Contained, and Hallucination-Free  

---

## 1. Directory Structure

```
ai/
├── data/
│   └── power_iq_telemetry_dataset.csv   # 1,500 labeled multi-parameter telemetry frames
├── data_builder/
│   └── build_training_dataset.py        # Dataset generator calibrated to hardware telemetry
├── models/
│   ├── power_iq_health_model.pkl        # Trained Random Forest classifier (System Health & Faults)
│   ├── power_iq_tracking_model.pkl      # Trained Decision Tree classifier (Sun Tracking Actions)
│   ├── power_iq_model_metadata.json     # Feature weights, accuracy metrics, and threshold bounds
│   └── power_iq_edge_tinyml.h           # Embedded C++ Edge header (runs directly on STM32 / ESP32)
├── power_iq_ai_inference.py             # Production live inference engine with natural language insight
├── train_custom_ai.py                   # Automated ML training, validation, and C++ export pipeline
└── README.md                            # Comprehensive AI technical documentation & viva guide
```

---

## 2. Machine Learning Architecture Overview

The **POWER IQ** AI pipeline employs a dual-model supervised learning architecture designed specifically for embedded solar arrays:

```
               [ Live Microcontroller Telemetry Stream ]
               (Solar V/I/W, Batt V, LDR Diff, Temp, Motor I, Angle)
                                  |
            +---------------------+---------------------+
            |                                           |
            v                                           v
+-------------------------------+       +-------------------------------+
|            MODEL 1            |       |            MODEL 2            |
|     SYSTEM HEALTH & FAULT     |       |      OPTICAL SUN TRACKING     |
|          CLASSIFIER           |       |          CLASSIFIER           |
+-------------------------------+       +-------------------------------+
| Algorithm : Random Forest     |       | Algorithm : Decision Tree     |
| Ensemble  : 60 Decision Trees |       | Structure : Max Depth 5       |
| Accuracy  : 100.0% Test Split |       | Accuracy  : 100.0% Test Split |
| Features  : 11 Vectors        |       | Features  : 3 Vectors         |
+-------------------------------+       +-------------------------------+
            |                                           |
            v                                           v
[ Diagnosed System State:       ]       [ Recommended Control Action:   ]
- NOMINAL                       ]       - HOLD                          ]
- SOILED_PANEL                  ]       - STEP_CW                       ]
- MECHANICAL_JAM                ]       - STEP_CCW                      ]
- BATTERY_UNDERVOLTAGE          ]       - PARK_ZERO                     ]
- THERMAL_OVERHEAT              ]
- NIGHT_HOLD                    ]
            \                                           /
             \                                         /
              v                                       v
      +-------------------------------------------------------+
      |             POWER IQ INFERENCE ENGINE                 |
      |   - Dynamic System Health Score (0 - 100%)            |
      |   - Prediction Confidence Score (%)                   |
      |   - Natural Language Diagnostic Insight (100% English)|
      +-------------------------------------------------------+
```

---

## 3. Monitored Telemetry Feature Vectors

The predictive pipeline continuously evaluates 11 physical sensor parameters:

| Feature Name | Unit | Physical Interpretation | Nominal Range | Critical Anomaly Threshold |
| :--- | :---: | :--- | :---: | :---: |
| `solar_voltage_v` | V | Photovoltaic bus voltage | $9.5 - 11.2\text{ V}$ | $< 6.0\text{ V}$ during daylight (Cell open-circuit fault) |
| `solar_current_a` | A | Active solar output current | $0.2 - 1.8\text{ A}$ | $< 0.1\text{ A}$ under high irradiance (Contact disconnection) |
| `solar_power_w` | W | Calculated power output ($V \times I$) | $2.0 - 16.5\text{ W}$ | $< 7.0\text{ W}$ at peak zenith (Panel soiling / dust layer) |
| `battery_voltage_v`| V | 12V lead-acid / Li-ion storage bus | $11.8 - 12.8\text{ V}$ | $< 10.5\text{ V}$ (Critical undervoltage discharge risk) |
| `ldr_diff` | counts| Differential light (Top minus Bottom) | $-25 \text{ to } +25$ | $> +25$ (Step CW) or $< -25$ (Step CCW) |
| `ldr_avg_brightness`| counts| Ambient daylight intensity (12-bit ADC)| $3800 - 4050$ | $< 500$ (Nightfall / safe park state) |
| `temperature_c` | $^\circ\text{C}$ | Ambient and driver heatsink temperature| $20.0 - 40.0^\circ\text{C}$| $> 48.0^\circ\text{C}$ (Thermal overload warning) |
| `humidity_pct` | $\%$ | Relative ambient humidity | $30.0 - 75.0\%$ | $> 90.0\%$ (Dew condensation / moisture ingress risk) |
| `motor_status` | bit | Stepper activity state | 0 (Sleep), 1 (Moving)| State validation against current draw |
| `motor_current_a` | A | Stepper coil drive current | $0.22\text{ A}$ (Idle) / $1.58\text{ A}$ (Move) | $> 2.05\text{ A}$ (Mechanical stall / worm binding) |
| `worm_backlash_deg`| $^\circ$ | Kinematic gear tooth flank play | $< 0.04^\circ$ | $> 0.08^\circ$ (Excessive transmission wear) |

---

## 4. Evaluated Fault States & Autonomous Actions

1. **`NOMINAL` (Health: 100%):**  
   All electrical, thermal, and kinematic parameters are within normal boundaries. System maintains optimal cosine sun alignment.
2. **`SOILED_PANEL` (Health: 70%):**  
   Irradiance is bright (`ldr_avg_brightness > 3800`), yet solar power generation is suppressed by $\ge 40\%$ compared to the theoretical baseline at the current angle. Signals that dust or dirt accumulation requires cleaning.
3. **`MECHANICAL_JAM` (Health: 52%):**  
   Motor current surges above $2.05\text{ A}$ (nominal $1.58\text{ A}$). Firmware halts stepping immediately to prevent A4988 driver thermal burnout or stripped worm gear teeth.
4. **`THERMAL_OVERHEAT` (Health: 64%):**  
   Enclosure or driver temperature exceeds $48.0^\circ\text{C}$. Slew movements are throttled to allow coil cooling.
5. **`BATTERY_UNDERVOLTAGE` (Health: 72%):**  
   Battery voltage drops below $10.5\text{ V}$. Secondary actuator movements are suspended to preserve reserve power.
6. **`NIGHT_HOLD` (Health: 98%):**  
   Ambient light drops below 500 counts. System automatically parks slats at $0.0^\circ$ zenith to resist overnight wind gusts.

---

## 5. Why Random Forest & Decision Trees? (Viva / Evaluator Defense)

Judges often ask: *"Why did you use Random Forest instead of Deep Learning or Neural Networks?"*

Here are the four key engineering justifications:

1. **Tabular Sensor Data Optimization:**  
   Tabular sensor telemetry with correlated continuous and categorical features is mathematically best modeled by gradient boosted or decision tree ensembles. Deep Neural Networks suffer from overfitting on tabular data of this scale.
2. **Deterministic, Microsecond Edge Execution:**  
   A deep neural network requires floating-point matrix multiplication libraries (TensorFlow Lite Micro, ONNX) which require megabytes of RAM and Flash memory. The STM32F103C6 has only **32 KB Flash** and **10 KB RAM**. A trained Decision Tree can be transpiled into pure C++ `if-else` branches executing in **$< 5\,\mu\text{s}$** with **zero dynamic memory allocation**.
3. **100% Explainability (No "Black Box"):**  
   In industrial and energy systems, safety standards require that every control decision be auditable. Every decision branch in our Decision Tree model is visible, verifiable, and cannot hallucinate.
4. **Resilience to Sensor Noise:**  
   The 60-tree ensemble voting in our Random Forest averages out electromagnetic noise generated by stepper motor PWM switching and solar inverter ripple.

---

## 6. How to Run Training & Inference

### 1. Re-Train Models from Dataset
```bash
python3 ai/train_custom_ai.py
```
*Outputs accuracy reports, feature importances, `.pkl` binary model files, and updates `ai/models/power_iq_edge_tinyml.h`.*

### 2. Run In-House AI Inference Demonstration
```bash
python3 ai/power_iq_ai_inference.py
```

### Sample Output:
```text
=================================================================
POWER IQ — Custom In-House AI Inference Demonstration
=================================================================

[Test 1] Real Solar Peak Telemetry (User's Exact Hardware Test Frame):
Status: NOMINAL (Confidence: 95.0%) | Health Score: 100%
Action: HOLD
AI Insight: System operating at peak efficiency (Yield: 16.3W). All kinematics within +/-35 deg limit.

[Test 2] High Friction Anomaly (Motor Current Spike to 2.35A):
Status: MECHANICAL_JAM (Confidence: 100.0%) | Health Score: 52%
Action: STEP_CW
AI Insight: CRITICAL: Elevated motor current (2.35A) indicates mechanical friction or worm gear binding!

[Test 3] Dusty / Soiled Panel Anomaly (Bright Sun but 4.5W output):
Status: SOILED_PANEL (Confidence: 100.0%) | Health Score: 70%
Action: HOLD
AI Insight: Solar yield suppressed by ~40% despite bright sun. Panel cleaning advised to recover lost power.
=================================================================
```

---

## 7. Edge TinyML C++ Header Usage (`power_iq_edge_tinyml.h`)

For direct on-device execution on microcontrollers without Python:

```cpp
#include "power_iq_edge_tinyml.h"

// Sample invocation within STM32 or ESP32 loop:
AiInferenceResult diag = evaluateEdgeAi(
    solarVoltage, solarCurrent, solarPower,
    battVoltage, ldrDiff, ldrAvg,
    currentTemp, motorCurrent, currentAngle
);

if (diag.healthState == STATE_MECHANICAL_JAM) {
    digitalWrite(PIN_ENABLE, HIGH); // Cut motor power instantly
    Serial.println(diag.diagnosticMessage);
}
```
