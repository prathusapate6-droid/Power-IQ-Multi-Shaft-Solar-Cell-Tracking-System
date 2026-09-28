import React from 'react';
import { 
  SunMedium, 
  Zap, 
  Compass, 
  Gauge, 
  BatteryCharging
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

  return (
    <div className="grid grid-cols-2 sm:grid-cols-3 lg:grid-cols-5 gap-4 mb-6">
      {/* 1. Solar Output */}
      <div className="bg-white rounded-xl p-4 border border-slate-200 shadow-xs">
        <div className="flex items-center gap-2 text-xs font-semibold text-slate-500 mb-2">
          <SunMedium className="w-4 h-4 text-amber-500" />
          <span>Solar Power</span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-bold tracking-tight text-slate-900 font-mono">
            {displayPower}
          </span>
          <span className="text-xs font-semibold text-slate-400">
            {displayPowerUnit}
          </span>
        </div>
        <div className="mt-2 text-xs text-slate-500 font-mono">
          {solar.voltageV.toFixed(1)} V &nbsp;•&nbsp; {solar.currentA.toFixed(2)} A
        </div>
      </div>

      {/* 2. Cumulative Energy Harvest */}
      <div className="bg-white rounded-xl p-4 border border-slate-200 shadow-xs">
        <div className="flex items-center gap-2 text-xs font-semibold text-slate-500 mb-2">
          <Zap className="w-4 h-4 text-emerald-600" />
          <span>Energy Yield</span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-bold tracking-tight text-slate-900 font-mono">
            {displayEnergy}
          </span>
          <span className="text-xs font-semibold text-slate-400">
            {displayEnergyUnit}
          </span>
        </div>
        <div className="mt-2 text-xs text-slate-500">
          Today's Harvest (+28% Gain)
        </div>
      </div>

      {/* 3. Slat Tracking Angle */}
      <div className="bg-white rounded-xl p-4 border border-slate-200 shadow-xs">
        <div className="flex items-center gap-2 text-xs font-semibold text-slate-500 mb-2">
          <Compass className="w-4 h-4 text-cyan-600" />
          <span>Slat Tilt</span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-bold tracking-tight text-slate-900 font-mono">
            {tracking.actualShaftAngle > 0 ? `+${tracking.actualShaftAngle.toFixed(1)}` : tracking.actualShaftAngle.toFixed(1)}°
          </span>
          <span className="text-xs font-semibold text-slate-400">tilt</span>
        </div>
        <div className="mt-2 text-xs text-slate-500">
          {tracking.trackingMode} Mode &nbsp;•&nbsp; {isHomed ? '0.0° Datum' : 'Calibrating'}
        </div>
      </div>

      {/* 4. 12V Battery Pack */}
      <div className="bg-white rounded-xl p-4 border border-slate-200 shadow-xs">
        <div className="flex items-center gap-2 text-xs font-semibold text-slate-500 mb-2">
          <BatteryCharging className="w-4 h-4 text-sky-600" />
          <span>Battery Pack</span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-bold tracking-tight text-slate-900 font-mono">
            {batteryV.toFixed(1)}
          </span>
          <span className="text-xs font-semibold text-slate-400">V</span>
        </div>
        <div className="mt-2 text-xs text-slate-500">
          ~{batteryPercent}% &nbsp;•&nbsp; {batteryV < 11.0 ? 'Low Charge' : 'Float Nominal'}
        </div>
      </div>

      {/* 5. DHT11 Climate */}
      <div className="bg-white rounded-xl p-4 border border-slate-200 shadow-xs col-span-2 sm:col-span-1">
        <div className="flex items-center gap-2 text-xs font-semibold text-slate-500 mb-2">
          <Gauge className="w-4 h-4 text-slate-500" />
          <span>Climate</span>
        </div>
        <div className="flex items-baseline gap-1">
          <span className="text-2xl sm:text-3xl font-bold tracking-tight text-slate-900 font-mono">
            {motor.temperature.toFixed(1)}
          </span>
          <span className="text-xs font-semibold text-slate-400">°C</span>
        </div>
        <div className="mt-2 text-xs text-slate-500">
          {hum.toFixed(0)}% Humidity &nbsp;•&nbsp; DHT11
        </div>
      </div>
    </div>
  );
};


