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
  faultInjected?: boolean;
}

export const KpiCards: React.FC<KpiCardsProps> = ({
  solar,
  motor,
  tracking,
}) => {
  const isMotorActive = motor.status === 'RUNNING' || motor.status === 'STEPPING';

  const powerInWatts = solar.powerKw * 1000;
  const displayPower = powerInWatts >= 1000 ? solar.powerKw.toFixed(2) : powerInWatts.toFixed(1);
  const displayPowerUnit = powerInWatts >= 1000 ? 'kW' : 'W';

  const energyInWh = solar.energyTodayKwh * 1000;
  const displayEnergy = energyInWh >= 1000 ? solar.energyTodayKwh.toFixed(2) : energyInWh.toFixed(1);
  const displayEnergyUnit = energyInWh >= 1000 ? 'kWh' : 'Wh';

  return (
    <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-6 gap-3.5 mb-6">
      {/* 1. Solar Output (Physical Current & Voltage) */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 shadow-xs hover:border-emerald-300 transition group relative overflow-hidden">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <SunMedium className="w-3.5 h-3.5 text-amber-500" />
            Solar Output
          </span>
          <span className="text-[10px] px-1.5 py-0.2 rounded font-mono font-bold bg-emerald-100 text-emerald-800">
            STM32 LIVE
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
          {solar.battVoltageV !== undefined && solar.battVoltageV > 0 && (
            <>
              <span>•</span>
              <span className="text-sky-600 font-semibold" title="Battery Voltage">Bat: {solar.battVoltageV.toFixed(1)}V</span>
            </>
          )}
        </div>
        <div className="absolute top-0 left-0 right-0 h-0.5 bg-gradient-to-r from-amber-400 to-emerald-500"></div>
      </div>

      {/* 2. Cumulative Energy Harvest (Wh) */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 shadow-xs hover:border-emerald-300 transition group relative overflow-hidden">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <Zap className="w-3.5 h-3.5 text-emerald-600" />
            Energy Yield
          </span>
          <span className="text-[10px] text-emerald-600 font-bold bg-emerald-50 px-1 rounded">
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
          <span>ESP32 Integration</span>
          <span className="text-emerald-600 font-bold">LIVE</span>
        </div>
        <div className="absolute top-0 left-0 right-0 h-0.5 bg-emerald-500"></div>
      </div>

      {/* 3. Physical Slat Tracking Angle */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 shadow-xs hover:border-cyan-300 transition group relative overflow-hidden">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <Compass className="w-3.5 h-3.5 text-cyan-600" />
            Slat Tilt Angle
          </span>
          <span className="text-[10px] bg-cyan-50 text-cyan-700 px-1 rounded font-mono font-semibold">
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
          <span>Target: {tracking.targetAngle}°</span>
          <span className="text-cyan-700 font-semibold">Hall Zero Datum</span>
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
          <span>A4988 Driver</span>
          <span className="truncate">Worm Locked</span>
        </div>
        <div className={`absolute top-0 left-0 right-0 h-0.5 ${isMotorActive ? 'bg-indigo-500' : 'bg-slate-300'}`}></div>
      </div>

      {/* 5. DHT11 Motor & Ambient Temperature */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 hover:border-amber-300 transition group relative overflow-hidden shadow-xs">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <Gauge className="w-3.5 h-3.5 text-amber-500" />
            Motor Temp (DHT11)
          </span>
          <span className="text-[10px] font-mono px-1 rounded bg-slate-100 text-slate-600">
            PHYSICAL
          </span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-black tracking-tight font-mono text-slate-900">
            {motor.temperature.toFixed(1)}
          </span>
          <span className="text-sm font-bold text-slate-500">°C</span>
        </div>
        <div className="mt-2 text-[11px] text-slate-500 flex items-center justify-between border-t border-slate-100 pt-1.5 font-mono">
          <span>Driver: Nominal</span>
          <span>PA7 Sense</span>
        </div>
        <div className="absolute top-0 left-0 right-0 h-0.5 bg-amber-500"></div>
      </div>

      {/* 6. System Health */}
      <div className="bg-white rounded-xl p-4 border border-slate-200/90 shadow-xs hover:border-emerald-300 transition group relative overflow-hidden">
        <div className="flex items-center justify-between text-xs text-slate-500 font-medium mb-1.5">
          <span className="flex items-center gap-1">
            <TrendingUp className="w-3.5 h-3.5 text-emerald-600" />
            System Health
          </span>
          <span className="text-[10px] bg-emerald-50 text-emerald-700 px-1 rounded font-mono font-bold">
            OPTIMAL
          </span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-black tracking-tight text-slate-900 font-mono">
            100%
          </span>
          <span className="text-xs text-slate-400 font-medium">link</span>
        </div>
        <div className="mt-2 text-[11px] text-slate-500 flex items-center justify-between border-t border-slate-100 pt-1.5 font-mono">
          <span>STM32 UART2</span>
          <span>115200 Baud</span>
        </div>
        <div className="absolute top-0 left-0 right-0 h-0.5 bg-emerald-500"></div>
      </div>
    </div>
  );
};
