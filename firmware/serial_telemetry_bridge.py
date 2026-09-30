#!/usr/bin/env python3
"""
==============================================================================
POWER IQ — Live Serial Telemetry Bridge & Predictive Maintenance Monitor
Target: Bridges STM32F103 Blue Pill hardware over USB-UART to Python AI & Web Dashboard
Team: Prathamesh Sapate (Lead), Shreyash Pachade, Vansh Dobhale, Prachi Ronge
==============================================================================

Usage:
  # Normal operation (auto-detects STM32 USB-UART port):
  python3 firmware/serial_telemetry_bridge.py

  # Specific port:
  python3 firmware/serial_telemetry_bridge.py --port /dev/cu.usbserial-1410 --baud 115200

  # Simulation / Mock Mode (runs bench evaluation without physical hardware):
  python3 firmware/serial_telemetry_bridge.py --mock
"""

import sys
import os
import time
import json
import math
import argparse
from typing import Optional

# Import In-House AI Inference Engine (Random Forest & Decision Tree)
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "ai")))
try:
    from power_iq_ai_inference import PowerIqCustomAi
except ImportError:
    PowerIqCustomAi = None


def find_stm32_port() -> Optional[str]:
    """Auto-detects active USB-UART bridge port (CH340, CP2102, FTDI, or STM32 VCP)."""
    try:
        import serial.tools.list_ports
        ports = serial.tools.list_ports.comports()
        for p in ports:
            desc = p.description.lower()
            hwid = p.hwid.lower()
            if any(k in desc or k in hwid for k in ["ch340", "cp210", "ftdi", "usb serial", "usbmodem", "stm32"]):
                return p.device
        if ports:
            return ports[0].device
    except ImportError:
        pass
    return None


class TelemetryBridge:
    """
    Serial Telemetry Interface & Machine Learning Diagnostics Bridge.
    Connects to the STM32F103 microcontroller via USART1 (PA9/PA10) at 115200 Baud,
    parses real-time sensor frames, and evaluates system health using the in-house AI model.
    """
    def __init__(self, port: Optional[str] = None, baud: int = 115200, mock: bool = False):
        self.port = port
        self.baud = baud
        self.mock = mock
        self.serial_conn = None
        self.custom_ai = PowerIqCustomAi() if PowerIqCustomAi else None

    def connect(self) -> bool:
        if self.mock:
            print("[INFO] Running in MOCK SIMULATION mode (bench validation without hardware).")
            return True

        if not self.port:
            self.port = find_stm32_port()

        if not self.port:
            print("[WARN] No STM32 USB-UART serial adapter detected on host ports.")
            print("       Switching automatically to --mock simulation mode.")
            self.mock = True
            return True

        try:
            import serial
            self.serial_conn = serial.Serial(self.port, self.baud, timeout=1.5)
            print(f"[CONNECTED] Serial connection established on {self.port} at {self.baud} baud.")
            time.sleep(2.0)  # Wait for Arduino bootloader reset
            return True
        except Exception as e:
            print(f"[ERROR] Could not open serial port {self.port}: {e}")
            print("        Falling back to --mock simulation mode.")
            self.mock = True
            return True

    def query_telemetry(self) -> dict:
        """Polls Arduino for status or generates simulated telemetry."""
        if self.mock or not self.serial_conn:
            sim_time = time.time()
            # Realistic travel between -35.0 and +35.0 deg
            angle = round(35.0 * math.sin((sim_time % 60) * (2 * math.pi / 60)), 1)
            frame = {
                "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
                "angle": angle,
                "target": angle,
                "moving": False,
                "steps_per_deg": 10.556,
                "driver": "A4988 / STM32F103",
                "motor": {"status": "IDLE", "current_a": 0.22, "temperature_c": 36.8, "voltage_v": 24.0},
                "shafts": [{"id": i, "angle_deg": angle, "backlash_deg": 0.025} for i in range(1, 9)],
                "solar": {"voltage_v": 10.62, "current_a": 1.54, "power_w": 16.33, "power_kw": 0.0163},
                "solar_voltage_v": 10.62,
                "solar_current_a": 1.54,
                "solar_power_w": 16.33,
                "battery_voltage_v": 12.25,
                "ldr_diff": -2,
                "ldr_top_val": 4022,
                "ldr_bot_val": 4024,
                "temperature_c": 37.2,
                "humidity_pct": 49.0,
                "shaft_angle_deg": angle,
                "motor_status": 0,
                "motor_current_a": 0.22
            }
            return frame

        try:
            self.serial_conn.reset_input_buffer()
            self.serial_conn.write(b"STATUS JSON\n")
            line = self.serial_conn.readline().decode("utf-8", errors="ignore").strip()

            if line.startswith("{") and line.endswith("}"):
                data = json.loads(line)
                data["timestamp"] = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())
                return data
            else:
                return {"raw_response": line}
        except Exception as e:
            return {"error": str(e)}

    def send_command(self, cmd: str) -> str:
        """Sends an ASCII command to the controller."""
        if self.mock or not self.serial_conn:
            return f"[MOCK-ACK] Commanded '{cmd}' (Simulated)"
        try:
            self.serial_conn.write(f"{cmd.strip()}\n".encode("utf-8"))
            return self.serial_conn.readline().decode("utf-8", errors="ignore").strip()
        except Exception as e:
            return f"[ERROR] {e}"

    def run_monitor(self, interval_sec: float = 1.0):
        print("\n=======================================================")
        print(" POWER IQ — LIVE TELEMETRY & AI HEALTH MONITOR        ")
        print(" Press Ctrl+C to exit.                                 ")
        print("=======================================================\n")

        while True:
            try:
                frame = self.query_telemetry()

                # Run In-House Trained AI Diagnostic Model
                custom_diag = None
                if self.custom_ai:
                    custom_diag = self.custom_ai.diagnose(frame)

                # Format terminal display
                angle = frame.get("angle", "N/A")
                moving = frame.get("moving", False)
                health = custom_diag["health_score"] if custom_diag else 100
                status = custom_diag["system_status"] if custom_diag else "NOMINAL"
                conf = custom_diag.get("confidence_pct", 99.0) if custom_diag else 100.0
                action = custom_diag.get("recommended_action", "HOLD") if custom_diag else "HOLD"

                print(f"[{time.strftime('%H:%M:%S')}] Slat: {angle}° | Motion: {'MOVING' if moving else 'LOCKED'} | Health: {health}% [{status} ({conf}%)] | Action: {action}")
                if custom_diag:
                    print(f"    🤖 AI Insight: {custom_diag['ai_insight']}")

                time.sleep(interval_sec)
            except KeyboardInterrupt:
                print("\n[SHUTDOWN] Exiting Telemetry Bridge.")
                break


def main():
    parser = argparse.ArgumentParser(description="POWER IQ Serial Telemetry Bridge")
    parser.add_argument("--port", type=str, default=None, help="Serial port (e.g. /dev/cu.usbmodem14101)")
    parser.add_argument("--baud", type=int, default=115200, help="Serial baud rate (default: 115200)")
    parser.add_argument("--mock", action="store_true", help="Force mock telemetry simulation")
    parser.add_argument("--interval", type=float, default=1.0, help="Poll interval in seconds (default: 1.0)")
    args = parser.parse_args()

    bridge = TelemetryBridge(port=args.port, baud=args.baud, mock=args.mock)
    bridge.connect()
    bridge.run_monitor(interval_sec=args.interval)


if __name__ == "__main__":
    main()
