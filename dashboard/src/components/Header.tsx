import React, { useState, useEffect } from 'react';
import { 
  Sun, 
  Play, 
  Pause, 
  AlertTriangle, 
  Info, 
  Clock
} from 'lucide-react';

interface HeaderProps {
  isPaused: boolean;
  onTogglePause: () => void;
  faultInjected: boolean;
  onToggleFault: () => void;
  simSpeed: number;
  onChangeSpeed: (speed: number) => void;
  onOpenConcept: () => void;
  isEmergencyStopped: boolean;
}

export const Header: React.FC<HeaderProps> = ({
  isPaused,
  onTogglePause,
  faultInjected,
  onToggleFault,
  simSpeed,
  onChangeSpeed,
  onOpenConcept,
  isEmergencyStopped,
}) => {
  const [currentTime, setCurrentTime] = useState<string>('');
  const [currentDate, setCurrentDate] = useState<string>('');

  useEffect(() => {
    const updateTime = () => {
      const now = new Date();
      setCurrentTime(now.toLocaleTimeString('en-US', { hour12: false }));
      setCurrentDate(now.toLocaleDateString('en-US', { 
        weekday: 'short', 
        year: 'numeric', 
        month: 'short', 
        day: 'numeric' 
      }));
    };
    updateTime();
    const timer = setInterval(updateTime, 1000);
    return () => clearInterval(timer);
  }, []);

  return (
    <header className="bg-white border-b border-slate-200 sticky top-0 z-30 shadow-sm">
      {/* Top Banner: Prototype & Simulation Disclaimer */}
      <div className="bg-gradient-to-r from-amber-500 via-orange-500 to-amber-600 text-white px-4 py-1 text-xs font-medium flex flex-wrap items-center justify-between gap-2 shadow-inner">
        <div className="flex items-center gap-2">
          <span className="bg-black/25 px-2 py-0.5 rounded text-[11px] font-bold tracking-wider uppercase border border-white/20">
            Hackathon Prototype
          </span>
          <span>
            ⚠️ <strong>SIMULATED HARDWARE DATA:</strong> Physical hardware not connected. Telemetry generated via kinematic & solar physics model.
          </span>
        </div>
        <div className="flex items-center gap-3 text-[11px] font-mono">
          <span className="hidden sm:inline opacity-90">Microcontroller: STM32F401RE</span>
          <span className="hidden md:inline opacity-90">Drive: 1x NEMA23 + Common Worm Axle</span>
          <span className="bg-white/20 px-2 py-0.5 rounded text-white font-semibold">
            STATUS: {isEmergencyStopped ? 'E-STOPPED' : 'ONLINE'}
          </span>
        </div>
      </div>

      {/* Main Navigation Header */}
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-3.5 flex flex-col md:flex-row md:items-center md:justify-between gap-3">
        {/* Brand & Subtitle */}
        <div className="flex items-center gap-3">
          <div className="h-11 w-11 rounded-xl bg-gradient-to-br from-amber-500 to-emerald-600 p-0.5 shadow-md flex-shrink-0 flex items-center justify-center">
            <div className="w-full h-full bg-slate-900 rounded-[10px] flex items-center justify-center relative overflow-hidden">
              <Sun className="w-6 h-6 text-amber-400" />
              <div className="absolute -bottom-1 -right-1 w-4 h-4 bg-emerald-500 rounded-full flex items-center justify-center text-[8px] text-white font-bold">
                IQ
              </div>
            </div>
          </div>
          <div>
            <div className="flex items-center gap-2">
              <h1 className="text-2xl font-black tracking-tight text-slate-900 flex items-center gap-1.5">
                POWER <span className="text-amber-500">IQ</span>
              </h1>
              <span className="bg-emerald-100 text-emerald-800 text-[11px] font-semibold px-2 py-0.5 rounded-full border border-emerald-300 flex items-center gap-1">
                <span className="w-1.5 h-1.5 rounded-full bg-emerald-500 animate-ping"></span>
                ONLINE / PROTOTYPE
              </span>
            </div>
            <p className="text-xs text-slate-500 font-medium flex items-center gap-1.5 mt-0.5">
              <span>Multi-Shaft Solar Cell Tracking System</span>
              <span className="text-slate-300">•</span>
              <span className="text-emerald-700 font-semibold bg-emerald-50 px-1.5 py-0.2 rounded border border-emerald-200">
                Rotate The Cells, Not The Panel
              </span>
            </p>
          </div>
        </div>

        {/* Live Clock & Interactive Simulation Bar */}
        <div className="flex flex-wrap items-center gap-2 sm:gap-3">
          {/* Live Date & Time */}
          <div className="hidden lg:flex items-center gap-2 px-3 py-1.5 bg-slate-100/80 rounded-lg border border-slate-200 text-xs font-mono text-slate-700">
            <Clock className="w-3.5 h-3.5 text-slate-500" />
            <span>{currentDate}</span>
            <span className="font-bold text-slate-900">{currentTime}</span>
          </div>

          {/* Simulation Controls for Evaluators */}
          <div className="flex items-center gap-1.5 bg-slate-100 p-1 rounded-lg border border-slate-200">
            <button
              onClick={onTogglePause}
              title={isPaused ? "Resume telemetry simulation" : "Pause telemetry simulation"}
              className={`flex items-center gap-1 px-2.5 py-1 rounded text-xs font-medium transition ${
                isPaused 
                  ? 'bg-amber-500 text-white shadow-sm' 
                  : 'bg-white text-slate-700 hover:bg-slate-50 shadow-xs'
              }`}
            >
              {isPaused ? <Play className="w-3 h-3" /> : <Pause className="w-3 h-3" />}
              <span>{isPaused ? 'Resume' : 'Pause'}</span>
            </button>

            {/* Sim Speed Toggle */}
            <div className="flex items-center text-xs font-mono">
              {[1, 5, 15].map((speed) => (
                <button
                  key={speed}
                  onClick={() => onChangeSpeed(speed)}
                  className={`px-2 py-0.5 rounded text-[11px] font-medium transition ${
                    simSpeed === speed 
                      ? 'bg-slate-800 text-white' 
                      : 'text-slate-600 hover:text-slate-900'
                  }`}
                >
                  {speed}x
                </button>
              ))}
            </div>
          </div>

          {/* Demo Fault Injection Button (Hackathon Highlight) */}
          <button
            onClick={onToggleFault}
            className={`flex items-center gap-1.5 px-3 py-1.5 rounded-lg text-xs font-semibold border transition shadow-sm ${
              faultInjected
                ? 'bg-rose-50 border-rose-300 text-rose-700 hover:bg-rose-100 animate-pulse'
                : 'bg-white border-slate-200 text-slate-700 hover:bg-slate-50 hover:border-slate-300'
            }`}
            title="Toggle simulated mechanical drag & current anomaly to test AI fault detection"
          >
            <AlertTriangle className={`w-3.5 h-3.5 ${faultInjected ? 'text-rose-600' : 'text-amber-500'}`} />
            <span>{faultInjected ? 'Fault Active (Shaft #4)' : 'Simulate Fault'}</span>
          </button>

          {/* Concept Modal Info Button */}
          <button
            onClick={onOpenConcept}
            className="flex items-center gap-1 px-3 py-1.5 rounded-lg bg-emerald-600 hover:bg-emerald-700 text-white text-xs font-semibold shadow-sm transition"
          >
            <Info className="w-3.5 h-3.5" />
            <span className="hidden sm:inline">Engineering Concept</span>
          </button>
        </div>
      </div>
    </header>
  );
};
