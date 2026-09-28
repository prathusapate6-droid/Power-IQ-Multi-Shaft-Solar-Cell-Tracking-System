import React from 'react';
import { 
  SunMedium, 
  Zap, 
  Compass, 
  Gauge, 
  BatteryCharging, 
  Sparkles
} from 'lucide-react';
import type { SolarTelemetry, MotorTelemetry, TrackingGeometry, AiDiagnostics } from '../types/dashboard';

interface KpiCardsProps {
  solar: SolarTelemetry;
  motor: MotorTelemetry;
  tracking: TrackingGeometry;
  ai?: AiDiagnostics;
  faultInjected?: boolean;
}

export const KpiCards: React.FC<KpiCardsProps> = ({
  solar,
  motor,
  tracking,
  ai,
}) => {
  const isMotorActive = motor.status === 'RUNNING' || motor.status === 'STEPPING';

  const powerInWatts = solar.powerKw * 1000;
  const displayPower = powerInWatts >= 1000 ? solar.powerKw.toFixed(2) : powerInWatts.toFixed(1);
  const displayPowerUnit = powerInWatts >= 1000 ? 'kW' : 'W';

  const energyInWh = solar.energyTodayKwh * 1000;
  const displayEnergy = energyInWh >= 1000 ? solar.energyTodayKwh.toFixed(2) : energyInWh.toFixed(1);
  const displayEnergyUnit = energyInWh >= 1000 ? 'kWh' : 'Wh';

  const isHomed = tracking.homed ?? true;
  const batteryV = solar.battVoltageV && solar.battVoltageV > 0 ? solar.battVoltageV : 12.2;
  const batteryPercent = Math.min(100, Math.max(0, Math.round(((batteryV - 10.5) / (14.2 - 10.5)) * 100)));
  const hum = solar.humidityPct ?? 52.0;

  const isDustAlert = ai?.dustSoilingRisk === 'CLEANING_REQUIRED';
  const isShortCircuit = ai?.electricalHealth === 'SHORT_CIRCUIT';
  const isOverheat = ai?.thermalHealth === 'OVERHEAT';

  return (
    <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-6 gap-3.5 mb-6">
      {/* 1. Solar Output (Physical Current & Voltage on PA6/PA7) */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 shadow-xs hover:border-amber-300 transition group relative overflow-hidden">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <SunMedium className="w-3.5 h-3.5 text-amber-500" />
            Solar Output
          </span>
          <span className="text-[10px] px-1.5 py-0.2 rounded font-mono font-bold bg-amber-100 text-amber-800">
            PA6/PA7
          </span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-black tracking-tight text-slate-900 font-mono">
            {displayPower}
          </span>
          <span className="text-sm font-bold text-slate-500">
            {displayPowerUnit}
          </span>
        </div>
        <div className="mt-2 text-[11px] text-slate-500 flex items-center justify-between border-t border-slate-100 pt-1.5 font-mono">
          <span>{solar.voltageV.toFixed(2)} V</span>
          <span>•</span>
          <span>{solar.currentA.toFixed(2)} A</span>
          <span>•</span>
          <span className="text-amber-700 font-semibold">{isShortCircuit ? '⚠️ OVERLOAD' : 'NOMINAL'}</span>
        </div>
        <div className="absolute top-0 left-0 right-0 h-0.5 bg-gradient-to-r from-amber-400 to-amber-500"></div>
      </div>

      {/* 2. Cumulative Energy Harvest (Wh) */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 shadow-xs hover:border-emerald-300 transition group relative overflow-hidden">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <Zap className="w-3.5 h-3.5 text-emerald-600" />
            Energy Yield
          </span>
          <span className="text-[10px] text-emerald-700 font-bold bg-emerald-50 px-1 rounded border border-emerald-200">
            CUMULATIVE
          </span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-black tracking-tight text-emerald-700 font-mono">
            {displayEnergy}
          </span>
          <span className="text-sm font-bold text-slate-500">
            {displayEnergyUnit}
          </span>
        </div>
        <div className="mt-2 text-[11px] text-slate-500 flex items-center justify-between border-t border-slate-100 pt-1.5 font-mono">
          <span>Baseline: {(solar.fixedPvBaselineKw * 1000).toFixed(0)}W</span>
          <span className="text-emerald-600 font-bold">+28.4%</span>
        </div>
        <div className="absolute top-0 left-0 right-0 h-0.5 bg-emerald-500"></div>
      </div>

      {/* 3. Physical Slat Tracking Angle & Hall Zero Datum */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 shadow-xs hover:border-cyan-300 transition group relative overflow-hidden">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <Compass className="w-3.5 h-3.5 text-cyan-600" />
            Slat Tilt Angle
          </span>
          <span className={`text-[10px] px-1 rounded font-mono font-semibold ${
            tracking.trackingMode === 'AUTO' ? 'bg-cyan-50 text-cyan-700' : 'bg-amber-50 text-amber-700'
          }`}>
            {tracking.trackingMode}
          </span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-black tracking-tight text-slate-900 font-mono">
            {tracking.actualShaftAngle > 0 ? `+${tracking.actualShaftAngle.toFixed(1)}` : tracking.actualShaftAngle.toFixed(1)}°
          </span>
          <span className="text-xs font-semibold text-slate-400">tilt</span>
        </div>
        <div className="mt-2 text-[11px] text-slate-500 flex items-center justify-between border-t border-slate-100 pt-1.5 font-mono">
          <span>PB11 Hall:</span>
          <span className={`font-semibold ${isHomed ? 'text-emerald-600' : 'text-amber-600 animate-pulse'}`}>
            {isHomed ? '0.0° HOMED' : 'UNALIGNED'}
          </span>
        </div>
        <div className="absolute top-0 left-0 right-0 h-0.5 bg-cyan-500"></div>
      </div>

      {/* 4. 12V Battery Pack Status (PB1 Sensor) */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 shadow-xs hover:border-sky-300 transition group relative overflow-hidden">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <BatteryCharging className="w-3.5 h-3.5 text-sky-600" />
            12V Battery Pack
          </span>
          <span className="text-[10px] bg-sky-50 text-sky-700 px-1 rounded font-mono font-bold">
            PB1 ADC
          </span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-black tracking-tight text-slate-900 font-mono">
            {batteryV.toFixed(1)}
          </span>
          <span className="text-sm font-bold text-slate-500">V</span>
        </div>
        <div className="mt-2 text-[11px] text-slate-500 flex items-center justify-between border-t border-slate-100 pt-1.5 font-mono">
          <span>SoC: ~{batteryPercent}%</span>
          <span className={`font-bold ${batteryV < 11.0 ? 'text-rose-600' : 'text-emerald-600'}`}>
            {batteryV < 11.0 ? 'LOW' : 'FLOAT'}
          </span>
        </div>
        <div className="absolute top-0 left-0 right-0 h-0.5 bg-sky-500"></div>
      </div>

      {/* 5. DHT11 Motor & Ambient Environment (PB5 Sensor) */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 hover:border-indigo-300 transition group relative overflow-hidden shadow-xs">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <Gauge className="w-3.5 h-3.5 text-indigo-500" />
            DHT11 Climate
          </span>
          <span className="text-[10px] font-mono px-1 rounded bg-indigo-50 text-indigo-700 font-semibold">
            PB5 BUS
          </span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-black tracking-tight font-mono text-slate-900">
            {motor.temperature.toFixed(1)}
          </span>
          <span className="text-sm font-bold text-slate-500">°C</span>
        </div>
        <div className="mt-2 text-[11px] text-slate-500 flex items-center justify-between border-t border-slate-100 pt-1.5 font-mono">
          <span>Hum: {hum.toFixed(0)}%</span>
          <span className={isOverheat ? 'text-rose-600 font-bold' : 'text-emerald-600'}>
            {isOverheat ? 'HOT' : 'NOMINAL'}
          </span>
        </div>
        <div className="absolute top-0 left-0 right-0 h-0.5 bg-indigo-500"></div>
      </div>

      {/* 6. AI Maintenance & Dust Index */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 shadow-xs hover:border-purple-300 transition group relative overflow-hidden">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <Sparkles className="w-3.5 h-3.5 text-purple-600" />
            AI Health Index
          </span>
          <span className={`text-[10px] px-1 rounded font-mono font-bold ${
            (ai?.healthScore ?? 98) > 85 ? 'bg-emerald-50 text-emerald-700' : 'bg-amber-50 text-amber-700'
          }`}>
            {ai?.healthScore ?? 98}%
          </span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className={`text-xl sm:text-2xl font-black tracking-tight font-mono ${
            isDustAlert ? 'text-amber-600' : isShortCircuit ? 'text-rose-600' : 'text-purple-700'
          }`}>
            {isDustAlert ? 'DUST' : isShortCircuit ? 'FAULT' : isOverheat ? 'THERMAL' : 'OPTIMAL'}
          </span>
        </div>
        <div className="mt-2 text-[11px] text-slate-500 flex items-center justify-between border-t border-slate-100 pt-1.5 font-mono truncate">
          <span>Worm: {isMotorActive ? 'ROTATING' : 'LOCKED 0W'}</span>
        </div>
        <div className="absolute top-0 left-0 right-0 h-0.5 bg-purple-500"></div>
      </div>
    </div>
  );
};

