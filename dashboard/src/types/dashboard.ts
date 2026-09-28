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
  isHardwareOnline?: boolean;
}

export interface TrackingGeometry {
  sunElevation: number;
  sunAzimuth: number;
  targetAngle: number;
  actualShaftAngle: number;
  trackingError: number;
  intermittentCountdownSec: number;
  isAdjusting: boolean;
  trackingMode: TrackingMode;
}

export interface HourlyGenerationPoint {
  time: string;
  hour: number;
  trackingKw: number;
  fixedKw: number;
  motorW: number;
  sunElevation: number;
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
  aiInsightText: string;
  faultInjected: boolean;
}

export interface SystemAlert {
  id: string;
  time: string;
  type: 'info' | 'success' | 'warning' | 'error';
  title: string;
  message: string;
  component: 'WORM_DRIVE' | 'STEPPER_MOTOR' | 'STM32_MCU' | 'PV_SHAFTS' | 'AI_ENGINE';
}
