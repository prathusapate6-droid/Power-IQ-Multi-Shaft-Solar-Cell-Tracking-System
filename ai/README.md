# AI-Based Predictive Maintenance & Telemetry Analysis (Prototype Concept)

## Overview

The **POWER IQ** system incorporates a planned **Supervisory AI & Predictive Maintenance** layer. In the physical system, individual rotating shafts and the central worm drive are subject to environmental stresses, dust, mechanical wear, and wind load. 

Rather than waiting for mechanical breakdown or tracking failure, the AI module continuously monitors electrical and kinematic telemetry to predict maintenance needs and flag abnormal operating states.

> **Hackathon Evaluator Notice:**  
> The AI functionality is currently implemented as an **algorithmic simulation prototype**. No trained deep learning model weights or commercial edge deployment are claimed. The scripts provided validate thresholding logic and demonstrate the intended data ingestion architecture for future TinyML deployment on the STM32 microcontroller.

---

## Architecture & Monitored Parameters

The predictive analysis module processes four telemetry vectors:

1. **Motor Current Signature ($I_{\text{motor}}$):**
   - *Nominal Active Current:* $\sim 1.58\text{ A}$
   - *Threshold:* $> 2.00\text{ A}$
   - *Indication:* Mechanical binding, worm gear misalignment, bearing seizure, or external obstacle blocking PV slat rotation.

2. **Shaft-to-Shaft Synchronization Variance ($\Delta \theta$):**
   - *Nominal Variance:* $\le \pm 0.20^\circ$
   - *Warning Threshold:* $> 0.35^\circ$
   - *Indication:* Mechanical slip, loose coupling, or worn worm-wheel teeth on an individual shaft.

3. **Gear Backlash Estimation:**
   - *Nominal Backlash:* $< 0.04^\circ$
   - *Warning Threshold:* $> 0.08^\circ$
   - *Indication:* Torsional play or tooth flank wear on the 40:1 worm gearing.

4. **Coil Temperature & Thermal Accumulation ($T_{\text{motor}}$):**
   - *Safe Upper Limit:* $< 55.0^\circ\text{C}$
   - *Indication:* Continuous duty cycle overload or driver thermal throttling.

---

## Directory Structure

```
ai/
├── README.md                                  # Architecture and algorithmic documentation
├── prototype_analysis/
│   └── predictive_maintenance_simulation.py  # Python anomaly detector & health score evaluator
└── sample_data/
    └── sample_telemetry_stream.json          # Multi-frame sample telemetry stream
```

---

## Running the Simulation

```bash
# From the project root
python3 ai/prototype_analysis/predictive_maintenance_simulation.py
```

### Sample Output:
```text
==================================================
POWER IQ — Multi-Shaft Solar Cell Tracking System
AI Predictive Maintenance Simulation Engine
==================================================

[1] Evaluating Nominal Synchronized Telemetry Frame...
Health Score: 100% | Status: HEALTHY
AI Insight: "System operating nominally. All 8 parallel shafts synchronized with central worm drive within ±0.15° margin."

[2] Evaluating Anomaly Injected Frame (Shaft #4 Backlash + Friction)...
Health Score: 73% | Status: CRITICAL
Issues: ['Elevated motor current (+36.1%): Possible mechanical friction or gear binding.', 'Shaft #4 sync deviation (Δ 0.52°): Backlash measured at 0.12°.']
AI Insight: "Critical mechanical anomaly! Elevated motor current (+36.1%): Possible mechanical friction or gear binding.; Shaft #4 sync deviation (Δ 0.52°): Backlash measured at 0.12°. Preventive shutdown or calibration advised."
```

---

## Future Roadmap: Edge TinyML Integration

1. **On-Device Vibration FFT:** Sampling an I2C accelerometer on the central worm gear axle to classify mechanical vibration signatures using a lightweight Random Forest or 1D-CNN directly on the STM32 MCU.
2. **Solar Irradiance Forecasting:** Cloud-based LSTM model predicting next-hour generation based on regional solar irradiance satellite feeds and historical local array yield.
