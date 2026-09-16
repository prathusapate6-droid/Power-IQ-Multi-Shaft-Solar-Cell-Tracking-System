import React from 'react';
import { 
  Network, 
  Sun, 
  Cpu, 
  Zap, 
  Cog, 
  Layers, 
  Radio, 
  BrainCircuit, 
  Activity,
  Gauge
} from 'lucide-react';

export const SystemArchitecture: React.FC = () => {
  return (
    <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs mb-6">
      {/* Header */}
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center gap-2">
          <div className="p-1.5 rounded-lg bg-slate-100 text-slate-700 border border-slate-200">
            <Network className="w-4 h-4" />
          </div>
          <div>
            <h2 className="text-base font-bold text-slate-900">
              System Architecture & Data / Actuation Pipeline
            </h2>
            <p className="text-xs text-slate-500">
              Physical Actuation Flow (Mechanical Drive) & IoT Telemetry Telematics Flow (AI Diagnostics)
            </p>
          </div>
        </div>
        <span className="text-[11px] font-mono bg-slate-100 text-slate-600 px-2 py-0.5 rounded border border-slate-200">
          Closed-Loop Topology
        </span>
      </div>

      {/* Pathway 1: Physical Mechanical Actuation Flow */}
      <div className="mb-4">
        <div className="text-xs font-bold text-slate-700 uppercase tracking-wider mb-2 flex items-center gap-1.5">
          <span className="w-2 h-2 rounded-full bg-amber-500"></span>
          <span>1. Physical Sensing & Actuation Pathway</span>
        </div>

        <div className="grid grid-cols-2 sm:grid-cols-4 lg:grid-cols-7 gap-2">
          {/* Step 1 */}
          <div className="bg-slate-50 p-2.5 rounded-lg border border-slate-200/80 flex flex-col items-center text-center">
            <Sun className="w-5 h-5 text-amber-500 mb-1" />
            <div className="text-xs font-bold text-slate-900">Sunlight</div>
            <div className="text-[10px] text-slate-500">Solar Vector</div>
          </div>

          {/* Step 2 */}
          <div className="bg-slate-50 p-2.5 rounded-lg border border-slate-200/80 flex flex-col items-center text-center">
            <Activity className="w-5 h-5 text-cyan-600 mb-1" />
            <div className="text-xs font-bold text-slate-900">LDR Sensors</div>
            <div className="text-[10px] text-slate-500">Differential Irradiance</div>
          </div>

          {/* Step 3 */}
          <div className="bg-slate-50 p-2.5 rounded-lg border border-slate-200/80 flex flex-col items-center text-center">
            <Cpu className="w-5 h-5 text-indigo-600 mb-1" />
            <div className="text-xs font-bold text-slate-900">STM32 Controller</div>
            <div className="text-[10px] text-slate-500">Intermittent Algorithm</div>
          </div>

          {/* Step 4 */}
          <div className="bg-slate-50 p-2.5 rounded-lg border border-slate-200/80 flex flex-col items-center text-center">
            <Zap className="w-5 h-5 text-amber-600 mb-1" />
            <div className="text-xs font-bold text-slate-900">Stepper Driver</div>
            <div className="text-[10px] text-slate-500">16x Microstepping</div>
          </div>

          {/* Step 5 */}
          <div className="bg-slate-50 p-2.5 rounded-lg border border-slate-200/80 flex flex-col items-center text-center">
            <Cog className="w-5 h-5 text-slate-700 mb-1" />
            <div className="text-xs font-bold text-slate-900">Worm Drive</div>
            <div className="text-[10px] text-slate-500">Single Common Axle</div>
          </div>

          {/* Step 6 */}
          <div className="bg-slate-50 p-2.5 rounded-lg border border-slate-200/80 flex flex-col items-center text-center">
            <Layers className="w-5 h-5 text-emerald-600 mb-1" />
            <div className="text-xs font-bold text-slate-900">Parallel Shafts</div>
            <div className="text-[10px] text-slate-500">8 PV Cell Rows</div>
          </div>

          {/* Step 7 */}
          <div className="bg-emerald-50 p-2.5 rounded-lg border border-emerald-200 flex flex-col items-center text-center">
            <Zap className="w-5 h-5 text-emerald-600 mb-1" />
            <div className="text-xs font-bold text-emerald-950">Solar Output</div>
            <div className="text-[10px] text-emerald-700 font-semibold">+30% Clean Power</div>
          </div>
        </div>
      </div>

      {/* Pathway 2: Telemetry, IoT & AI Diagnostics Flow */}
      <div>
        <div className="text-xs font-bold text-slate-700 uppercase tracking-wider mb-2 flex items-center gap-1.5">
          <span className="w-2 h-2 rounded-full bg-purple-500"></span>
          <span>2. IoT Telemetry, Cloud Ingestion & Predictive AI Analytics</span>
        </div>

        <div className="grid grid-cols-1 sm:grid-cols-4 gap-2">
          {/* Sub-step 1 */}
          <div className="bg-purple-50/40 p-2.5 rounded-lg border border-purple-200/70 flex items-center gap-3">
            <Gauge className="w-6 h-6 text-purple-600 flex-shrink-0" />
            <div>
              <div className="text-xs font-bold text-slate-900">V / I & Encoders</div>
              <div className="text-[10px] text-slate-500">INA219 Voltage/Current + Hall Encoders</div>
            </div>
          </div>

          {/* Sub-step 2 */}
          <div className="bg-purple-50/40 p-2.5 rounded-lg border border-purple-200/70 flex items-center gap-3">
            <Radio className="w-6 h-6 text-indigo-600 flex-shrink-0" />
            <div>
              <div className="text-xs font-bold text-slate-900">IoT Telemetry Node</div>
              <div className="text-[10px] text-slate-500">MQTT over WiFi / LoRa Packet Transmission</div>
            </div>
          </div>

          {/* Sub-step 3 */}
          <div className="bg-purple-50/40 p-2.5 rounded-lg border border-purple-200/70 flex items-center gap-3">
            <BrainCircuit className="w-6 h-6 text-purple-600 flex-shrink-0" />
            <div>
              <div className="text-xs font-bold text-slate-900">AI Predictive Engine</div>
              <div className="text-[10px] text-slate-500">Backlash, Overload & Degradation Modeling</div>
            </div>
          </div>

          {/* Sub-step 4 */}
          <div className="bg-emerald-50/60 p-2.5 rounded-lg border border-emerald-200 flex items-center gap-3">
            <Network className="w-6 h-6 text-emerald-700 flex-shrink-0" />
            <div>
              <div className="text-xs font-bold text-emerald-950">POWER IQ Web UI</div>
              <div className="text-[10px] text-emerald-800 font-semibold">Real-Time Supervisory Dashboard</div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
