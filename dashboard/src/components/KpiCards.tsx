import React from 'react';
import { 
  SunMedium, 
  Zap, 
  Compass, 
  Gauge, 
  Activity, 
  TrendingUp
} from 'lucide-react';
import type { SolarTelemetry, MotorTelemetry, TrackingGeometry } from '../types/dashboard';

interface KpiCardsProps {
  solar: SolarTelemetry;
  motor: MotorTelemetry;
  tracking: TrackingGeometry;
  faultInjected: boolean;
}

export const KpiCards: React.FC<KpiCardsProps> = ({
  solar,
  motor,
  tracking,
  faultInjected,
}) => {
  const isMotorActive = motor.status === 'RUNNING';

  return (
    <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-6 gap-3.5 mb-6">
      {/* 1. Solar Power Output */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 shadow-xs hover:border-emerald-300 transition group relative overflow-hidden">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <SunMedium className="w-3.5 h-3.5 text-amber-500" />
            Solar Output
          </span>
          <span className={`text-[10px] px-1.5 py-0.2 rounded font-mono font-bold ${
            solar.isHardwareOnline ? 'bg-emerald-100 text-emerald-800' : 'bg-emerald-50 text-emerald-700'
          }`}>
            {solar.isHardwareOnline ? 'HARDWARE' : 'LIVE'}
          </span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-black tracking-tight text-slate-900 font-mono">
            {solar.isHardwareOnline ? (solar.powerKw * 1000).toFixed(1) : solar.powerKw.toFixed(2)}
          </span>
          <span className="text-sm font-bold text-slate-500">
            {solar.isHardwareOnline ? 'W' : 'kW'}
          </span>
        </div>
        <div className="mt-2 text-[11px] text-slate-500 flex items-center justify-between border-t border-slate-100 pt-1.5 font-mono">
          <span>{solar.voltageV.toFixed(1)} V</span>
          <span>•</span>
          <span>{solar.currentA.toFixed(2)} A</span>
          {solar.battVoltageV !== undefined && (
            <>
              <span>•</span>
              <span className="text-sky-600 font-semibold" title="Battery Voltage">Bat: {solar.battVoltageV.toFixed(1)}V</span>
            </>
          )}
        </div>
        <div className="absolute top-0 left-0 right-0 h-0.5 bg-gradient-to-r from-amber-400 to-emerald-500"></div>
      </div>

      {/* 2. Energy Generated Today */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 shadow-xs hover:border-emerald-300 transition group relative overflow-hidden">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <Zap className="w-3.5 h-3.5 text-emerald-600" />
            Energy Today
          </span>
          <span className="text-[10px] text-emerald-600 font-bold bg-emerald-50 px-1 rounded">
            {solar.isHardwareOnline ? 'CUMULATIVE' : '+30% Net'}
          </span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-black tracking-tight text-emerald-700 font-mono">
            {solar.energyTodayKwh.toFixed(2)}
          </span>
          <span className="text-sm font-bold text-slate-500">
            {solar.isHardwareOnline ? 'Wh' : 'kWh'}
          </span>
        </div>
        <div className="mt-2 text-[11px] text-slate-500 flex items-center justify-between border-t border-slate-100 pt-1.5 font-mono">
          <span>Fixed: 11.2 kWh</span>
          <span className="text-emerald-600 font-bold">Δ +3.4</span>
        </div>
        <div className="absolute top-0 left-0 right-0 h-0.5 bg-emerald-500"></div>
      </div>

      {/* 3. Solar Tracking Angle */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 shadow-xs hover:border-cyan-300 transition group relative overflow-hidden">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <Compass className="w-3.5 h-3.5 text-cyan-600" />
            Tracking Angle
          </span>
          <span className="text-[10px] bg-cyan-50 text-cyan-700 px-1 rounded font-mono">
            Target: {tracking.targetAngle}°
          </span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-black tracking-tight text-slate-900 font-mono">
            {tracking.actualShaftAngle > 0 ? `+${tracking.actualShaftAngle}` : tracking.actualShaftAngle}°
          </span>
          <span className="text-xs font-semibold text-slate-400">tilt</span>
        </div>
        <div className="mt-2 text-[11px] text-slate-500 flex items-center justify-between border-t border-slate-100 pt-1.5 font-mono">
          <span>Error: ±{tracking.trackingError}°</span>
          <span className="text-cyan-700 font-semibold">{tracking.trackingMode}</span>
        </div>
        <div className="absolute top-0 left-0 right-0 h-0.5 bg-cyan-500"></div>
      </div>

      {/* 4. Motor Status */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 shadow-xs hover:border-indigo-300 transition group relative overflow-hidden">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <Activity className="w-3.5 h-3.5 text-indigo-500" />
            Motor Status
          </span>
          <span className={`w-2 h-2 rounded-full ${isMotorActive ? 'bg-emerald-500 animate-ping' : 'bg-slate-300'}`}></span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className={`text-xl sm:text-2xl font-black tracking-tight font-mono ${
            isMotorActive ? 'text-emerald-700' : 'text-slate-600'
          }`}>
            {motor.status}
          </span>
          <span className="text-xs text-slate-400 font-medium">
            {motor.direction}
          </span>
        </div>
        <div className="mt-2 text-[11px] text-slate-500 flex items-center justify-between border-t border-slate-100 pt-1.5 font-mono">
          <span>{motor.rpm} RPM</span>
          <span className="truncate">Worm: {motor.wormDriveEngagement}</span>
        </div>
        <div className={`absolute top-0 left-0 right-0 h-0.5 ${isMotorActive ? 'bg-indigo-500' : 'bg-slate-300'}`}></div>
      </div>

      {/* 5. Motor Power */}
      <div className={`bg-white rounded-xl p-4 border transition group relative overflow-hidden shadow-xs ${
        faultInjected ? 'border-rose-300 bg-rose-50/20' : 'border-slate-200/90 hover:border-amber-300'
      }`}>
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <Gauge className="w-3.5 h-3.5 text-amber-500" />
            Motor Power
          </span>
          <span className={`text-[10px] font-mono px-1 rounded ${
            faultInjected ? 'bg-rose-100 text-rose-700 font-bold' : 'bg-slate-100 text-slate-600'
          }`}>
            {faultInjected ? 'LOAD HIGH' : '24V STEPPER'}
          </span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className={`text-2xl sm:text-3xl font-black tracking-tight font-mono ${
            faultInjected ? 'text-rose-600' : 'text-slate-900'
          }`}>
            {motor.power.toFixed(0)}
          </span>
          <span className="text-sm font-bold text-slate-500">W</span>
        </div>
        <div className="mt-2 text-[11px] text-slate-500 flex items-center justify-between border-t border-slate-100 pt-1.5 font-mono">
          <span>{motor.current.toFixed(2)} A</span>
          <span>{motor.temperature.toFixed(1)}°C</span>
        </div>
        <div className={`absolute top-0 left-0 right-0 h-0.5 ${faultInjected ? 'bg-rose-500' : 'bg-amber-500'}`}></div>
      </div>

      {/* 6. System Efficiency */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 shadow-xs hover:border-emerald-300 transition group relative overflow-hidden">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <TrendingUp className="w-3.5 h-3.5 text-emerald-600" />
            System Eff.
          </span>
          <span className="text-[10px] bg-emerald-50 text-emerald-700 px-1 rounded font-mono font-bold">
            HIGH
          </span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-black tracking-tight text-slate-900 font-mono">
            {solar.efficiency}%
          </span>
          <span className="text-xs text-slate-400 font-medium">conv.</span>
        </div>
        <div className="mt-2 text-[11px] text-slate-500 flex items-center justify-between border-t border-slate-100 pt-1.5 font-mono">
          <span>Opt: 94.8%</span>
          <span>Elec: 96.4%</span>
        </div>
        <div className="absolute top-0 left-0 right-0 h-0.5 bg-emerald-500"></div>
      </div>
    </div>
  );
};
