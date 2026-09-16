import React from 'react';
import { 
  Gauge, 
  Thermometer, 
  ShieldCheck, 
  Cog, 
  Cable,
  Zap,
  RotateCw,
  Activity
} from 'lucide-react';
import type { MotorTelemetry } from '../types/dashboard';

interface MechanicalHealthProps {
  motor: MotorTelemetry;
  faultInjected: boolean;
}

export const MechanicalHealth: React.FC<MechanicalHealthProps> = ({
  motor,
  faultInjected,
}) => {
  return (
    <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs mb-6">
      {/* Header */}
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center gap-2">
          <div className="p-1.5 rounded-lg bg-indigo-50 text-indigo-700 border border-indigo-200/60">
            <Cog className={`w-4 h-4 ${motor.status === 'RUNNING' ? 'animate-spin' : ''}`} />
          </div>
          <div>
            <h2 className="text-base font-bold text-slate-900">Motor & Mechanical Transmission Health</h2>
            <p className="text-xs text-slate-500">Single NEMA-23 Stepper Motor + Common Worm Gear Telemetry</p>
          </div>
        </div>

        <span className={`text-xs font-mono font-semibold px-2.5 py-1 rounded-md border ${
          faultInjected 
            ? 'bg-rose-50 text-rose-700 border-rose-200' 
            : 'bg-emerald-50 text-emerald-700 border-emerald-200'
        }`}>
          {faultInjected ? 'WARNING: HIGH LOAD' : 'MECHANICAL: NOMINAL'}
        </span>
      </div>

      {/* Grid of Telemetry Cards */}
      <div className="grid grid-cols-2 sm:grid-cols-4 gap-3 mb-4">
        {/* RPM */}
        <div className="bg-slate-50 p-3 rounded-lg border border-slate-200/60">
          <div className="text-[11px] text-slate-500 font-medium flex items-center justify-between mb-1">
            <span>Motor Speed</span>
            <RotateCw className="w-3 h-3 text-indigo-500" />
          </div>
          <div className="text-xl font-bold font-mono text-slate-900">
            {motor.rpm} <span className="text-xs font-normal text-slate-500">RPM</span>
          </div>
          <div className="text-[10px] text-slate-400 mt-1 font-mono">Microstepping: 1/16</div>
        </div>

        {/* Current & Voltage */}
        <div className="bg-slate-50 p-3 rounded-lg border border-slate-200/60">
          <div className="text-[11px] text-slate-500 font-medium flex items-center justify-between mb-1">
            <span>Drive Current</span>
            <Zap className="w-3 h-3 text-amber-500" />
          </div>
          <div className={`text-xl font-bold font-mono ${faultInjected ? 'text-rose-600' : 'text-slate-900'}`}>
            {motor.current.toFixed(2)} <span className="text-xs font-normal text-slate-500">A</span>
          </div>
          <div className="text-[10px] text-slate-400 mt-1 font-mono">{motor.voltage.toFixed(1)} V DC Bus</div>
        </div>

        {/* Motor Winding Temp */}
        <div className="bg-slate-50 p-3 rounded-lg border border-slate-200/60">
          <div className="text-[11px] text-slate-500 font-medium flex items-center justify-between mb-1">
            <span>Coil Temperature</span>
            <Thermometer className="w-3 h-3 text-rose-500" />
          </div>
          <div className={`text-xl font-bold font-mono ${faultInjected ? 'text-amber-600' : 'text-slate-900'}`}>
            {motor.temperature.toFixed(1)} <span className="text-xs font-normal text-slate-500">°C</span>
          </div>
          <div className="text-[10px] text-slate-400 mt-1 font-mono">Safe Limit: &lt; 70°C</div>
        </div>

        {/* Mechanical Load */}
        <div className="bg-slate-50 p-3 rounded-lg border border-slate-200/60">
          <div className="text-[11px] text-slate-500 font-medium flex items-center justify-between mb-1">
            <span>Transmission Load</span>
            <Gauge className="w-3 h-3 text-cyan-600" />
          </div>
          <div className={`text-xl font-bold font-mono ${faultInjected ? 'text-rose-600' : 'text-slate-900'}`}>
            {motor.loadFactor}%
          </div>
          {/* Progress bar */}
          <div className="w-full bg-slate-200 h-1.5 rounded-full mt-2 overflow-hidden">
            <div 
              className={`h-full transition-all duration-500 ${
                faultInjected ? 'bg-rose-500' : 'bg-indigo-600'
              }`}
              style={{ width: `${motor.loadFactor}%` }}
            ></div>
          </div>
        </div>
      </div>

      {/* Engineering Diagnostic Details & Flexible Wiring Strip */}
      <div className="grid grid-cols-1 md:grid-cols-3 gap-3 pt-3 border-t border-slate-100 text-xs font-mono">
        {/* Worm Gear Engagement */}
        <div className="flex items-start gap-2.5 p-2.5 bg-slate-50 rounded-lg border border-slate-200/60">
          <ShieldCheck className="w-4 h-4 text-emerald-600 flex-shrink-0 mt-0.5" />
          <div>
            <div className="font-bold text-slate-800">Worm Drive Self-Locking</div>
            <div className="text-[11px] text-slate-500 mt-0.5">
              Zero back-drive under wind shear; shafts hold stationary without consuming holding torque.
            </div>
          </div>
        </div>

        {/* Shaft Synchronization Tolerance */}
        <div className="flex items-start gap-2.5 p-2.5 bg-slate-50 rounded-lg border border-slate-200/60">
          <Activity className="w-4 h-4 text-cyan-600 flex-shrink-0 mt-0.5" />
          <div>
            <div className="font-bold text-slate-800">Shaft Synchronization: OK</div>
            <div className="text-[11px] text-slate-500 mt-0.5">
              Variance across all 8 parallel shafts measured at ±0.12° (design margin &lt; 0.50°).
            </div>
          </div>
        </div>

        {/* Flexible Wiring Fatigue Monitor */}
        <div className="flex items-start gap-2.5 p-2.5 bg-slate-50 rounded-lg border border-slate-200/60">
          <Cable className="w-4 h-4 text-amber-600 flex-shrink-0 mt-0.5" />
          <div>
            <div className="font-bold text-slate-800">Flexible Wiring Harness</div>
            <div className="text-[11px] text-slate-500 mt-0.5">
              Service loop torsion cycle count: 1,420 cycles. Zero conductor fatigue or twist strain.
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
