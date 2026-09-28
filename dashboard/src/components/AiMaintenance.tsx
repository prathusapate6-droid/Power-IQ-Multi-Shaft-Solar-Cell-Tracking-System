import React from 'react';
import { 
  Sparkles, 
  BrainCircuit, 
  RefreshCw,
  AlertTriangle,
  Flame,
  Zap,
  BatteryCharging,
  Compass
} from 'lucide-react';
import type { AiDiagnostics } from '../types/dashboard';

interface AiMaintenanceProps {
  ai: AiDiagnostics;
  onToggleFault?: () => void;
  onSelectScenario?: (scenario: 'NONE' | 'DUST_SOILING' | 'SHORT_CIRCUIT' | 'THERMAL_OVERHEAT' | 'LOW_BATTERY') => void;
}

export const AiMaintenance: React.FC<AiMaintenanceProps> = ({
  ai,
  onSelectScenario,
}) => {
  const isHealthy = ai.healthScore > 85;
  const isDustAlert = ai.dustSoilingRisk === 'CLEANING_REQUIRED';
  const isShortCircuit = ai.electricalHealth === 'SHORT_CIRCUIT';
  const isOverheat = ai.thermalHealth === 'OVERHEAT';
  const isLowBattery = ai.batteryHealth === 'LOW_BATTERY';
  const isHallUncalibrated = ai.hallDatumStatus === 'CALIBRATION_DUE';

  return (
    <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs mb-6">
      {/* Header with AI Badge & Status */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 mb-4">
        <div>
          <div className="flex items-center gap-2">
            <div className="p-1.5 rounded-lg bg-purple-50 text-purple-700 border border-purple-200/60">
              <BrainCircuit className="w-4 h-4" />
            </div>
            <div>
              <div className="flex items-center gap-2">
                <h2 className="text-base font-bold text-slate-900">
                  Predictive Maintenance & Fault Diagnostics
                </h2>
              </div>
              <p className="text-xs text-slate-500">
                Live monitoring: Short-circuit protection, solar bus voltage, panel soiling, and driver thermal status
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
              {ai.maintenancePrediction}
            </span>
          </div>
        </div>
      </div>

      {/* 6 Predictive Health Vectors Matrix */}
      <div className="grid grid-cols-2 sm:grid-cols-3 lg:grid-cols-6 gap-2.5 mb-4">
        {/* 1. Short Circuit / Current Inrush (PA7) */}
        <div className={`p-3 rounded-lg border transition ${
          isShortCircuit 
            ? 'bg-rose-50 border-rose-300 text-rose-900' 
            : 'bg-slate-50 border-slate-200/60'
        }`}>
          <div className="flex items-center justify-between text-[11px] text-slate-500 font-medium">
            <span>Electrical Bus</span>
            <Zap className={`w-3 h-3 ${isShortCircuit ? 'text-rose-600' : 'text-slate-400'}`} />
          </div>
          <div className="flex items-center gap-1.5 mt-1">
            <span className={`w-2 h-2 rounded-full ${isShortCircuit ? 'bg-rose-500 animate-ping' : 'bg-emerald-500'}`}></span>
            <span className="text-xs font-bold font-mono text-slate-800">
              {ai.electricalHealth}
            </span>
          </div>
          <div className="text-[10px] text-slate-400 mt-0.5 font-mono">PA7 ACS712</div>
        </div>

        {/* 2. Dust & Soiling Accumulation */}
        <div className={`p-3 rounded-lg border transition ${
          isDustAlert 
            ? 'bg-amber-50 border-amber-300 text-amber-900' 
            : 'bg-slate-50 border-slate-200/60'
        }`}>
          <div className="flex items-center justify-between text-[11px] text-slate-500 font-medium">
            <span>Dust & Soiling</span>
            <AlertTriangle className={`w-3 h-3 ${isDustAlert ? 'text-amber-600' : 'text-slate-400'}`} />
          </div>
          <div className="flex items-center gap-1.5 mt-1">
            <span className={`w-2 h-2 rounded-full ${isDustAlert ? 'bg-amber-500 animate-bounce' : 'bg-emerald-500'}`}></span>
            <span className="text-xs font-bold font-mono text-slate-800">
              {ai.dustSoilingRisk}
            </span>
          </div>
          <div className="text-[10px] text-slate-400 mt-0.5 font-mono">
            {isDustAlert ? 'CLEAN PANEL' : 'Clean Surface'}
          </div>
        </div>

        {/* 3. Thermal Runaway (PB5 DHT11) */}
        <div className={`p-3 rounded-lg border transition ${
          isOverheat 
            ? 'bg-rose-50 border-rose-300 text-rose-900' 
            : 'bg-slate-50 border-slate-200/60'
        }`}>
          <div className="flex items-center justify-between text-[11px] text-slate-500 font-medium">
            <span>Thermal Safety</span>
            <Flame className={`w-3 h-3 ${isOverheat ? 'text-rose-600' : 'text-slate-400'}`} />
          </div>
          <div className="flex items-center gap-1.5 mt-1">
            <span className={`w-2 h-2 rounded-full ${isOverheat ? 'bg-rose-500' : 'bg-emerald-500'}`}></span>
            <span className="text-xs font-bold font-mono text-slate-800">
              {ai.thermalHealth}
            </span>
          </div>
          <div className="text-[10px] text-slate-400 mt-0.5 font-mono">PB5 DHT11</div>
        </div>

        {/* 4. 12V Battery Bank (PB1) */}
        <div className={`p-3 rounded-lg border transition ${
          isLowBattery 
            ? 'bg-amber-50 border-amber-300 text-amber-900' 
            : 'bg-slate-50 border-slate-200/60'
        }`}>
          <div className="flex items-center justify-between text-[11px] text-slate-500 font-medium">
            <span>Battery Health</span>
            <BatteryCharging className={`w-3 h-3 ${isLowBattery ? 'text-amber-600' : 'text-slate-400'}`} />
          </div>
          <div className="flex items-center gap-1.5 mt-1">
            <span className={`w-2 h-2 rounded-full ${isLowBattery ? 'bg-amber-500' : 'bg-emerald-500'}`}></span>
            <span className="text-xs font-bold font-mono text-slate-800">
              {ai.batteryHealth}
            </span>
          </div>
          <div className="text-[10px] text-slate-400 mt-0.5 font-mono">PB1 Voltage</div>
        </div>

        {/* 5. Hall Zero Datum (PB11) */}
        <div className={`p-3 rounded-lg border transition ${
          isHallUncalibrated 
            ? 'bg-amber-50 border-amber-300 text-amber-900' 
            : 'bg-slate-50 border-slate-200/60'
        }`}>
          <div className="flex items-center justify-between text-[11px] text-slate-500 font-medium">
            <span>Hall 0.0° Datum</span>
            <Compass className={`w-3 h-3 ${isHallUncalibrated ? 'text-amber-600' : 'text-slate-400'}`} />
          </div>
          <div className="flex items-center gap-1.5 mt-1">
            <span className={`w-2 h-2 rounded-full ${isHallUncalibrated ? 'bg-amber-500' : 'bg-emerald-500'}`}></span>
            <span className="text-xs font-bold font-mono text-slate-800">
              {ai.hallDatumStatus}
            </span>
          </div>
          <div className="text-[10px] text-slate-400 mt-0.5 font-mono">PB11 Sensor</div>
        </div>

        {/* 6. Gear & Worm Backlash */}
        <div className="p-3 bg-slate-50 rounded-lg border border-slate-200/60">
          <div className="flex items-center justify-between text-[11px] text-slate-500 font-medium">
            <span>Worm Backlash</span>
            <RefreshCw className="w-3 h-3 text-slate-400" />
          </div>
          <div className="flex items-center gap-1.5 mt-1">
            <span className="w-2 h-2 rounded-full bg-emerald-500"></span>
            <span className="text-xs font-bold font-mono text-slate-800">{ai.gearBacklashRisk}</span>
          </div>
          <div className="text-[10px] text-slate-400 mt-0.5 font-mono">&lt; 0.04° Holding</div>
        </div>
      </div>

      {/* Diagnostic Advisory & Corrective Action */}
      <div className={`rounded-xl p-4 border transition-all ${
        ai.faultInjected 
          ? 'bg-rose-50/80 border-rose-200 text-rose-900' 
          : isDustAlert
          ? 'bg-amber-50/80 border-amber-200 text-amber-900'
          : 'bg-slate-50 border-slate-200 text-slate-800'
      }`}>
        <div className="flex items-center justify-between mb-2">
          <div className="flex items-center gap-2">
            <Sparkles className={`w-4 h-4 ${
              ai.faultInjected ? 'text-rose-600' : isDustAlert ? 'text-amber-600' : 'text-slate-600'
            }`} />
            <span className="text-xs font-bold uppercase tracking-wider">
              Diagnostic Advisory
            </span>
          </div>
          <span className="text-[10px] font-mono text-slate-400">
            Real-Time Edge Evaluation
          </span>
        </div>

        <p className="text-xs leading-relaxed font-sans font-medium">
          “{ai.aiInsightText}”
        </p>

        {/* Diagnostic Test Strip */}
        <div className="mt-3 pt-3 border-t border-slate-200/60 flex flex-wrap items-center justify-between gap-2">
          <div className="flex items-center gap-1.5 text-xs text-slate-500">
            <span>Test Diagnostic Response:</span>
            <button
              onClick={() => onSelectScenario?.('DUST_SOILING')}
              className="px-2 py-0.5 rounded bg-slate-200/80 hover:bg-slate-300 text-slate-800 text-[11px] font-medium transition"
            >
              Dust Alert
            </button>
            <button
              onClick={() => onSelectScenario?.('SHORT_CIRCUIT')}
              className="px-2 py-0.5 rounded bg-slate-200/80 hover:bg-slate-300 text-slate-800 text-[11px] font-medium transition"
            >
              Short Circuit
            </button>
            <button
              onClick={() => onSelectScenario?.('THERMAL_OVERHEAT')}
              className="px-2 py-0.5 rounded bg-slate-200/80 hover:bg-slate-300 text-slate-800 text-[11px] font-medium transition"
            >
              Overheat
            </button>
            <button
              onClick={() => onSelectScenario?.('LOW_BATTERY')}
              className="px-2 py-0.5 rounded bg-slate-200/80 hover:bg-slate-300 text-slate-800 text-[11px] font-medium transition"
            >
              Low Battery
            </button>
          </div>

          <button
            onClick={() => onSelectScenario?.('NONE')}
            className="text-slate-700 hover:text-slate-900 font-semibold text-xs flex items-center gap-1 transition"
          >
            <RefreshCw className="w-3 h-3" />
            <span>Reset to Live Sensor Feed</span>
          </button>
        </div>
      </div>

    </div>
  );
};

