import React from 'react';
import { Sun } from 'lucide-react';

export const Footer: React.FC = () => {
  return (
    <footer className="bg-white border-t border-slate-200 mt-8 py-5 text-xs text-slate-500">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex flex-col sm:flex-row items-center justify-between gap-3">
          <div className="flex items-center gap-2">
            <div className="h-5 w-5 rounded bg-slate-900 flex items-center justify-center text-amber-400">
              <Sun className="w-3.5 h-3.5" />
            </div>
            <span className="font-bold text-slate-800">
              POWER IQ
            </span>
            <span className="text-slate-300">•</span>
            <span className="text-slate-500">
              Multi-Shaft Solar Cell Tracking System
            </span>
          </div>

          <div className="text-[11px] text-slate-400 font-mono">
            Dual-MCU Controller Architecture (STM32F103 + ESP32 IoT Gateway)
          </div>
        </div>
      </div>
    </footer>
  );
};

