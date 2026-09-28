import { useState, useEffect, useCallback, useRef } from 'react';
import type {
  ShaftData,
  MotorTelemetry,
  SolarTelemetry,
  TrackingGeometry,
  AiDiagnostics,
  SystemAlert,
  TrackingMode,
} from '../types/dashboard';
import { calculateSunPosition, generateDiurnalCurve, addJitter } from '../utils/solarMath';
import { useHardwareMqtt } from './useHardwareMqtt';

export function useSolarSimulation() {
  // Base time of day: 14.45 ~ 2:27 PM produces target angle of ~47° and ~2.84 kW output
  const [hourDecimal, setHourDecimal] = useState<number>(14.45);
  const [isPaused, setIsPaused] = useState<boolean>(false);
  const [simSpeed, setSimSpeed] = useState<number>(1); // 1x, 5x, 20x
  const [faultInjected, setFaultInjected] = useState<boolean>(false);
  const [trackingMode, setTrackingMode] = useState<TrackingMode>('AUTO');
  const [isEmergencyStopped, setIsEmergencyStopped] = useState<boolean>(false);

  // Mechanical tracking angle state
  const [actualShaftAngle, setActualShaftAngle] = useState<number>(47.0);
  const [isMotorMoving, setIsMotorMoving] = useState<boolean>(true);
  const [intermittentTimer, setIntermittentTimer] = useState<number>(18); // seconds to next step

  // Live Cloud Bridge via HiveMQ MQTT (STM32 + ESP32 Gateway)
  const {
    isMqttConnected,
    isHardwareOnline,
    telemetry,
    sendCommand,
  } = useHardwareMqtt();

  // Alerts log
  const [alerts, setAlerts] = useState<SystemAlert[]>([
    {
      id: 'al-1',
      time: '14:25:10',
      type: 'success',
      title: 'Shaft Synchronization Verified',
      message: 'All 8 parallel shafts synchronized with central worm drive within ±0.1° tolerance.',
      component: 'WORM_DRIVE',
    },
    {
      id: 'al-2',
      time: '14:26:02',
      type: 'info',
      title: 'Intermittent Step Executed',
      message: 'Micro-step angular correction of +0.4° completed. Stepper motor returned to low-power holding.',
      component: 'STEPPER_MOTOR',
    },
    {
      id: 'al-3',
      time: '14:27:00',
      type: 'info',
      title: 'STM32 LDR Differential Valid',
      message: 'Dual-axis LDR irradiance sensor delta is within 3.2% optical tracking threshold.',
      component: 'STM32_MCU',
    },
  ]);

  // Alert on hardware link state transitions
  const prevOnlineRef = useRef<boolean>(false);
  useEffect(() => {
    if (isHardwareOnline && !prevOnlineRef.current) {
      setAlerts((prev) => [
        {
          id: `al-${Date.now()}`,
          time: new Date().toLocaleTimeString(),
          type: 'success',
          title: 'Physical Hardware Online (MQTT)',
          message: 'Real-time telemetry streaming from STM32 Blue Pill + ESP32 Gateway via HiveMQ Cloud.',
          component: 'STM32_MCU',
        },
        ...prev.slice(0, 7),
      ]);
    } else if (!isHardwareOnline && prevOnlineRef.current) {
      setAlerts((prev) => [
        {
          id: `al-${Date.now()}`,
          time: new Date().toLocaleTimeString(),
          type: 'warning',
          title: 'Hardware Disconnected',
          message: 'Fallback to simulated solar kinematic & physics model.',
          component: 'STM32_MCU',
        },
        ...prev.slice(0, 7),
      ]);
    }
    prevOnlineRef.current = isHardwareOnline;
  }, [isHardwareOnline]);

  // Sun position & Target calculation
  const sunPos = calculateSunPosition(hourDecimal);
  const targetAngle = sunPos.targetTrackingAngle;

  // Real or simulated angles
  const effectiveShaftAngle = (isHardwareOnline && telemetry) ? telemetry.angle : actualShaftAngle;
  const effectiveTrackingMode = (isHardwareOnline && telemetry) 
    ? (telemetry.mode === 'AUTO' ? 'AUTO' : 'MANUAL') 
    : trackingMode;

  // Intermittent tracking kinematics simulation (only active if hardware is offline)
  useEffect(() => {
    if (isPaused || isEmergencyStopped || isHardwareOnline) return;

    const interval = setInterval(() => {
      // Advance simulated time slightly
      setHourDecimal((prev) => {
        const next = prev + 0.0004 * simSpeed;
        return next >= 18.5 ? 6.0 : next;
      });

      // Intermittent adjustment countdown
      setIntermittentTimer((prev) => {
        if (prev <= 1) {
          setIsMotorMoving(true);
          return 25;
        }
        return prev - 1;
      });
    }, 1000);

    return () => clearInterval(interval);
  }, [isPaused, isEmergencyStopped, simSpeed, isHardwareOnline]);

  // Motor movement toward target angle in AUTO mode (simulation)
  useEffect(() => {
    if (isEmergencyStopped || isHardwareOnline) {
      if (isEmergencyStopped) setIsMotorMoving(false);
      return;
    }

    if (trackingMode === 'AUTO') {
      const diff = targetAngle - actualShaftAngle;
      if (Math.abs(diff) > 0.08) {
        setIsMotorMoving(true);
        const step = diff > 0 ? 0.06 : -0.06;
        const timer = setTimeout(() => {
          setActualShaftAngle((curr) => Number((curr + step).toFixed(2)));
        }, 300);
        return () => clearTimeout(timer);
      } else {
        const timer = setTimeout(() => {
          setIsMotorMoving(false);
        }, 1200);
        return () => clearTimeout(timer);
      }
    }
  }, [targetAngle, actualShaftAngle, trackingMode, isEmergencyStopped, isHardwareOnline]);

  // 8 Parallel Shafts Data
  const shafts: ShaftData[] = [
    {
      id: 1,
      name: 'Shaft 1 (West Outer)',
      currentAngle: Number((effectiveShaftAngle + 0.1).toFixed(1)),
      targetAngle: Number(targetAngle.toFixed(1)),
      variance: 0.1,
      status: 'SYNCHRONIZED',
      wormGearBacklash: 0.03,
      cellStringVoltage: 7.48,
      cellStringCurrent: 5.94,
      cellStringPower: 44.4,
    },
    {
      id: 2,
      name: 'Shaft 2 (Row B)',
      currentAngle: Number(effectiveShaftAngle.toFixed(1)),
      targetAngle: Number(targetAngle.toFixed(1)),
      variance: 0.0,
      status: 'SYNCHRONIZED',
      wormGearBacklash: 0.02,
      cellStringVoltage: 7.46,
      cellStringCurrent: 5.93,
      cellStringPower: 44.2,
    },
    {
      id: 3,
      name: 'Shaft 3 (Row C)',
      currentAngle: Number((effectiveShaftAngle + 0.2).toFixed(1)),
      targetAngle: Number(targetAngle.toFixed(1)),
      variance: 0.2,
      status: 'SYNCHRONIZED',
      wormGearBacklash: 0.04,
      cellStringVoltage: 7.45,
      cellStringCurrent: 5.92,
      cellStringPower: 44.1,
    },
    {
      id: 4,
      name: 'Shaft 4 (Central Left)',
      currentAngle: Number(
        (effectiveShaftAngle + (faultInjected ? 0.6 : 0.0)).toFixed(1)
      ),
      targetAngle: Number(targetAngle.toFixed(1)),
      variance: faultInjected ? 0.6 : 0.0,
      status: faultInjected ? 'WARN_DEVIATION' : 'SYNCHRONIZED',
      wormGearBacklash: faultInjected ? 0.12 : 0.03,
      cellStringVoltage: 7.47,
      cellStringCurrent: faultInjected ? 5.21 : 5.95,
      cellStringPower: faultInjected ? 38.9 : 44.4,
    },
    {
      id: 5,
      name: 'Shaft 5 (Central Right)',
      currentAngle: Number((effectiveShaftAngle - 0.1).toFixed(1)),
      targetAngle: Number(targetAngle.toFixed(1)),
      variance: -0.1,
      status: 'SYNCHRONIZED',
      wormGearBacklash: 0.03,
      cellStringVoltage: 7.49,
      cellStringCurrent: 5.96,
      cellStringPower: 44.6,
    },
    {
      id: 6,
      name: 'Shaft 6 (Row F)',
      currentAngle: Number((effectiveShaftAngle + 0.1).toFixed(1)),
      targetAngle: Number(targetAngle.toFixed(1)),
      variance: 0.1,
      status: 'SYNCHRONIZED',
      wormGearBacklash: 0.02,
      cellStringVoltage: 7.46,
      cellStringCurrent: 5.93,
      cellStringPower: 44.2,
    },
    {
      id: 7,
      name: 'Shaft 7 (Row G)',
      currentAngle: Number(effectiveShaftAngle.toFixed(1)),
      targetAngle: Number(targetAngle.toFixed(1)),
      variance: 0.0,
      status: 'SYNCHRONIZED',
      wormGearBacklash: 0.03,
      cellStringVoltage: 7.48,
      cellStringCurrent: 5.94,
      cellStringPower: 44.4,
    },
    {
      id: 8,
      name: 'Shaft 8 (East Outer)',
      currentAngle: Number((effectiveShaftAngle + 0.2).toFixed(1)),
      targetAngle: Number(targetAngle.toFixed(1)),
      variance: 0.2,
      status: 'SYNCHRONIZED',
      wormGearBacklash: 0.04,
      cellStringVoltage: 7.45,
      cellStringCurrent: 5.91,
      cellStringPower: 44.0,
    },
  ];

  // Dynamic values with realistic jitter
  const trackingError = Number(Math.abs(targetAngle - effectiveShaftAngle).toFixed(2));
  const motorVoltage = 24.0;
  const motorCurrent = isEmergencyStopped
    ? 0
    : isMotorMoving
    ? addJitter(faultInjected ? 2.15 : 1.58, 0.04)
    : 0.22;
  const motorPower = Number((motorVoltage * motorCurrent).toFixed(1));

  // Solar power telemetry (proportional to angle alignment)
  const alignmentEfficiency = Math.max(0.7, Math.cos(((trackingError * Math.PI) / 180)));
  const basePowerKw = 2.84 * alignmentEfficiency;
  const simulatedPowerKw = Number(addJitter(basePowerKw, 0.03).toFixed(2));
  const simulatedVoltageV = Number(addJitter(59.8, 0.2).toFixed(1));
  const simulatedCurrentA = Number((simulatedPowerKw > 0 ? (simulatedPowerKw * 1000) / simulatedVoltageV : 0).toFixed(1));

  // Effective values based on live hardware vs simulation
  const solarPowerKw = (isHardwareOnline && telemetry)
    ? Number((telemetry.solar_power / 1000).toFixed(3))
    : simulatedPowerKw;
  const solarVoltageV = (isHardwareOnline && telemetry)
    ? telemetry.solar_voltage
    : simulatedVoltageV;
  const solarCurrentA = (isHardwareOnline && telemetry)
    ? telemetry.solar_current
    : simulatedCurrentA;
  const energyTodayKwh = (isHardwareOnline && telemetry)
    ? Number((telemetry.energy_wh / 1000).toFixed(2))
    : 14.62;

  // Motor telemetry object
  const motor: MotorTelemetry = {
    status: isEmergencyStopped
      ? 'STOPPED'
      : (isHardwareOnline && telemetry && (telemetry.homed === false || telemetry.homed === 0))
      ? 'STEPPING'
      : isMotorMoving
      ? 'RUNNING'
      : 'IDLE',
    rpm: isMotorMoving && !isEmergencyStopped ? 120 : 0,
    voltage: isEmergencyStopped ? 0 : motorVoltage,
    current: motorCurrent,
    power: motorPower,
    temperature: (isHardwareOnline && telemetry && telemetry.temperature > 0)
      ? telemetry.temperature
      : faultInjected ? 48.2 : 38.4,
    direction: targetAngle > effectiveShaftAngle ? 'CW' : targetAngle < effectiveShaftAngle ? 'CCW' : 'HOLD',
    totalSteps: 14820,
    loadFactor: faultInjected ? 78 : isMotorMoving ? 42 : 8,
    wormDriveEngagement: isEmergencyStopped ? 'DISENGAGED' : isMotorMoving ? 'ROTATING' : 'LOCKED',
  };

  // Solar telemetry object
  const solar: SolarTelemetry = {
    powerKw: solarPowerKw,
    voltageV: solarVoltageV,
    currentA: solarCurrentA,
    energyTodayKwh,
    efficiency: 91.4,
    fixedPvBaselineKw: 2.18,
    instantGainPercent: 30.2,
    irradianceWm2: 892,
    battVoltageV: (isHardwareOnline && telemetry) ? telemetry.batt_voltage : undefined,
    isHardwareOnline,
  };

  // Tracking geometry object
  const tracking: TrackingGeometry = {
    sunElevation: sunPos.elevation,
    sunAzimuth: sunPos.azimuth,
    targetAngle: Number(targetAngle.toFixed(1)),
    actualShaftAngle: Number(effectiveShaftAngle.toFixed(1)),
    trackingError,
    intermittentCountdownSec: intermittentTimer,
    isAdjusting: isMotorMoving,
    trackingMode: effectiveTrackingMode,
  };

  // AI Diagnostics state
  const ai: AiDiagnostics = {
    healthScore: faultInjected ? 76 : 96,
    gearBacklashRisk: faultInjected ? 'MODERATE' : 'LOW',
    motorHealth: faultInjected ? 'WARNING' : 'GOOD',
    shaftSynchronization: faultInjected ? 'DEGRADED' : 'GOOD',
    overloadRisk: faultInjected ? ('HIGH' as const) : ('LOW' as const),
    flexibleCableFatigue: 'LOW',
    bearingFriction: faultInjected ? 'ELEVATED' : 'NOMINAL',
    maintenancePrediction: faultInjected ? 'INSPECTION_RECOMMENDED' : 'NORMAL',
    aiInsightText: faultInjected
      ? 'Elevated motor current (+36%) and slight backlash variance (+0.6°) detected on Shaft #4. Pattern indicates probable mechanical drag or bearing misalignment on Shaft #4 worm wheel. Scheduled inspection recommended before thermal accumulation.'
      : 'System operating normally. No abnormal motor load detected. Shaft synchronization across all 8 parallel worm gears is within acceptable mechanical limits (±0.2°). Intermittent tracking is optimizing net energy harvest.',
    faultInjected,
  };

  // Control action simulation & cloud dispatch handlers
  const handleAutoToggle = useCallback(() => {
    const nextMode = trackingMode === 'AUTO' ? 'MANUAL' : 'AUTO';
    setTrackingMode(nextMode);
    if (isHardwareOnline) {
      sendCommand(nextMode);
    }
    setAlerts((prev) => [
      {
        id: `al-${Date.now()}`,
        time: new Date().toLocaleTimeString(),
        type: 'info',
        title: nextMode === 'MANUAL' ? 'Switched to Manual Control' : 'Switched to Auto Tracking',
        message: nextMode === 'MANUAL'
          ? (isHardwareOnline ? 'Sent MANUAL command to physical STM32 via HiveMQ Cloud.' : 'Automatic sun tracking suspended. Awaiting manual jog commands.')
          : (isHardwareOnline ? 'Sent AUTO command to physical STM32 via HiveMQ Cloud.' : 'Autonomous STM32 LDR tracking engaged with intermittent step controller.'),
        component: 'STM32_MCU',
      },
      ...prev.slice(0, 7),
    ]);
  }, [trackingMode, isHardwareOnline, sendCommand]);

  const handleJogAngle = useCallback((delta: number) => {
    setIsEmergencyStopped(false);
    setTrackingMode('MANUAL');
    setIsMotorMoving(true);
    const nextAngle = Math.max(-40, Math.min(40, effectiveShaftAngle + delta));
    setActualShaftAngle(Number(nextAngle.toFixed(1)));
    if (isHardwareOnline) {
      sendCommand(`GOTO ${nextAngle.toFixed(1)}`);
    }
    setTimeout(() => setIsMotorMoving(false), 800);
    setAlerts((prev) => [
      {
        id: `al-${Date.now()}`,
        time: new Date().toLocaleTimeString(),
        type: 'info',
        title: `Manual Jog: ${delta > 0 ? `+${delta}°` : `${delta}°`}`,
        message: isHardwareOnline
          ? `Dispatched 'GOTO ${nextAngle.toFixed(1)}' to physical STM32 stepper motor via Cloud.`
          : `Stepper motor jogged shafts by ${delta}°. Worm transmission locked in position.`,
        component: 'STEPPER_MOTOR',
      },
      ...prev.slice(0, 7),
    ]);
  }, [effectiveShaftAngle, isHardwareOnline, sendCommand]);

  const handleHomePosition = useCallback(() => {
    setIsEmergencyStopped(false);
    setTrackingMode('STOW');
    setIsMotorMoving(true);
    setActualShaftAngle(0.0);
    if (isHardwareOnline) {
      sendCommand('HOME');
    }
    setTimeout(() => setIsMotorMoving(false), 1000);
    setAlerts((prev) => [
      {
        id: `al-${Date.now()}`,
        time: new Date().toLocaleTimeString(),
        type: 'info',
        title: 'Home / Stow Position Commanded',
        message: isHardwareOnline
          ? "Dispatched 'HOME' calibration command to physical STM32 Hall sensor via Cloud."
          : 'All PV cell rows rotated to 0° zenith (horizontal stow profile for wind resistance / night).',
        component: 'PV_SHAFTS',
      },
      ...prev.slice(0, 7),
    ]);
  }, [isHardwareOnline, sendCommand]);

  const handleEmergencyStop = useCallback(() => {
    setIsEmergencyStopped((prev) => {
      const next = !prev;
      if (isHardwareOnline) {
        sendCommand(next ? 'STOP' : 'AUTO');
      }
      setAlerts((alertList) => [
        {
          id: `al-${Date.now()}`,
          time: new Date().toLocaleTimeString(),
          type: next ? 'error' : 'success',
          title: next ? 'EMERGENCY STOP TRIGGERED' : 'Emergency Stop Reset',
          message: next
            ? 'Stepper driver output disabled. Mechanical worm-gear self-locking holds shafts in current position.'
            : 'Emergency brake disengaged. Power restored to stepper driver.',
          component: 'STEPPER_MOTOR',
        },
        ...alertList.slice(0, 7),
      ]);
      return next;
    });
  }, [isHardwareOnline, sendCommand]);

  const handleToggleFault = useCallback(() => {
    setFaultInjected((prev) => {
      const next = !prev;
      setAlerts((alertList) => [
        {
          id: `al-${Date.now()}`,
          time: new Date().toLocaleTimeString(),
          type: next ? 'warning' : 'success',
          title: next ? 'Demo Fault Injected: Shaft #4 Friction' : 'System Normality Restored',
          message: next
            ? 'AI predictive model flagged abnormal torque and 0.6° sync deviation on Shaft #4.'
            : 'Shaft #4 resistance cleared. All telemetry returned to nominal baselines.',
          component: 'AI_ENGINE',
        },
        ...alertList.slice(0, 7),
      ]);
      return next;
    });
  }, []);

  // Generation curve profile
  const diurnalData = generateDiurnalCurve(hourDecimal);

  return {
    hourDecimal,
    isPaused,
    setIsPaused,
    simSpeed,
    setSimSpeed,
    trackingMode: effectiveTrackingMode,
    isEmergencyStopped,
    faultInjected,
    shafts,
    motor,
    solar,
    tracking,
    ai,
    alerts,
    diurnalData,
    handleAutoToggle,
    handleJogAngle,
    handleHomePosition,
    handleEmergencyStop,
    handleToggleFault,
    // Live Hardware Link State
    isHardwareOnline,
    isMqttConnected,
    telemetry,
    sendCommand,
  };
}
