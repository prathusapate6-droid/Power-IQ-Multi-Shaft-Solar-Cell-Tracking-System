import React, { useState, useEffect } from 'react';
import { 
  Sun, 
  Info, 
  Clock,
  Radio
} from 'lucide-react';

interface HeaderProps {
  onOpenConcept: () => void;
  isEmergencyStopped: boolean;
  isHardwareOnline?: boolean;
  isMqttConnected?: boolean;
}

export const Header: React.FC<HeaderProps> = ({
  onOpenConcept,
  isEmergencyStopped,
  isMqttConnected = true,
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
      {/* Top Banner: Direct Physical Hardware Telemetry Stream */}
      <div className="bg-gradient-to-r from-emerald-600 via-teal-600 to-emerald-700 text-white px-4 py-1.5 text-xs font-medium flex flex-wrap items-center justify-between gap-2 shadow-inner">
        <div className="flex items-center gap-2">
          <span className="bg-emerald-950/70 text-emerald-200 border border-emerald-300/40 px-2 py-0.5 rounded text-[11px] font-bold tracking-wider uppercase flex items-center gap-1.5">
            <span className="w-1.5 h-1.5 rounded-full bg-emerald-400 animate-ping"></span>
            LIVE HARDWARE IOT LINK
          </span>
          <span>
            🟢 <strong>PHYSICAL TELEMETRY STREAM:</strong> ESP32 IoT Gateway + STM32 Blue Pill (Dedicated HiveMQ Cloud TLS Encrypted)
          </span>
        </div>
        <div className="flex items-center gap-3 text-[11px] font-mono">
          <span className="hidden sm:inline opacity-90">MCU: STM32F103C8T6 (115200 Baud)</span>
          <span className="hidden md:inline opacity-90">Cloud: HiveMQ Dedicated (TLS 8883/8884)</span>
          <span className={`px-2 py-0.5 rounded font-semibold ${
            isEmergencyStopped 
              ? 'bg-rose-900/80 text-white border border-rose-400' 
              : 'bg-emerald-900/60 text-emerald-200 border border-emerald-400/40'
          }`}>
            STATUS: {isEmergencyStopped ? 'E-STOPPED' : isMqttConnected ? 'HARDWARE CONNECTED' : 'CONNECTING...'}
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
              <span className="bg-emerald-100 text-emerald-800 border border-emerald-300 text-[11px] font-semibold px-2.5 py-0.5 rounded-full flex items-center gap-1.5 font-mono">
                <span className="w-2 h-2 rounded-full bg-emerald-500 animate-pulse"></span>
                ORIGINAL HARDWARE STREAM
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

        {/* Live Status Bar */}
        <div className="flex flex-wrap items-center gap-2 sm:gap-3">
          {/* Cloud Broker Status Indicator */}
          <div 
            className="flex items-center gap-2 px-3 py-1.5 rounded-lg border text-xs font-mono bg-emerald-50/70 border-emerald-200 text-emerald-900 shadow-xs"
            title="Dedicated HiveMQ Cloud Broker (TLS Port 8884)"
          >
            <Radio className="w-3.5 h-3.5 text-emerald-600 animate-pulse" />
            <span className="text-[11px] font-semibold">
              HiveMQ Cloud: {isMqttConnected ? 'Active' : 'Connecting'}
            </span>
          </div>

          {/* Live Date & Time */}
          <div className="hidden lg:flex items-center gap-2 px-3 py-1.5 bg-slate-100/80 rounded-lg border border-slate-200 text-xs font-mono text-slate-700">
            <Clock className="w-3.5 h-3.5 text-slate-500" />
            <span>{currentDate}</span>
            <span className="font-bold text-slate-900">{currentTime}</span>
          </div>

          {/* Concept Modal Info Button */}
          <button
            onClick={onOpenConcept}
            className="flex items-center gap-1.5 px-3 py-1.5 rounded-lg bg-emerald-600 hover:bg-emerald-700 text-white text-xs font-semibold shadow-sm transition"
          >
            <Info className="w-3.5 h-3.5" />
            <span>Engineering Concept</span>
          </button>
        </div>
      </div>
    </header>
  );
};
