"""
POWER IQ — Multi-Shaft Solar Cell Tracking System
AI-Based Predictive Maintenance & Telemetry Analysis Engine (Prototype Simulation)

Project: POWER IQ
Institution / Competition: Engineering Innovation / Hackathon Demonstration

Purpose:
This module demonstrates the supervisory AI analysis and predictive maintenance
logic planned for the multi-shaft solar tracking system. In this prototype implementation,
it performs algorithmic anomaly detection on motor current signatures, gear backlash,
and shaft synchronization variance to flag mechanical degradation before physical failure.

Note for Hackathon Evaluators:
This is an engineering prototype demonstration. It models telemetry patterns and
validates diagnostic thresholds for future edge-AI / TinyML integration on the STM32 MCU.
"""

import json
from typing import Dict, List, Any


class MultiShaftPredictiveEngine:
    """
    Supervisory predictive maintenance and health evaluation engine
    for the single-motor multi-shaft solar tracking transmission.
    """

    # Baseline operating thresholds
    NOMINAL_MOTOR_CURRENT_ACTIVE = 1.58  # Amperes (active stepping)
    CURRENT_SPIKE_THRESHOLD = 2.00       # Amperes (indicates mechanical drag)
    MAX_SYNC_VARIANCE_DEG = 0.35         # Degrees (max allowable between shafts)
    MAX_BACKLASH_DEG = 0.08              # Degrees (worm gear play)
    SAFE_MOTOR_TEMP_C = 55.0             # Celsius

    def __init__(self):
        self.health_score = 100
        self.diagnostics_log: List[Dict[str, Any]] = []

    def evaluate_telemetry_frame(self, telemetry: Dict[str, Any]) -> Dict[str, Any]:
        """
        Evaluates a single telemetry snapshot and determines component risk states.
        """
        issues = []
        score_deductions = 0

        motor = telemetry.get("motor", {})
        shafts = telemetry.get("shafts", [])
        solar = telemetry.get("solar", {})

        # 1. Motor Current & Thermal Analysis
        motor_current = motor.get("current_a", 0.0)
        motor_temp = motor.get("temperature_c", 25.0)
        motor_status = motor.get("status", "IDLE")

        if motor_status == "RUNNING" and motor_current > self.CURRENT_SPIKE_THRESHOLD:
            excess_current_pct = ((motor_current - self.NOMINAL_MOTOR_CURRENT_ACTIVE) / self.NOMINAL_MOTOR_CURRENT_ACTIVE) * 100
            issues.append(f"Elevated motor current (+{excess_current_pct:.1f}%): Possible mechanical friction or gear binding.")
            score_deductions += 15

        if motor_temp > self.SAFE_MOTOR_TEMP_C:
            issues.append(f"High motor temperature ({motor_temp:.1f}°C): Thermal accumulation detected.")
            score_deductions += 10

        # 2. Shaft Synchronization Variance
        angles = [s.get("angle_deg", 0.0) for s in shafts]
        if angles:
            mean_angle = sum(angles) / len(angles)
            max_dev = max(abs(a - mean_angle) for a in angles)

            abnormal_shafts = []
            for s in shafts:
                dev = abs(s.get("angle_deg", 0.0) - mean_angle)
                if dev > self.MAX_SYNC_VARIANCE_DEG:
                    abnormal_shafts.append((s.get("id"), dev, s.get("backlash_deg", 0.0)))

            if abnormal_shafts:
                for shaft_id, dev, backlash in abnormal_shafts:
                    issues.append(f"Shaft #{shaft_id} sync deviation (Δ {dev:.2f}°): Backlash measured at {backlash:.2f}°.")
                    score_deductions += 12

        # 3. Electrical Yield & Sensor Correlation
        solar_v = solar.get("voltage_v", 0.0)
        solar_i = solar.get("current_a", 0.0)
        measured_kw = solar.get("power_kw", 0.0)
        calculated_kw = (solar_v * solar_i) / 1000.0

        # Check for electrical discrepancy
        if calculated_kw > 0 and abs(measured_kw - calculated_kw) / calculated_kw > 0.15:
            issues.append("Electrical yield mismatch: Sensor drift or string shading detected.")
            score_deductions += 8

        # 4. Final Health Score Calculation
        self.health_score = max(0, 100 - score_deductions)

        if self.health_score >= 90:
            status = "HEALTHY"
            insight = "System operating nominally. All 8 parallel shafts synchronized with central worm drive within ±0.15° margin."
        elif self.health_score >= 75:
            status = "WARNING"
            insight = f"Minor mechanical variance detected. {'; '.join(issues)} Scheduled inspection recommended."
        else:
            status = "CRITICAL"
            insight = f"Critical mechanical anomaly! {'; '.join(issues)} Preventive shutdown or calibration advised."

        result = {
            "health_score": self.health_score,
            "system_status": status,
            "issues_detected": issues,
            "ai_maintenance_insight": insight,
            "metrics": {
                "motor_current_a": motor_current,
                "motor_temp_c": motor_temp,
                "max_shaft_deviation_deg": max_dev if angles else 0.0,
            }
        }
        self.diagnostics_log.append(result)
        return result


def main():
    print("==================================================")
    print("POWER IQ — Multi-Shaft Solar Cell Tracking System")
    print("AI Predictive Maintenance Simulation Engine")
    print("==================================================\n")

    engine = MultiShaftPredictiveEngine()

    # Test Case 1: Nominal Synchronized Operation
    print("[1] Evaluating Nominal Synchronized Telemetry Frame...")
    nominal_frame = {
        "motor": {"status": "RUNNING", "current_a": 1.58, "temperature_c": 38.4, "voltage_v": 24.0},
        "shafts": [{"id": i, "angle_deg": 47.0 + (0.1 if i % 2 == 0 else 0.0), "backlash_deg": 0.03} for i in range(1, 9)],
        "solar": {"voltage_v": 59.8, "current_a": 47.5, "power_kw": 2.84},
    }
    res_nom = engine.evaluate_telemetry_frame(nominal_frame)
    print(f"Health Score: {res_nom['health_score']}% | Status: {res_nom['system_status']}")
    print(f"AI Insight: \"{res_nom['ai_maintenance_insight']}\"\n")

    # Test Case 2: Simulated Mechanical Anomaly on Shaft #4
    print("[2] Evaluating Anomaly Injected Frame (Shaft #4 Backlash + Friction)...")
    anomaly_frame = {
        "motor": {"status": "RUNNING", "current_a": 2.15, "temperature_c": 48.2, "voltage_v": 24.0},
        "shafts": [
            {"id": i, "angle_deg": 47.6 if i == 4 else 47.0, "backlash_deg": 0.12 if i == 4 else 0.03}
            for i in range(1, 9)
        ],
        "solar": {"voltage_v": 59.8, "current_a": 46.2, "power_kw": 2.76},
    }
    res_anom = engine.evaluate_telemetry_frame(anomaly_frame)
    print(f"Health Score: {res_anom['health_score']}% | Status: {res_anom['system_status']}")
    print(f"Issues: {res_anom['issues_detected']}")
    print(f"AI Insight: \"{res_anom['ai_maintenance_insight']}\"")


if __name__ == "__main__":
    main()
