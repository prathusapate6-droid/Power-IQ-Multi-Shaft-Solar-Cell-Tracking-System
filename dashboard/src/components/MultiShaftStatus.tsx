import React from 'react';
import { 
  Layers, 
  CheckCircle2, 
  AlertCircle, 
  Settings, 
  Link2,
  Info
} from 'lucide-react';
import type { ShaftData } from '../types/dashboard';

interface MultiShaftStatusProps {
  shafts: ShaftData[];
  motorMoving: boolean;
  actualAngle: number;
}

export const MultiShaftStatus: React.FC<MultiShaftStatusProps> = ({
  shafts,
  motorMoving,
  actualAngle,
}) => {
  return (
    <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs mb-6">
      {/* Section Header */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 mb-4">
        <div>
          <div className="flex items-center gap-2">
            <div className="p-1.5 rounded-lg bg-emerald-50 text-emerald-700 border border-emerald-200/60">
              <Layers className="w-4 h-4" />
            </div>
            <div>
              <h2 className="text-base font-bold text-slate-900 flex items-center gap-2">
                <span>Multi-Shaft Mechanical Synchronization</span>
                <span className="text-xs bg-emerald-100 text-emerald-800 font-semibold px-2 py-0.5 rounded border border-emerald-300">
                  8 Parallel Shafts
                </span>
              </h2>
              <p className="text-xs text-slate-500">
                Single Stepper Motor $\rightarrow$ Common Central Worm Shaft $\rightarrow$ Synchronized PV Cell Rows
              </p>
            </div>
          </div>
        </div>

        <div className="flex items-center gap-3 text-xs">
          <div className="flex items-center gap-1.5 font-mono text-slate-600 bg-slate-50 px-2.5 py-1 rounded-md border border-slate-200">
            <Link2 className="w-3.5 h-3.5 text-emerald-600" />
            <span>Master Drive: <strong>Continuous Worm Shaft</strong></span>
          </div>
          <div className="flex items-center gap-1.5 font-mono text-emerald-700 bg-emerald-50 px-2.5 py-1 rounded-md border border-emerald-200">
            <span className="w-2 h-2 rounded-full bg-emerald-500 animate-pulse"></span>
            <span>All Shafts Synced (±0.2° max)</span>
          </div>
        </div>
      </div>

      {/* Visual Mechanical Architecture Diagram (Interactive Slat Tilt Animation) */}
      <div className="bg-gradient-to-b from-slate-900 to-slate-950 rounded-xl p-4 text-white mb-5 border border-slate-800 relative overflow-hidden">
        <div className="flex items-center justify-between text-xs text-slate-400 mb-2">
          <span className="font-mono text-emerald-400 font-semibold flex items-center gap-1.5">
            <Settings className={`w-3.5 h-3.5 ${motorMoving ? 'animate-spin' : ''}`} />
            SYNCHRONIZED MECHANICAL TRANSMISSION SCHEMATIC (TOP & CROSS-SECTION)
          </span>
          <span className="text-[11px] bg-slate-800 text-slate-300 px-2 py-0.5 rounded font-mono">
            Slat Tilt Angle: {actualAngle}°
          </span>
        </div>

        {/* Isometric / Side-view PV Slats */}
        <div className="py-3 px-2 grid grid-cols-4 sm:grid-cols-8 gap-3 sm:gap-2">
          {shafts.map((shaft) => {
            const isWarn = shaft.status === 'WARN_DEVIATION';
            return (
              <div 
                key={shaft.id} 
                className={`flex flex-col items-center p-2 rounded-lg border transition ${
                  isWarn 
                    ? 'bg-rose-950/40 border-rose-500/50' 
                    : 'bg-slate-800/60 border-slate-700/60 hover:border-emerald-500/50'
                }`}
              >
                <div className="text-[10px] font-mono text-slate-400 mb-1">
                  Shaft #{shaft.id}
                </div>

                {/* Simulated PV Cell Slat Pivot Graphic */}
                <div className="w-14 h-12 flex items-center justify-center relative">
                  {/* Fixed Pivot Base */}
                  <div className="w-2 h-2 rounded-full bg-slate-500 absolute"></div>
                  {/* Worm Wheel Gear */}
                  <div className={`w-6 h-6 rounded-full border border-dashed ${
                    isWarn ? 'border-rose-400' : 'border-amber-400'
                  } absolute opacity-60`}></div>
                  {/* Rotating Solar Cell Row Slat */}
                  <div 
                    className={`w-12 h-3.5 rounded-sm shadow-md transition-transform duration-300 flex items-center justify-center text-[8px] font-bold tracking-tight ${
                      isWarn 
                        ? 'bg-gradient-to-r from-rose-600 to-amber-600 text-white' 
                        : 'bg-gradient-to-r from-blue-700 via-indigo-600 to-blue-500 text-cyan-100 border border-blue-400/40'
                    }`}
                    style={{ transform: `rotate(${-shaft.currentAngle}deg)` }}
                    title={`Shaft ${shaft.id}: ${shaft.currentAngle}°`}
                  >
                    PV CELL
                  </div>
                </div>

                {/* Telemetry readouts below slat */}
                <div className="mt-1 font-mono text-xs font-bold text-center">
                  <span className={isWarn ? 'text-rose-400' : 'text-emerald-300'}>
                    {shaft.currentAngle}°
                  </span>
                </div>
                <div className="text-[9px] font-mono text-slate-400 text-center">
                  {isWarn ? 'Δ +0.6°' : `±${shaft.variance}°`}
                </div>
              </div>
            );
          })}
        </div>

        {/* Central Worm Drive Axle Bar Graphic */}
        <div className="mt-2 pt-2 border-t border-slate-800/80 flex items-center justify-between text-[11px] font-mono text-slate-400">
          <div className="flex items-center gap-2">
            <span className="w-2 h-2 rounded-full bg-amber-400"></span>
            <span>Single Common Worm Gear Axle (Rotating @ {motorMoving ? '120 RPM' : '0 RPM'})</span>
          </div>
          <span className="text-slate-500 hidden sm:inline">Self-locking anti-backlash worm gearing</span>
        </div>
      </div>

      {/* Shafts Table List */}
      <div className="overflow-x-auto">
        <table className="w-full text-left text-xs">
          <thead>
            <tr className="border-b border-slate-200 text-slate-500 font-medium bg-slate-50/70">
              <th className="py-2.5 px-3">Shaft Identifier</th>
              <th className="py-2.5 px-3">Current Angle</th>
              <th className="py-2.5 px-3">Sync Variance</th>
              <th className="py-2.5 px-3">Gear Backlash</th>
              <th className="py-2.5 px-3">String Output</th>
              <th className="py-2.5 px-3 text-right">Mechanical Status</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-slate-100 font-mono">
            {shafts.map((shaft) => {
              const isWarn = shaft.status === 'WARN_DEVIATION';
              return (
                <tr 
                  key={shaft.id} 
                  className={`hover:bg-slate-50/80 transition ${
                    isWarn ? 'bg-rose-50/40' : ''
                  }`}
                >
                  <td className="py-2 px-3 font-semibold text-slate-800 flex items-center gap-2">
                    <span className={`w-1.5 h-1.5 rounded-full ${isWarn ? 'bg-rose-500' : 'bg-emerald-500'}`}></span>
                    <span>{shaft.name}</span>
                  </td>
                  <td className="py-2 px-3 text-slate-900 font-bold">
                    {shaft.currentAngle}°
                  </td>
                  <td className="py-2 px-3">
                    <span className={isWarn ? 'text-rose-600 font-bold' : 'text-slate-600'}>
                      {shaft.variance > 0 ? `+${shaft.variance}°` : `${shaft.variance}°`}
                    </span>
                  </td>
                  <td className="py-2 px-3 text-slate-600">
                    {shaft.wormGearBacklash}°
                  </td>
                  <td className="py-2 px-3 text-slate-600">
                    {shaft.cellStringPower} W <span className="text-[10px] text-slate-400">({shaft.cellStringVoltage}V × {shaft.cellStringCurrent}A)</span>
                  </td>
                  <td className="py-2 px-3 text-right">
                    <span className={`inline-flex items-center gap-1 px-2 py-0.5 rounded-full text-[11px] font-semibold ${
                      isWarn
                        ? 'bg-rose-100 text-rose-800 border border-rose-300'
                        : 'bg-emerald-100 text-emerald-800 border border-emerald-300'
                    }`}>
                      {isWarn ? <AlertCircle className="w-3 h-3" /> : <CheckCircle2 className="w-3 h-3" />}
                      {isWarn ? 'BACKLASH SLIP' : 'SYNCHRONIZED'}
                    </span>
                  </td>
                </tr>
              );
            })}
          </tbody>
        </table>
      </div>

      <div className="mt-3 text-[11px] text-slate-500 flex items-center justify-between border-t border-slate-100 pt-2">
        <span className="flex items-center gap-1">
          <Info className="w-3 h-3 text-slate-400" />
          Individual shafts mount rows of solar cells; worm gear eliminates need for 8 separate servo motors.
        </span>
        <span className="font-mono text-slate-400">Total Cell Rows: 8 | Transmission Ratio: 40:1</span>
      </div>
    </div>
  );
};
