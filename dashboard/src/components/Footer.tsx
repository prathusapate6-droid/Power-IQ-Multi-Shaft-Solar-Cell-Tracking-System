import React from 'react';
import { Sun } from 'lucide-react';

export const Footer: React.FC = () => {
  return (
    <footer className="bg-white border-t border-slate-200 mt-8 py-6 text-xs text-slate-500">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex flex-col md:flex-row items-center justify-between gap-4">
          <div className="flex items-center gap-2">
            <div className="h-6 w-6 rounded bg-amber-500 flex items-center justify-center text-white font-bold text-xs">
              <Sun className="w-4 h-4" />
            </div>
            <span className="font-bold text-slate-800">
              POWER IQ | Multi-Shaft Solar Cell Tracking System
            </span>
            <span className="text-slate-300 hidden sm:inline">•</span>
            <span className="text-slate-500 hidden sm:inline">Prototype Dashboard – Simulated Data</span>
          </div>

          <div className="flex items-center gap-3 font-mono text-[11px]">
            <span className="bg-amber-50 text-amber-800 px-2 py-0.5 rounded border border-amber-200 font-semibold">
              Designed for Hackathon Demonstration
            </span>
            <span className="text-slate-400">Deployable via Vercel / Netlify</span>
          </div>
        </div>

        <div className="mt-4 pt-4 border-t border-slate-100 flex flex-col sm:flex-row items-center justify-between text-[11px] text-slate-400 gap-2">
          <p>
            Engineering Concept: "Rotate the Cells, Not the Panel" • 1 Motor • 8 Synchronized PV Shafts • STM32 MCU
          </p>
          <p>
            Client-Side IoT Telemetry Simulation • Academic & Demonstration Use Only
          </p>
        </div>
      </div>
    </footer>
  );
};
