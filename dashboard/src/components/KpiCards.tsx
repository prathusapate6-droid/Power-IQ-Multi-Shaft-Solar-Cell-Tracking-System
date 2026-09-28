import React from 'react';
import { 
  SunMedium, 
  Zap, 
  Activity, 
  Gauge, 
  Sparkles,
  Thermometer,
  Droplets
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
}) => {
  // Real Power in Watts (prototype scale)
  const powerW = solar.powerKw * 1000;
  const displayPower = powerW >= 1000 ? (powerW / 1000).toFixed(2) + ' kW' : powerW.toFixed(1) + ' W';

  // Real Energy in Watt-hours
  const energyWh = solar.energyTodayKwh * 1000;
  const displayEnergy = energyWh >= 1000 ? (energyWh / 1000).toFixed(2) + ' kWh' : energyWh.toFixed(1) + ' Wh';

  // Ambient Climate from DHT11
  const tempC = solar.temperatureC && solar.temperatureC > 0 ? solar.temperatureC : motor.temperature;
  const humPct = solar.humidityPct && solar.humidityPct > 0 ? solar.humidityPct : 52.0;

  return (
    <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-5 gap-4 mb-6">
      {/* 1. Solar Generation Power (WATTS) */}
      <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs hover:border-emerald-300 transition">
        <div className="flex items-center justify-between text-xs font-semibold text-slate-500 mb-2">
          <span className="flex items-center gap-1.5 uppercase tracking-wider text-[11px] font-bold text-emerald-700">
            <SunMedium className="w-4 h-4 text-emerald-600" />
            Solar Power
          </span>
          <span className="w-2 h-2 rounded-full bg-emerald-500 animate-pulse"></span>
        </div>
        <div className="flex items-baseline gap-1.5 mt-1">
          <span className="text-3xl sm:text-4xl font-black font-mono tracking-tight text-slate-900">
            {powerW.toFixed(1)}
          </span>
          <span className="text-base font-bold text-emerald-600 font-mono">W</span>
        </div>
        <div className="mt-2.5 pt-2 border-t border-slate-100 flex items-center justify-between text-[11px] text-slate-500 font-mono">
          <span>Active Yield</span>
          <span className="text-emerald-700 font-semibold">{displayPower} ({tracking.actualShaftAngle > 0 ? `+${tracking.actualShaftAngle.toFixed(1)}` : tracking.actualShaftAngle.toFixed(1)}° Tilt)</span>
        </div>
      </div>

      {/* 2. Solar Bus Voltage (VOLTS) */}
      <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs hover:border-blue-300 transition">
        <div className="flex items-center justify-between text-xs font-semibold text-slate-500 mb-2">
          <span className="flex items-center gap-1.5 uppercase tracking-wider text-[11px] font-bold text-blue-700">
            <Zap className="w-4 h-4 text-blue-600" />
            Solar Voltage
          </span>
          <span className="text-[10px] font-mono font-semibold bg-blue-50 text-blue-700 px-1.5 py-0.5 rounded border border-blue-200">
            PA4 ADC
          </span>
        </div>
        <div className="flex items-baseline gap-1.5 mt-1">
          <span className="text-3xl sm:text-4xl font-black font-mono tracking-tight text-slate-900">
            {solar.voltageV.toFixed(2)}
          </span>
          <span className="text-base font-bold text-blue-600 font-mono">V</span>
        </div>
        <div className="mt-2.5 pt-2 border-t border-slate-100 flex items-center justify-between text-[11px] text-slate-500 font-mono">
          <span>DC Bus Level</span>
          <span className="text-slate-700 font-semibold">{solar.voltageV > 1.0 ? 'Array Generating' : 'Standby / Low Lux'}</span>
        </div>
      </div>

      {/* 3. Solar PV Current (AMPERES) */}
      <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs hover:border-amber-300 transition">
        <div className="flex items-center justify-between text-xs font-semibold text-slate-500 mb-2">
          <span className="flex items-center gap-1.5 uppercase tracking-wider text-[11px] font-bold text-amber-700">
            <Activity className="w-4 h-4 text-amber-600" />
            Solar Current
          </span>
          <span className="text-[10px] font-mono font-semibold bg-amber-50 text-amber-700 px-1.5 py-0.5 rounded border border-amber-200">
            ACS712
          </span>
        </div>
        <div className="flex items-baseline gap-1.5 mt-1">
          <span className="text-3xl sm:text-4xl font-black font-mono tracking-tight text-slate-900">
            {solar.currentA.toFixed(2)}
          </span>
          <span className="text-base font-bold text-amber-600 font-mono">A</span>
        </div>
        <div className="mt-2.5 pt-2 border-t border-slate-100 flex items-center justify-between text-[11px] text-slate-500 font-mono">
          <span>Current Draw</span>
          <span className="text-slate-700 font-semibold">{(solar.currentA * 1000).toFixed(0)} mA</span>
        </div>
      </div>

      {/* 4. Total Energy Harvested (WATT-HOURS) */}
      <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs hover:border-purple-300 transition">
        <div className="flex items-center justify-between text-xs font-semibold text-slate-500 mb-2">
          <span className="flex items-center gap-1.5 uppercase tracking-wider text-[11px] font-bold text-purple-700">
            <Sparkles className="w-4 h-4 text-purple-600" />
            Total Energy
          </span>
          <span className="text-[10px] font-mono font-semibold bg-purple-50 text-purple-700 px-1.5 py-0.5 rounded border border-purple-200">
            Today
          </span>
        </div>
        <div className="flex items-baseline gap-1.5 mt-1">
          <span className="text-3xl sm:text-4xl font-black font-mono tracking-tight text-slate-900">
            {energyWh.toFixed(1)}
          </span>
          <span className="text-base font-bold text-purple-600 font-mono">Wh</span>
        </div>
        <div className="mt-2.5 pt-2 border-t border-slate-100 flex items-center justify-between text-[11px] text-slate-500 font-mono">
          <span>Cumulative Yield</span>
          <span className="text-purple-700 font-semibold">{displayEnergy}</span>
        </div>
      </div>

      {/* 5. Climate: Temperature & Humidity */}
      <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs hover:border-slate-300 transition col-span-1 sm:col-span-2 lg:col-span-1">
        <div className="flex items-center justify-between text-xs font-semibold text-slate-500 mb-2">
          <span className="flex items-center gap-1.5 uppercase tracking-wider text-[11px] font-bold text-slate-700">
            <Gauge className="w-4 h-4 text-slate-600" />
            Ambient Climate
          </span>
          <span className="text-[10px] font-mono font-semibold bg-slate-100 text-slate-600 px-1.5 py-0.5 rounded border border-slate-200">
            DHT11
          </span>
        </div>
        <div className="flex items-baseline justify-between mt-1">
          <div className="flex items-baseline gap-1">
            <span className="text-2xl sm:text-3xl font-black font-mono tracking-tight text-slate-900">
              {tempC.toFixed(1)}
            </span>
            <span className="text-xs font-bold text-slate-500 font-mono">°C</span>
          </div>
          <div className="flex items-baseline gap-1 border-l border-slate-200 pl-3">
            <span className="text-2xl sm:text-3xl font-black font-mono tracking-tight text-cyan-600">
              {humPct.toFixed(0)}
            </span>
            <span className="text-xs font-bold text-slate-500 font-mono">% RH</span>
          </div>
        </div>
        <div className="mt-2.5 pt-2 border-t border-slate-100 flex items-center justify-between text-[11px] text-slate-500 font-mono">
          <span className="flex items-center gap-1">
            <Thermometer className="w-3 h-3 text-rose-500" /> Temp
          </span>
          <span className="flex items-center gap-1 text-cyan-700">
            <Droplets className="w-3 h-3 text-cyan-500" /> Humidity
          </span>
        </div>
      </div>
    </div>
  );
};
