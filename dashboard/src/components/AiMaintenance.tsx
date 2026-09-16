import React from 'react';
import { 
  Sparkles, 
  BrainCircuit, 
  RefreshCw
} from 'lucide-react';
import type { AiDiagnostics } from '../types/dashboard';

interface AiMaintenanceProps {
  ai: AiDiagnostics;
  onToggleFault: () => void;
}

export const AiMaintenance: React.FC<AiMaintenanceProps> = ({
  ai,
  onToggleFault,
}) => {
  const isHealthy = ai.healthScore > 85;

  return (
    <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs mb-6">
      {/* Header with Prototype Badges */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 mb-4">
        <div>
          <div className="flex items-center gap-2">
            <div className="p-1.5 rounded-lg bg-purple-50 text-purple-700 border border-purple-200/60">
              <BrainCircuit className="w-4 h-4" />
            </div>
            <div>
              <div className="flex items-center gap-2">
                <h2 className="text-base font-bold text-slate-900">
                  AI-Based Predictive Maintenance & System Health
                </h2>
                <span className="text-[10px] uppercase font-bold tracking-wider bg-purple-100 text-purple-800 px-2 py-0.5 rounded border border-purple-300">
                  AI-Based Predictive Analysis – Prototype
                </span>
              </div>
              <p className="text-xs text-slate-500">
                Pattern anomaly detection on motor current harmonics, gear backlash, and shaft synchronization
              </p>
            </div>
          </div>
        </div>

        {/* AI Health Score Badge */}
        <div className="flex items-center gap-2">
          <div className="flex items-center gap-2 bg-slate-50 px-3 py-1.5 rounded-lg border border-slate-200">
            <span className="text-xs text-slate-500 font-medium">Health Index:</span>
            <span className={`text-base font-black font-mono ${
              isHealthy ? 'text-emerald-700' : 'text-rose-600'
            }`}>
              {ai.healthScore}%
            </span>
            <span className={`text-[10px] font-bold px-1.5 py-0.5 rounded ${
              isHealthy ? 'bg-emerald-100 text-emerald-800' : 'bg-rose-100 text-rose-800'
            }`}>
              {isHealthy ? 'HEALTHY' : 'WARNING'}
            </span>
          </div>
        </div>
      </div>

      {/* Predictive Health Matrix (KPI Indicators) */}
      <div className="grid grid-cols-2 sm:grid-cols-5 gap-2.5 mb-4">
        {/* Gear Backlash Risk */}
        <div className="p-3 bg-slate-50 rounded-lg border border-slate-200/60">
          <div className="text-[11px] text-slate-500 font-medium">Gear Backlash Risk</div>
          <div className="flex items-center gap-1.5 mt-1">
            <span className={`w-2 h-2 rounded-full ${ai.gearBacklashRisk === 'LOW' ? 'bg-emerald-500' : 'bg-amber-500'}`}></span>
            <span className="text-sm font-bold font-mono text-slate-800">{ai.gearBacklashRisk}</span>
          </div>
          <div className="text-[10px] text-slate-400 mt-0.5 font-mono">&lt; 0.04° backlash</div>
        </div>

        {/* Motor Health */}
        <div className="p-3 bg-slate-50 rounded-lg border border-slate-200/60">
          <div className="text-[11px] text-slate-500 font-medium">Motor Health</div>
          <div className="flex items-center gap-1.5 mt-1">
            <span className={`w-2 h-2 rounded-full ${ai.motorHealth === 'GOOD' ? 'bg-emerald-500' : 'bg-rose-500'}`}></span>
            <span className="text-sm font-bold font-mono text-slate-800">{ai.motorHealth}</span>
          </div>
          <div className="text-[10px] text-slate-400 mt-0.5 font-mono">Current harmonics</div>
        </div>

        {/* Shaft Synchronization */}
        <div className="p-3 bg-slate-50 rounded-lg border border-slate-200/60">
          <div className="text-[11px] text-slate-500 font-medium">Shaft Sync Health</div>
          <div className="flex items-center gap-1.5 mt-1">
            <span className={`w-2 h-2 rounded-full ${ai.shaftSynchronization === 'GOOD' ? 'bg-emerald-500' : 'bg-rose-500'}`}></span>
            <span className="text-sm font-bold font-mono text-slate-800">{ai.shaftSynchronization}</span>
          </div>
          <div className="text-[10px] text-slate-400 mt-0.5 font-mono">8 shafts synced</div>
        </div>

        {/* Overload Risk */}
        <div className="p-3 bg-slate-50 rounded-lg border border-slate-200/60">
          <div className="text-[11px] text-slate-500 font-medium">Overload Risk</div>
          <div className="flex items-center gap-1.5 mt-1">
            <span className={`w-2 h-2 rounded-full ${ai.overloadRisk === 'LOW' ? 'bg-emerald-500' : 'bg-amber-500'}`}></span>
            <span className="text-sm font-bold font-mono text-slate-800">{ai.overloadRisk}</span>
          </div>
          <div className="text-[10px] text-slate-400 mt-0.5 font-mono">Torque margins</div>
        </div>

        {/* Maintenance Prediction */}
        <div className="p-3 bg-slate-50 rounded-lg border border-slate-200/60 col-span-2 sm:col-span-1">
          <div className="text-[11px] text-slate-500 font-medium">Maintenance State</div>
          <div className="flex items-center gap-1.5 mt-1">
            <span className={`w-2 h-2 rounded-full ${ai.maintenancePrediction === 'NORMAL' ? 'bg-emerald-500' : 'bg-amber-500'}`}></span>
            <span className="text-xs font-bold font-mono text-slate-800">{ai.maintenancePrediction}</span>
          </div>
          <div className="text-[10px] text-slate-400 mt-0.5 font-mono">30-day forecast</div>
        </div>
      </div>

      {/* AI Maintenance Insight Box */}
      <div className={`rounded-xl p-4 border transition-all ${
        ai.faultInjected 
          ? 'bg-rose-50/80 border-rose-200 text-rose-900' 
          : 'bg-gradient-to-r from-purple-50/70 to-slate-50 border-purple-200/70 text-slate-800'
      }`}>
        <div className="flex items-center justify-between mb-2">
          <div className="flex items-center gap-2">
            <Sparkles className={`w-4 h-4 ${ai.faultInjected ? 'text-rose-600' : 'text-purple-600'}`} />
            <span className="text-xs font-bold uppercase tracking-wider">
              AI Maintenance Insight
            </span>
          </div>
          <span className="text-[10px] font-mono text-slate-500">
            Model: Multi-Shaft Telemetry Diagnostic Net (Demonstration)
          </span>
        </div>

        <p className="text-xs leading-relaxed font-sans font-medium">
          “{ai.aiInsightText}”
        </p>

        <div className="mt-3 pt-2.5 border-t border-purple-200/40 flex flex-wrap items-center justify-between text-[11px] text-slate-500 gap-2">
          <span className="italic">
            Note: Demonstrates how future IoT predictive analytics will monitor continuous worm drive torque and individual shaft encoders.
          </span>
          <button
            onClick={onToggleFault}
            className="text-purple-700 hover:text-purple-900 font-semibold underline text-[11px] flex items-center gap-1"
          >
            <RefreshCw className="w-3 h-3" />
            <span>{ai.faultInjected ? 'Clear Anomaly' : 'Test Anomaly Scenario'}</span>
          </button>
        </div>
      </div>
    </div>
  );
};
