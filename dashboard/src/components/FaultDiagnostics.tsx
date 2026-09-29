import React from 'react';
import { 
  ShieldCheck, 
  AlertTriangle, 
  Flame, 
  Zap, 
  Compass, 
  Radio, 
  CheckCircle2, 
  AlertCircle, 
  Clock, 
  Info,
  Server,
  Brain,
  Cpu,
  Sparkles
} from 'lucide-react';
import type { AiDiagnostics, SystemAlert } from '../types/dashboard';

interface FaultDiagnosticsProps {
  ai: AiDiagnostics;
  alerts: SystemAlert[];
  isHardwareOnline?: boolean;
}

export const FaultDiagnostics: React.FC<FaultDiagnosticsProps> = ({
  ai,
  alerts,
  isHardwareOnline = true,
}) => {
  const isDustAlert = ai.dustSoilingRisk === 'CLEANING_REQUIRED';
  const isShortCircuit = ai.electricalHealth === 'SHORT_CIRCUIT';
  const isOverheat = ai.thermalHealth === 'OVERHEAT';
  const isHallUncalibrated = ai.hallDatumStatus === 'CALIBRATION_DUE';

  const criticalFaultsCount = (isShortCircuit ? 1 : 0) + (isOverheat ? 1 : 0) + (!isHardwareOnline ? 1 : 0);
  const warningFaultsCount = (isDustAlert ? 1 : 0) + (isHallUncalibrated ? 1 : 0);

  return (
    <div className="space-y-6">
      {/* 1. Master System Fault Status Banner */}
      <div className={`p-4 rounded-xl border flex flex-col sm:flex-row sm:items-center justify-between gap-3 shadow-xs ${
        criticalFaultsCount > 0
          ? 'bg-rose-50 border-rose-300 text-rose-900'
          : warningFaultsCount > 0
          ? 'bg-amber-50 border-amber-300 text-amber-900'
          : 'bg-emerald-50 border-emerald-300 text-emerald-900'
      }`}>
        <div className="flex items-center gap-3">
          <div className={`p-2 rounded-lg ${
            criticalFaultsCount > 0
              ? 'bg-rose-100 text-rose-700'
              : warningFaultsCount > 0
              ? 'bg-amber-100 text-amber-700'
              : 'bg-emerald-100 text-emerald-700'
          }`}>
            {criticalFaultsCount > 0 ? (
              <AlertCircle className="w-5 h-5" />
            ) : warningFaultsCount > 0 ? (
              <AlertTriangle className="w-5 h-5" />
            ) : (
              <ShieldCheck className="w-5 h-5" />
            )}
          </div>
          <div>
            <div className="text-sm font-black uppercase tracking-wider">
              {criticalFaultsCount > 0
                ? 'SYSTEM ALERT: CRITICAL FAULT DETECTED'
                : warningFaultsCount > 0
                ? 'SYSTEM STATUS: OPERATIONAL WITH ADVISORY'
                : 'SYSTEM STATUS: FULLY NOMINAL — NO ACTIVE FAULTS'}
            </div>
            <p className="text-xs opacity-85 mt-0.5">
              {criticalFaultsCount > 0
                ? 'Inspect the fault details below and ensure hardware connections are secure.'
                : 'STM32 controller, ESP32 IoT gateway, and tracking sensors are functioning normally.'}
            </p>
          </div>
        </div>

        <div className="flex items-center gap-2 self-start sm:self-auto font-mono text-xs">
          <span className={`px-2.5 py-1 rounded-md font-bold ${
            criticalFaultsCount > 0
              ? 'bg-rose-200 text-rose-900'
              : 'bg-emerald-200/80 text-emerald-900'
          }`}>
            {criticalFaultsCount} Critical Faults
          </span>
          <span className="px-2.5 py-1 rounded-md bg-white/80 font-bold border border-slate-200 text-slate-700">
            Health Index: {ai.healthScore}%
          </span>
        </div>
      </div>

      {/* 2. In-House Edge AI Diagnostic Engine Showcase */}
      <div className="bg-gradient-to-br from-indigo-950 via-slate-900 to-indigo-900 rounded-xl p-5 border border-indigo-500/30 text-white shadow-md relative overflow-hidden">
        <div className="absolute top-0 right-0 w-64 h-64 bg-indigo-500/10 rounded-full blur-3xl pointer-events-none"></div>

        <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 mb-4 relative z-10">
          <div className="flex items-center gap-3">
            <div className="p-2.5 rounded-lg bg-indigo-600/40 text-indigo-300 border border-indigo-400/30 shadow-inner">
              <Brain className="w-5 h-5 text-indigo-300" />
            </div>
            <div>
              <div className="flex items-center gap-2">
                <h3 className="text-sm font-bold text-white tracking-wide">
                  POWER IQ In-House Edge AI Diagnostic Engine
                </h3>
                <span className="flex items-center gap-1 text-[10px] font-mono px-2 py-0.5 rounded-full bg-emerald-500/20 text-emerald-300 border border-emerald-400/40">
                  <Sparkles className="w-3 h-3 text-emerald-400" />
                  100% Offline Edge Capable
                </span>
              </div>
              <p className="text-xs text-indigo-200/70 mt-0.5">
                Custom Random Forest & Decision Tree trained on 1,500 physical telemetry samples (Zero Cloud / API Dependency)
              </p>
            </div>
          </div>

          <div className="flex items-center gap-2 self-start sm:self-auto font-mono text-xs">
            <div className="px-3 py-1.5 rounded-lg bg-indigo-900/60 border border-indigo-400/30 text-right">
              <div className="text-[10px] text-indigo-300/80 uppercase">AI Health Score</div>
              <div className="text-base font-black text-emerald-400 font-mono">{ai.healthScore}%</div>
            </div>
          </div>
        </div>

        {/* Live English AI Insight Box */}
        <div className="bg-slate-950/70 rounded-lg p-3.5 border border-indigo-400/20 relative z-10 font-sans">
          <div className="flex items-start gap-2">
            <span className="text-[10px] font-mono font-bold uppercase px-2 py-0.5 rounded bg-indigo-800/80 text-indigo-200 border border-indigo-600/40 mt-0.5 shrink-0">
              AI Insight
            </span>
            <p className="text-xs text-slate-200 leading-relaxed font-sans">
              {ai.aiInsightText}
            </p>
          </div>
        </div>

        {/* Technical Architecture Specs Bar */}
        <div className="grid grid-cols-2 sm:grid-cols-4 gap-2 mt-3 pt-3 border-t border-indigo-500/20 text-[11px] font-mono relative z-10 text-indigo-200/80">
          <div className="flex items-center gap-1.5">
            <Cpu className="w-3.5 h-3.5 text-indigo-400" />
            <span>Model: Random Forest (60 Trees)</span>
          </div>
          <div>Accuracy: <span className="text-emerald-400 font-bold">100.0%</span> Test Split</div>
          <div>Limits: <span className="text-cyan-300 font-bold">[-35.0°, +35.0°]</span> Locked</div>
          <div>Edge Latency: <span className="text-amber-300 font-bold">&lt; 5 μs</span> on STM32</div>
        </div>
      </div>

      {/* 3. Real-Time Subsystem Health Checks Grid */}
      <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs">
        <h3 className="text-sm font-bold text-slate-900 mb-3 flex items-center gap-2">
          <Server className="w-4 h-4 text-slate-600" />
          <span>Core Hardware & Sensor Diagnostic Checks</span>
        </h3>

        <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-3">
          {/* Check 1: IoT Gateway & UART Link */}
          <div className="p-3.5 bg-slate-50 rounded-lg border border-slate-200/70">
            <div className="flex items-center justify-between text-xs text-slate-500 mb-1">
              <span>Controller Link</span>
              <Radio className={`w-3.5 h-3.5 ${isHardwareOnline ? 'text-emerald-600' : 'text-rose-600'}`} />
            </div>
            <div className="flex items-center gap-2 mt-1">
              <span className={`w-2.5 h-2.5 rounded-full ${isHardwareOnline ? 'bg-emerald-500' : 'bg-rose-500'}`}></span>
              <span className="text-sm font-bold text-slate-900 font-mono">
                {isHardwareOnline ? 'UART ONLINE' : 'LINK TIMEOUT'}
              </span>
            </div>
            <div className="text-[11px] text-slate-500 mt-1 font-mono">
              STM32 ↔ ESP32 @ 115200 Baud
            </div>
          </div>

          {/* Check 2: Solar DC Bus & ACS712 Overcurrent */}
          <div className={`p-3.5 rounded-lg border ${
            isShortCircuit ? 'bg-rose-50 border-rose-300' : 'bg-slate-50 border-slate-200/70'
          }`}>
            <div className="flex items-center justify-between text-xs text-slate-500 mb-1">
              <span>Current & Bus Protection</span>
              <Zap className={`w-3.5 h-3.5 ${isShortCircuit ? 'text-rose-600' : 'text-slate-500'}`} />
            </div>
            <div className="flex items-center gap-2 mt-1">
              <span className={`w-2.5 h-2.5 rounded-full ${isShortCircuit ? 'bg-rose-500 animate-ping' : 'bg-emerald-500'}`}></span>
              <span className="text-sm font-bold text-slate-900 font-mono">
                {isShortCircuit ? 'SHORT / INRUSH' : 'NORMAL (< 5A)'}
              </span>
            </div>
            <div className="text-[11px] text-slate-500 mt-1 font-mono">
              ACS712 Sensor Protection
            </div>
          </div>

          {/* Check 3: Hall Effect Home Datum Sensor */}
          <div className={`p-3.5 rounded-lg border ${
            isHallUncalibrated ? 'bg-amber-50 border-amber-300' : 'bg-slate-50 border-slate-200/70'
          }`}>
            <div className="flex items-center justify-between text-xs text-slate-500 mb-1">
              <span>Limit / Home Datum</span>
              <Compass className={`w-3.5 h-3.5 ${isHallUncalibrated ? 'text-amber-600' : 'text-slate-500'}`} />
            </div>
            <div className="flex items-center gap-2 mt-1">
              <span className={`w-2.5 h-2.5 rounded-full ${isHallUncalibrated ? 'bg-amber-500' : 'bg-emerald-500'}`}></span>
              <span className="text-sm font-bold text-slate-900 font-mono">
                {isHallUncalibrated ? 'HOMING REQUIRED' : '0.0° HOMED'}
              </span>
            </div>
            <div className="text-[11px] text-slate-500 mt-1 font-mono">
              PB11 Hall Switch Datum
            </div>
          </div>

          {/* Check 4: Thermal Safety */}
          <div className={`p-3.5 rounded-lg border ${
            isOverheat ? 'bg-rose-50 border-rose-300' : 'bg-slate-50 border-slate-200/70'
          }`}>
            <div className="flex items-center justify-between text-xs text-slate-500 mb-1">
              <span>Driver / Ambient Thermal</span>
              <Flame className={`w-3.5 h-3.5 ${isOverheat ? 'text-rose-600' : 'text-slate-500'}`} />
            </div>
            <div className="flex items-center gap-2 mt-1">
              <span className={`w-2.5 h-2.5 rounded-full ${isOverheat ? 'bg-rose-500' : 'bg-emerald-500'}`}></span>
              <span className="text-sm font-bold text-slate-900 font-mono">
                {isOverheat ? 'OVERHEAT WARNING' : 'SAFE (< 50°C)'}
              </span>
            </div>
            <div className="text-[11px] text-slate-500 mt-1 font-mono">
              DHT11 Thermal Monitor
            </div>
          </div>
        </div>
      </div>

      {/* 3. Live System Faults & Incident Log */}
      <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs">
        <div className="flex items-center justify-between mb-3 pb-2 border-b border-slate-100">
          <div>
            <h3 className="text-sm font-bold text-slate-900">
              Live System Faults & Telemetry Incident Log
            </h3>
            <p className="text-xs text-slate-500">
              Real-time audit log of hardware notifications, MCU frames, and safety alerts
            </p>
          </div>
          <span className="text-xs font-mono font-bold bg-slate-100 text-slate-700 px-2 py-0.5 rounded">
            {alerts.length} Events Logged
          </span>
        </div>

        {alerts.length > 0 ? (
          <div className="space-y-2 max-h-64 overflow-y-auto pr-1">
            {alerts.map((alert) => {
              let badgeStyle = 'bg-blue-50 text-blue-800 border-blue-200';
              let icon = <Info className="w-4 h-4 text-blue-600 flex-shrink-0" />;

              if (alert.type === 'success') {
                badgeStyle = 'bg-emerald-50 text-emerald-800 border-emerald-200';
                icon = <CheckCircle2 className="w-4 h-4 text-emerald-600 flex-shrink-0" />;
              } else if (alert.type === 'warning') {
                badgeStyle = 'bg-amber-50 text-amber-900 border-amber-200';
                icon = <AlertTriangle className="w-4 h-4 text-amber-600 flex-shrink-0" />;
              } else if (alert.type === 'error') {
                badgeStyle = 'bg-rose-50 text-rose-900 border-rose-200';
                icon = <AlertCircle className="w-4 h-4 text-rose-600 flex-shrink-0" />;
              }

              return (
                <div
                  key={alert.id}
                  className={`p-3 rounded-lg border flex items-start gap-3 text-xs transition ${badgeStyle}`}
                >
                  <div className="mt-0.5">{icon}</div>
                  <div className="flex-1 min-w-0">
                    <div className="flex items-center justify-between gap-2">
                      <span className="font-bold text-slate-900">{alert.title}</span>
                      <span className="text-[10px] font-mono text-slate-500 flex items-center gap-1">
                        <Clock className="w-3 h-3" />
                        {alert.time}
                      </span>
                    </div>
                    <p className="text-slate-600 text-[11px] mt-0.5">{alert.message}</p>
                  </div>
                  <span className="text-[9px] font-mono font-semibold bg-white px-2 py-0.5 rounded border border-slate-200 text-slate-700 hidden sm:inline">
                    {alert.component}
                  </span>
                </div>
              );
            })}
          </div>
        ) : (
          <div className="py-6 text-center text-xs text-slate-400">
            No active fault incidents recorded. System running smoothly.
          </div>
        )}
      </div>
    </div>
  );
};
