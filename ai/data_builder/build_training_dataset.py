#!/usr/bin/env python3
"""
POWER IQ — Dataset Builder
Generates a structured, labeled telemetry dataset based on the physical
measurements of the Multi-Shaft Solar Cell Tracking System.

Sources & Baseline Calibration:
  - Solar Panel Peak: ~16.3 W (10.62V @ 1.54A) under direct noon sun
  - Travel Range: -35.0 deg to +35.0 deg (0.0 deg Zero Datum)
  - LDR Readings: 0-4095 scale (Active daylight ~3950-4030, Balanced diff +/-15, Tilted diff >25)
  - Stepper Motor Current: 0.22A idle, 1.58A stepping, >2.0A jammed/friction
  - Battery Bank: 12V nominal (11.8V-12.6V healthy, <10.8V undervoltage)
  - Ambient Conditions: 25-42 deg C, 35-70% humidity
"""

import csv
import math
import random
import os

OUTPUT_CSV = os.path.join(os.path.dirname(__file__), "..", "data", "power_iq_telemetry_dataset.csv")

def generate_dataset(num_samples=1500, random_seed=42):
    random.seed(random_seed)
    os.makedirs(os.path.dirname(OUTPUT_CSV), exist_ok=True)

    headers = [
        "timestamp_hour",
        "sun_elevation_deg",
        "target_angle_deg",
        "shaft_angle_deg",
        "angle_error_deg",
        "ldr_top_val",
        "ldr_bot_val",
        "ldr_diff",
        "ldr_avg_brightness",
        "solar_voltage_v",
        "solar_current_a",
        "solar_power_w",
        "battery_voltage_v",
        "temperature_c",
        "humidity_pct",
        "motor_status",        # 0=IDLE, 1=STEPPING
        "motor_current_a",
        "worm_backlash_deg",
        "system_status",       # NOMINAL, SOILED_PANEL, MECHANICAL_JAM, BATTERY_UNDERVOLTAGE, THERMAL_OVERHEAT, NIGHT_HOLD
        "tracking_action",     # HOLD, STEP_CW, STEP_CCW, PARK_ZERO
        "health_score"         # 0 - 100 integer
    ]

    rows = []

    # Category distribution:
    # 60% Nominal daylight tracking
    # 10% Soiled / Dusty panel anomaly
    # 10% Mechanical friction / binding anomaly
    # 8%  Battery undervoltage anomaly
    # 4%  Thermal overheat anomaly
    # 8%  Night / Darkness mode

    for i in range(num_samples):
        # Time of day between 6:00 (6.0) and 18:00 (18.0)
        scenario = random.random()

        if scenario < 0.08:
            # 1. NIGHT / DARKNESS (early morning or late evening)
            hour = random.choice([5.5, 18.5, 19.0, 20.0, 4.5])
            sun_elev = 0.0
            target_angle = 0.0
            shaft_angle = round(random.uniform(-1.0, 1.0), 1)
            angle_err = abs(shaft_angle - target_angle)
            ldr_top = random.randint(80, 450)
            ldr_bot = random.randint(80, 450)
            ldr_diff = ldr_top - ldr_bot
            ldr_avg = (ldr_top + ldr_bot) // 2
            solar_v = round(random.uniform(0.0, 2.5), 2)
            solar_i = 0.0
            solar_w = 0.0
            batt_v = round(random.uniform(11.5, 12.4), 2)
            temp_c = round(random.uniform(22.0, 28.0), 1)
            hum_pct = round(random.uniform(55.0, 75.0), 1)
            motor_status = 0
            motor_i = round(random.uniform(0.18, 0.24), 2)
            backlash = round(random.uniform(0.02, 0.04), 3)
            status = "NIGHT_HOLD"
            action = "PARK_ZERO"
            health = 98

        elif scenario < 0.18:
            # 2. SOILED / DUSTY PANEL ANOMALY
            # Sunlight is strong, but solar power is 30-50% lower than expected
            hour = random.uniform(9.0, 15.0)
            day_fraction = (hour - 6.0) / 12.0
            sun_elev = round(math.sin(day_fraction * math.pi) * 75.0, 1)
            target_angle = round((day_fraction - 0.5) * 70.0, 1)  # -35 to +35
            target_angle = max(-35.0, min(35.0, target_angle))
            shaft_angle = target_angle
            angle_err = 0.0
            ldr_top = random.randint(3800, 4020)
            ldr_bot = random.randint(3800, 4020)
            ldr_diff = random.randint(-12, 12)
            ldr_avg = (ldr_top + ldr_bot) // 2
            solar_v = round(random.uniform(9.2, 10.3), 2)
            # Power severely reduced due to dirt/dust
            solar_i = round(random.uniform(0.35, 0.70), 2)
            solar_w = round(solar_v * solar_i, 2)
            batt_v = round(random.uniform(11.4, 12.2), 2)
            temp_c = round(random.uniform(32.0, 40.0), 1)
            hum_pct = round(random.uniform(40.0, 55.0), 1)
            motor_status = 0
            motor_i = round(random.uniform(0.20, 0.25), 2)
            backlash = round(random.uniform(0.02, 0.04), 3)
            status = "SOILED_PANEL"
            action = "HOLD"
            health = random.randint(62, 75)

        elif scenario < 0.28:
            # 3. MECHANICAL FRICTION / BINDING ANOMALY
            # Motor current spikes during motion, backlash high
            hour = random.uniform(8.0, 16.0)
            day_fraction = (hour - 6.0) / 12.0
            sun_elev = round(math.sin(day_fraction * math.pi) * 75.0, 1)
            target_angle = round((day_fraction - 0.5) * 70.0, 1)
            target_angle = max(-35.0, min(35.0, target_angle))
            shaft_angle = round(target_angle + random.uniform(-3.5, 3.5), 1)
            shaft_angle = max(-35.0, min(35.0, shaft_angle))
            angle_err = round(abs(shaft_angle - target_angle), 1)
            ldr_diff = random.choice([random.randint(28, 65), random.randint(-65, -28)])
            ldr_top = random.randint(3700, 4000)
            ldr_bot = ldr_top - ldr_diff
            ldr_avg = (ldr_top + ldr_bot) // 2
            solar_v = round(random.uniform(9.8, 10.6), 2)
            solar_i = round(random.uniform(0.9, 1.4), 2)
            solar_w = round(solar_v * solar_i, 2)
            batt_v = round(random.uniform(11.2, 12.0), 2)
            temp_c = round(random.uniform(36.0, 44.0), 1)
            hum_pct = round(random.uniform(42.0, 52.0), 1)
            motor_status = 1
            motor_i = round(random.uniform(2.10, 2.65), 2)  # High current!
            backlash = round(random.uniform(0.08, 0.15), 3)  # High gear play!
            status = "MECHANICAL_JAM"
            action = "STEP_CW" if ldr_diff > 0 else "STEP_CCW"
            health = random.randint(45, 60)

        elif scenario < 0.36:
            # 4. BATTERY UNDERVOLTAGE ANOMALY
            # Battery drops below 10.5V (e.g. user's 9.44V deep discharge)
            hour = random.uniform(7.0, 17.0)
            day_fraction = (hour - 6.0) / 12.0
            sun_elev = round(math.sin(day_fraction * math.pi) * 75.0, 1)
            target_angle = round((day_fraction - 0.5) * 70.0, 1)
            target_angle = max(-35.0, min(35.0, target_angle))
            shaft_angle = target_angle
            angle_err = 0.0
            ldr_top = random.randint(3800, 4020)
            ldr_bot = random.randint(3800, 4020)
            ldr_diff = random.randint(-15, 15)
            ldr_avg = (ldr_top + ldr_bot) // 2
            solar_v = round(random.uniform(10.2, 10.7), 2)
            solar_i = round(random.uniform(0.5, 1.5), 2)
            solar_w = round(solar_v * solar_i, 2)
            batt_v = round(random.uniform(8.8, 9.8), 2)     # Severely low!
            temp_c = round(random.uniform(30.0, 38.0), 1)
            hum_pct = round(random.uniform(45.0, 60.0), 1)
            motor_status = 0
            motor_i = round(random.uniform(0.18, 0.24), 2)
            backlash = round(random.uniform(0.02, 0.04), 3)
            status = "BATTERY_UNDERVOLTAGE"
            action = "HOLD"
            health = random.randint(65, 78)

        elif scenario < 0.40:
            # 5. THERMAL OVERHEAT ANOMALY
            # Ambient/driver temp > 48C
            hour = random.uniform(12.0, 15.0)
            day_fraction = (hour - 6.0) / 12.0
            sun_elev = round(math.sin(day_fraction * math.pi) * 75.0, 1)
            target_angle = round((day_fraction - 0.5) * 70.0, 1)
            shaft_angle = target_angle
            angle_err = 0.0
            ldr_top = random.randint(3950, 4030)
            ldr_bot = random.randint(3950, 4030)
            ldr_diff = random.randint(-10, 10)
            ldr_avg = (ldr_top + ldr_bot) // 2
            solar_v = round(random.uniform(10.1, 10.6), 2)
            solar_i = round(random.uniform(1.2, 1.5), 2)
            solar_w = round(solar_v * solar_i, 2)
            batt_v = round(random.uniform(11.8, 12.4), 2)
            temp_c = round(random.uniform(49.0, 56.0), 1)  # Overheat!
            hum_pct = round(random.uniform(28.0, 38.0), 1)
            motor_status = 0
            motor_i = round(random.uniform(0.20, 0.26), 2)
            backlash = round(random.uniform(0.03, 0.05), 3)
            status = "THERMAL_OVERHEAT"
            action = "HOLD"
            health = random.randint(55, 68)

        else:
            # 6. NOMINAL DAYLIGHT TRACKING (60% of data)
            # Directly calibrated to user's real hardware test logs
            hour = random.uniform(6.5, 17.5)
            day_fraction = (hour - 6.0) / 12.0
            sun_elev = round(math.sin(day_fraction * math.pi) * 75.0, 1)
            target_angle = round((day_fraction - 0.5) * 70.0, 1)
            target_angle = max(-35.0, min(35.0, target_angle))

            # Simulate sub-cases:
            # 70% balanced tracking, 30% stepping towards sun
            sub_case = random.random()
            if sub_case < 0.70:
                # Balanced at current sun angle
                shaft_angle = target_angle
                angle_err = round(random.uniform(0.0, 0.3), 1)
                ldr_diff = random.randint(-14, 14)  # within deadband
                ldr_top = random.randint(3940, 4030)
                ldr_bot = ldr_top - ldr_diff
                motor_status = 0
                motor_i = round(random.uniform(0.18, 0.24), 2)
                action = "HOLD"
            elif sub_case < 0.85:
                # Tilted CCW, sun has moved Eastward / slat lags
                shaft_angle = max(-35.0, round(target_angle - random.uniform(1.5, 4.0), 1))
                angle_err = round(abs(shaft_angle - target_angle), 1)
                ldr_diff = random.randint(26, 85)   # Top brighter -> CW
                ldr_top = random.randint(3950, 4030)
                ldr_bot = ldr_top - ldr_diff
                motor_status = 1
                motor_i = round(random.uniform(1.48, 1.62), 2)
                action = "STEP_CW"
            else:
                # Tilted CW, sun has moved Westward / slat leads
                shaft_angle = min(35.0, round(target_angle + random.uniform(1.5, 4.0), 1))
                angle_err = round(abs(shaft_angle - target_angle), 1)
                ldr_diff = random.randint(-85, -26)  # Bot brighter -> CCW
                ldr_bot = random.randint(3950, 4030)
                ldr_top = ldr_bot + ldr_diff
                motor_status = 1
                motor_i = round(random.uniform(1.48, 1.62), 2)
                action = "STEP_CCW"

            ldr_avg = (ldr_top + ldr_bot) // 2
            # Cosine efficiency drop based on tilt misalignment
            cos_factor = max(0.4, math.cos(math.radians(angle_err)))
            sun_sin = max(0.1, math.sin(day_fraction * math.pi))
            
            # Real 20W panel peak yield: ~16.3W @ 10.62V / 1.54A
            solar_v = round(10.62 + random.uniform(-0.15, 0.15), 2)
            solar_i = round((1.54 * sun_sin * cos_factor) + random.uniform(-0.04, 0.04), 2)
            solar_i = max(0.05, min(1.58, solar_i))
            solar_w = round(solar_v * solar_i, 2)
            batt_v = round(12.2 + (0.4 * sun_sin) + random.uniform(-0.1, 0.1), 2)
            temp_c = round(32.0 + (6.0 * sun_sin) + random.uniform(-1.0, 1.0), 1)
            hum_pct = round(52.0 - (10.0 * sun_sin) + random.uniform(-2.0, 2.0), 1)
            backlash = round(random.uniform(0.02, 0.035), 3)
            status = "NOMINAL"
            health = random.randint(95, 100)

        rows.append([
            round(hour, 2),
            sun_elev,
            target_angle,
            shaft_angle,
            angle_err,
            ldr_top,
            ldr_bot,
            ldr_diff,
            ldr_avg,
            solar_v,
            solar_i,
            solar_w,
            batt_v,
            temp_c,
            hum_pct,
            motor_status,
            motor_i,
            backlash,
            status,
            action,
            health
        ])

    with open(OUTPUT_CSV, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(headers)
        writer.writerows(rows)

    print(f"✅ Generated {len(rows)} structured telemetry samples.")
    print(f"📁 Dataset written to: {OUTPUT_CSV}")
    return OUTPUT_CSV

if __name__ == "__main__":
    generate_dataset()
