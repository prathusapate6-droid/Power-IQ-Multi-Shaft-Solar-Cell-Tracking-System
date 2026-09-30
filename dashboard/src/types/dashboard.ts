export type SystemStatus = 'ONLINE' | 'STANDBY' | 'FAULT_WARNING' | 'EMERGENCY_STOP';

export type TrackingMode = 'AUTO' | 'MANUAL' | 'STOW';

export interface ShaftData {
  id: number;
  name: string;
  currentAngle: number;
  targetAngle: number;
  variance: number;
  status: 'SYNCHRONIZED' | 'ALIGNED' | 'WARN_DEVIATION';
  wormGearBacklash: number;
  cellStringVoltage: number;
  cellStringCurrent: number;
  cellStringPower: number;
}

export interface MotorTelemetry {
  status: 'RUNNING' | 'IDLE' | 'STEPPING' | 'STOPPED';
  rpm: number;
  voltage: number;
  current: number;
  power: number;
  temperature: number;
  direction: 'CW' | 'CCW' | 'HOLD';
  totalSteps: number;
  loadFactor: number;
  wormDriveEngagement: 'LOCKED' | 'ROTATING' | 'DISENGAGED';
}

export interface SolarTelemetry {
  powerKw: number;
  voltageV: number;
  currentA: number;
  energyTodayKwh: number;
  efficiency: number;
  fixedPvBaselineKw: number;
  instantGainPercent: number;
  irradianceWm2: number;
  battVoltageV?: number;
  temperatureC?: number;
  humidityPct?: number;
  homed?: boolean;
  isHardwareOnline?: boolean;
}

export interface TrackingGeometry {
  sunElevation: number;
  sunAzimuth: number;
  targetAngle: number;
  actualShaftAngle: number;
  potAngle?: number;
  trackingError: number;
  intermittentCountdownSec: number;
  isAdjusting: boolean;
  trackingMode: TrackingMode;
  homed?: boolean;
}

export interface HourlyGenerationPoint {
  time: string;
  hour: number;
  trackingKw: number;
  fixedKw: number;
  trackingW: number;
  fixedW: number;
  motorW: number;
  sunElevation: number;
  solarVoltage?: number;
  battVoltage?: number;
  solarCurrent?: number;
  temperature?: number;
  humidity?: number;
}


export interface AiDiagnostics {
  healthScore: number;
  gearBacklashRisk: 'LOW' | 'MODERATE' | 'HIGH';
  motorHealth: 'GOOD' | 'NORMAL' | 'WARNING';
  shaftSynchronization: 'GOOD' | 'ACCEPTABLE' | 'DEGRADED';
  overloadRisk: 'LOW' | 'MODERATE' | 'HIGH';
  flexibleCableFatigue: 'LOW' | 'NORMAL' | 'EVALUATE';
  bearingFriction: 'NOMINAL' | 'ELEVATED' | 'HIGH';
  maintenancePrediction: 'NORMAL' | 'INSPECTION_RECOMMENDED' | 'CRITICAL';
  dustSoilingRisk: 'CLEAN' | 'MODERATE_DUST' | 'CLEANING_REQUIRED';
  thermalHealth: 'NOMINAL' | 'ELEVATED' | 'OVERHEAT';
  electricalHealth: 'NORMAL' | 'SHORT_CIRCUIT' | 'OVERVOLTAGE' | 'PV_DISCONNECTED';
  batteryHealth: 'OPTIMAL' | 'LOW_BATTERY' | 'OVERCHARGED' | 'DISCONNECTED';
  hallDatumStatus: 'ALIGNED' | 'CALIBRATION_DUE';
  cleaningRecommended: boolean;
  aiInsightText: string;
  activeScenario?: 'NONE' | 'DUST_SOILING' | 'MECHANICAL_JAM' | 'SHORT_CIRCUIT' | 'THERMAL_OVERHEAT' | 'LOW_BATTERY' | 'NIGHT_SETTLE';
  faultInjected: boolean;
  // Mathematical In-House AI Telemetry Vectors
  cleannessRatio: number;         // 0 - 100% Cleanness index
  expectedPowerW: number;         // Theoretical power from LDR & Cosine tilt
  actualPowerW: number;           // Instantaneous measured electrical power
  soilingLossPct: number;         // Suppressed yield loss percentage
  mechanicalStressPct: number;    // Motor current vs 2.05A jam limit
  thermalMarginPct: number;       // Heatsink/ambient margin before 48°C limit
  diagnosedState: 'NOMINAL' | 'SOILED_PANEL' | 'MECHANICAL_JAM' | 'BATTERY_UNDERVOLTAGE' | 'THERMAL_OVERHEAT' | 'NIGHT_HOLD';
  stateConfidence: {
    nominal: number;
    soiled: number;
    jam: number;
    undervoltage: number;
    overheat: number;
    night: number;
  };
}

export interface SystemAlert {
  id: string;
  time: string;
  type: 'info' | 'success' | 'warning' | 'error';
  title: string;
  message: string;
  component: 'WORM_DRIVE' | 'STEPPER_MOTOR' | 'STM32_MCU' | 'PV_SHAFTS' | 'AI_ENGINE' | 'BATTERY' | 'SOLAR_BUS';
}

