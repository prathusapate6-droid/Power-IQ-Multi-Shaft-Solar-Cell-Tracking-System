import React from 'react';
import { 
  Sliders, 
  RotateCw, 
  RotateCcw, 
  Home, 
  OctagonAlert, 
  Hand, 
  Compass, 
  ShieldCheck
} from 'lucide-react';
import type { TrackingMode } from '../types/dashboard';

interface ControlPanelProps {
  trackingMode: TrackingMode;
  onToggleAuto: () => void;
  onJogAngle: (delta: number) => void;
  onHomePosition: () => void;
  onEmergencyStop: () => void;
  isEmergencyStopped: boolean;
  actualAngle: number;
}

export const ControlPanel: React.FC<ControlPanelProps> = ({
  trackingMode,
  onToggleAuto,
  onJogAngle,
  onHomePosition,
  onEmergencyStop,
  isEmergencyStopped,
  actualAngle,
}) => {
  return (
    <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs mb-6">
      {/* Header */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 mb-4">
        <div className="flex items-center gap-2">
          <div className="p-1.5 rounded-lg bg-slate-100 text-slate-700 border border-slate-200">
            <Sliders className="w-4 h-4" />
          </div>
          <div>
            <h2 className="text-base font-bold text-slate-900 flex items-center gap-2">
              <span>System Actuation & Control Simulation</span>
              <span className="text-[10px] font-mono uppercase bg-amber-100 text-amber-900 px-2 py-0.5 rounded border border-amber-300 font-bold">
                Simulation Only
              </span>
            </h2>
            <p className="text-xs text-slate-500">
              Interactive test interface simulating STM32 stepper motor driver commands
            </p>
          </div>
        </div>

        {/* Current Slat Angle Badge */}
        <div className="flex items-center gap-2 text-xs font-mono">
          <span className="text-slate-500">Shaft Position:</span>
          <span className="bg-slate-900 text-amber-400 px-2.5 py-1 rounded-md font-bold text-sm">
            {actualAngle > 0 ? `+${actualAngle}°` : `${actualAngle}°`}
          </span>
        </div>
      </div>

      {/* Control Buttons Grid */}
      <div className="grid grid-cols-2 sm:grid-cols-3 lg:grid-cols-6 gap-2.5">
        {/* Automatic Tracking Toggle */}
        <button
          onClick={onToggleAuto}
          className={`flex flex-col items-center justify-center p-3 rounded-xl border text-center transition shadow-xs ${
            trackingMode === 'AUTO'
              ? 'bg-emerald-600 border-emerald-700 text-white hover:bg-emerald-700'
              : 'bg-white border-slate-200 text-slate-700 hover:bg-slate-50'
          }`}
        >
          <div className="flex items-center gap-1 mb-1">
            {trackingMode === 'AUTO' ? <Compass className="w-4 h-4 animate-spin-slow" /> : <Hand className="w-4 h-4" />}
            <span className="text-xs font-bold">
              {trackingMode === 'AUTO' ? 'AUTO TRACKING: ON' : 'MANUAL MODE'}
            </span>
          </div>
          <span className={`text-[10px] ${trackingMode === 'AUTO' ? 'text-emerald-100' : 'text-slate-500'}`}>
            {trackingMode === 'AUTO' ? 'STM32 LDR Intermittent' : 'Click to enable Auto'}
          </span>
        </button>

        {/* Manual Jog -5° (Eastward) */}
        <button
          onClick={() => onJogAngle(-5)}
          disabled={isEmergencyStopped}
          className="flex flex-col items-center justify-center p-3 rounded-xl border border-slate-200 bg-white text-slate-800 hover:bg-slate-50 hover:border-slate-300 transition shadow-xs disabled:opacity-50"
        >
          <div className="flex items-center gap-1 mb-1">
            <RotateCcw className="w-4 h-4 text-cyan-600" />
            <span className="text-xs font-bold">Rotate -5°</span>
          </div>
          <span className="text-[10px] text-slate-500 font-mono">CCW (East Jog)</span>
        </button>

        {/* Manual Jog +5° (Westward) */}
        <button
          onClick={() => onJogAngle(5)}
          disabled={isEmergencyStopped}
          className="flex flex-col items-center justify-center p-3 rounded-xl border border-slate-200 bg-white text-slate-800 hover:bg-slate-50 hover:border-slate-300 transition shadow-xs disabled:opacity-50"
        >
          <div className="flex items-center gap-1 mb-1">
            <RotateCw className="w-4 h-4 text-cyan-600" />
            <span className="text-xs font-bold">Rotate +5°</span>
          </div>
          <span className="text-[10px] text-slate-500 font-mono">CW (West Jog)</span>
        </button>

        {/* Fine Step ±1° */}
        <button
          onClick={() => onJogAngle(1)}
          disabled={isEmergencyStopped}
          className="flex flex-col items-center justify-center p-3 rounded-xl border border-slate-200 bg-white text-slate-800 hover:bg-slate-50 hover:border-slate-300 transition shadow-xs disabled:opacity-50"
        >
          <div className="flex items-center gap-1 mb-1">
            <Sliders className="w-4 h-4 text-indigo-600" />
            <span className="text-xs font-bold">Fine Step +1°</span>
          </div>
          <span className="text-[10px] text-slate-500 font-mono">Microstep Calib</span>
        </button>

        {/* Home / Stow Position */}
        <button
          onClick={onHomePosition}
          disabled={isEmergencyStopped}
          className="flex flex-col items-center justify-center p-3 rounded-xl border border-slate-200 bg-white text-slate-800 hover:bg-slate-50 hover:border-slate-300 transition shadow-xs disabled:opacity-50"
        >
          <div className="flex items-center gap-1 mb-1">
            <Home className="w-4 h-4 text-amber-600" />
            <span className="text-xs font-bold">Home (0° Stow)</span>
          </div>
          <span className="text-[10px] text-slate-500 font-mono">Horizontal Stow</span>
        </button>

        {/* Emergency Stop Button */}
        <button
          onClick={onEmergencyStop}
          className={`flex flex-col items-center justify-center p-3 rounded-xl border text-center transition shadow-xs ${
            isEmergencyStopped
              ? 'bg-rose-600 border-rose-700 text-white animate-pulse'
              : 'bg-white border-rose-200 text-rose-700 hover:bg-rose-50'
          }`}
        >
          <div className="flex items-center gap-1 mb-1">
            <OctagonAlert className="w-4 h-4 text-rose-600" />
            <span className="text-xs font-bold">
              {isEmergencyStopped ? 'E-STOP ACTIVE' : 'Emergency Stop'}
            </span>
          </div>
          <span className={`text-[10px] ${isEmergencyStopped ? 'text-rose-100' : 'text-rose-500'}`}>
            {isEmergencyStopped ? 'Click to Reset' : 'Disable Stepper Driver'}
          </span>
        </button>
      </div>

      <div className="mt-3 pt-2.5 border-t border-slate-100 flex items-center justify-between text-[11px] text-slate-500">
        <span className="flex items-center gap-1.5">
          <ShieldCheck className="w-3.5 h-3.5 text-slate-400" />
          <span>Intermittent Tracking Mode activates motor for only 2–3 seconds every few minutes, minimizing parasitic motor energy consumption.</span>
        </span>
        <span className="font-mono text-slate-400 hidden sm:inline">Actuator: High-Torque Bipolar Stepper (1.8°/step, 16x microstep)</span>
      </div>
    </div>
  );
};
