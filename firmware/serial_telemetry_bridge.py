#!/usr/bin/env python3
"""
==============================================================================
POWER IQ — Live Serial Telemetry Bridge & Predictive Maintenance Monitor
Target: Bridges Arduino UNO hardware over USB Serial to Python AI / Dashboard
Team: Prathamesh Sapate (Lead), Shreyash Pachade, Vansh Dobhale, Prachi Ronge
==============================================================================

Usage:
  # Normal operation (auto-detects Arduino port):
  python3 serial_telemetry_bridge.py

  # Specific port:
  python3 serial_telemetry_bridge.py --port /dev/cu.usbmodem14101 --baud 115200

  # Simulation / Mock Mode (runs without physical hardware connected):
  python3 serial_telemetry_bridge.py --mock
"""

import sys
import os
import time
import json
import argparse
from typing import Optional

# Import AI Predictive Engine from ai/prototype_analysis
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "ai", "prototype_analysis")))
try:
    from predictive_maintenance_simulation import MultiShaftPredictiveEngine
except ImportError:
    MultiShaftPredictiveEngine = None


def find_arduino_port() -> Optional[str]:
    """Auto-detects active USB-Serial port on macOS / Linux / Windows."""
    try:
        import serial.tools.list_ports
        ports = serial.tools.list_ports.comports()
        for p in ports:
            desc = p.description.lower()
            hwid = p.hwid.lower()
            if any(k in desc or k in hwid for k in ["arduino", "ch340", "cp210", "ftdi", "usb serial", "usbmodem"]):
                return p.device
        if ports:
            return ports[0].device
    except ImportError:
        pass
    return None


class TelemetryBridge:
    def __init__(self, port: Optional[str] = None, baud: int = 115200, mock: bool = False):
        self.port = port
        self.baud = baud
        self.mock = mock
        self.serial_conn = None
        self.ai_engine = MultiShaftPredictiveEngine() if MultiShaftPredictiveEngine else None

    def connect(self) -> bool:
        if self.mock:
            print("[INFO] Running in MOCK SIMULATION mode (no physical hardware required).")
            return True

        if not self.port:
            self.port = find_arduino_port()

        if not self.port:
            print("[WARN] No Arduino USB serial port detected.")
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
            angle = round(45.0 * (0.5 + 0.5 * (sim_time % 60) / 60.0), 1)
            frame = {
                "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
                "angle": angle,
                "target": 45.0,
                "moving": False,
                "steps_per_deg": 16.67,
                "driver": "L298N (Simulated)",
                "motor": {"status": "IDLE", "current_a": 0.05, "temperature_c": 32.4, "voltage_v": 12.0},
                "shafts": [{"id": i, "angle_deg": angle, "backlash_deg": 0.02} for i in range(1, 11)],
                "solar": {"voltage_v": 18.2, "current_a": 2.4, "power_kw": 0.043},
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

                # Run AI predictive analysis if applicable
                ai_result = None
                if self.ai_engine and "shafts" in frame and "motor" in frame:
                    ai_result = self.ai_engine.evaluate_telemetry_frame(frame)

                # Format terminal display
                angle = frame.get("angle", "N/A")
                moving = frame.get("moving", False)
                health = ai_result.get("health_score", 100) if ai_result else 100
                status = ai_result.get("system_status", "HEALTHY") if ai_result else "NOMINAL"

                print(f"[{time.strftime('%H:%M:%S')}] Slat Angle: {angle}° | Motion: {'MOVING' if moving else 'LOCKED'} | Health: {health}% [{status}]")
                if ai_result and ai_result.get("issues_detected"):
                    print(f"    ⚠️ Diagnostics: {ai_result['issues_detected']}")

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
