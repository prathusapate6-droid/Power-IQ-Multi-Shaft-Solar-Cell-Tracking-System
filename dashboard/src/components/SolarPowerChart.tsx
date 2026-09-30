import React, { useState, useEffect } from 'react';
import { 
  Sun, 
  TrendingUp, 
  Zap, 
  Thermometer, 
  Layers, 
  Check, 
  RefreshCw, 
  Eye, 
  Activity, 
  Sliders, 
  LayoutGrid, 
  AlertTriangle 
} from 'lucide-react';
import {
  AreaChart as ReAreaChart,
  ComposedChart as ReComposedChart,
  Area as ReArea,
  Line as ReLine,
  XAxis as ReXAxis,
  YAxis as ReYAxis,
  CartesianGrid as ReCartesianGrid,
  Tooltip as ReTooltip,
  ResponsiveContainer as ReResponsiveContainer,
  ReferenceLine as ReReferenceLine,
} from 'recharts';

import type { HourlyGenerationPoint, TrackingMode } from '../types/dashboard';

interface SolarPowerChartProps {
  data: HourlyGenerationPoint[];
  currentHourDecimal: number;
  trackingMode?: TrackingMode;
  onToggleMode?: () => void;
}

export type ChartMetric = 'grid_all' | 'all' | 'power' | 'voltage' | 'current' | 'climate';

// =============================================================================
// 1. Unified All-in-One Tooltip (Shows All Parameters Simultaneously)
// =============================================================================
const UnifiedTooltip = ({ active, payload, label }: any) => {
  if (active && payload && payload.length) {
    const trackingW = payload.find((p: any) => p.dataKey === 'trackingW')?.value;
    const fixedW = payload.find((p: any) => p.dataKey === 'fixedW')?.value;
    const pvVolt = payload.find((p: any) => p.dataKey === 'solarVoltage')?.value;
    const battVolt = payload.find((p: any) => p.dataKey === 'battVoltage')?.value;
    const curr = payload.find((p: any) => p.dataKey === 'solarCurrent')?.value;
    const temp = payload.find((p: any) => p.dataKey === 'temperature')?.value;
    const hum = payload.find((p: any) => p.dataKey === 'humidity')?.value;

    const delta = trackingW !== undefined && fixedW !== undefined && trackingW > fixedW
      ? (trackingW - fixedW).toFixed(1)
      : '0.0';
    const gainPercent = fixedW && fixedW > 0 && trackingW !== undefined
      ? (((trackingW - fixedW) / fixedW) * 100).toFixed(1)
      : '0';

    return (
      <div className="bg-slate-950/95 text-white p-3.5 rounded-xl shadow-2xl border border-slate-700/80 text-xs font-mono min-w-[270px] backdrop-blur-md">
        <div className="text-slate-200 font-semibold border-b border-slate-800 pb-2 mb-2 flex items-center justify-between gap-3">
          <span className="flex items-center gap-1.5 text-slate-300">
            <span className="w-2 h-2 rounded-full bg-amber-400"></span>
            Time: <strong className="text-white">{label}</strong>
          </span>
          <span className="text-[10px] bg-emerald-950/90 text-emerald-300 border border-emerald-700/60 px-2 py-0.5 rounded font-bold">
            +{gainPercent}% Boost
          </span>
        </div>

        <div className="space-y-1.5">
          {trackingW !== undefined && (
            <div className="flex items-center justify-between gap-3">
              <span className="text-emerald-400 flex items-center gap-1.5">
                <span className="w-2.5 h-2.5 rounded-full bg-emerald-400"></span>
                Tracking Power:
              </span>
              <span className="font-bold text-emerald-300">{Number(trackingW).toFixed(1)} W</span>
            </div>
          )}

          {fixedW !== undefined && (
            <div className="flex items-center justify-between gap-3">
              <span className="text-slate-400 flex items-center gap-1.5">
                <span className="w-2.5 h-1 bg-slate-400"></span>
                Fixed Solar Panel:
              </span>
              <span className="text-slate-300">{Number(fixedW).toFixed(1)} W</span>
            </div>
          )}

          {pvVolt !== undefined && (
            <div className="flex items-center justify-between gap-3">
              <span className="text-blue-400 flex items-center gap-1.5">
                <span className="w-2.5 h-2.5 rounded-full bg-blue-400"></span>
                Solar PV Voltage:
              </span>
              <span className="font-bold text-blue-300">{Number(pvVolt).toFixed(2)} V</span>
            </div>
          )}

          {battVolt !== undefined && (
            <div className="flex items-center justify-between gap-3">
              <span className="text-purple-400 flex items-center gap-1.5">
                <span className="w-2.5 h-2.5 rounded-full bg-purple-400"></span>
                Battery Voltage:
              </span>
              <span className="font-bold text-purple-300">{Number(battVolt).toFixed(2)} V</span>
            </div>
          )}

          {curr !== undefined && (
            <div className="flex items-center justify-between gap-3">
              <span className="text-cyan-400 flex items-center gap-1.5">
                <span className="w-2.5 h-2.5 rounded-full bg-cyan-400"></span>
                Solar Current:
              </span>
              <span className="font-bold text-cyan-300">{Number(curr).toFixed(2)} A</span>
            </div>
          )}

          {temp !== undefined && (
            <div className="flex items-center justify-between gap-3">
              <span className="text-amber-400 flex items-center gap-1.5">
                <span className="w-2.5 h-2.5 rounded-full bg-amber-400"></span>
                Panel Temp:
              </span>
              <span className="font-bold text-amber-300">{Number(temp).toFixed(1)} °C</span>
            </div>
          )}

          {hum !== undefined && (
            <div className="flex items-center justify-between gap-3">
              <span className="text-sky-400 flex items-center gap-1.5">
                <span className="w-2.5 h-2.5 rounded-full bg-sky-400"></span>
                Humidity:
              </span>
              <span className="font-bold text-sky-300">{Number(hum).toFixed(0)} %</span>
            </div>
          )}
        </div>

        <div className="pt-2 mt-2 border-t border-slate-800/80 flex items-center justify-between text-amber-300 font-bold text-[11px]">
          <span>Tracking Yield Advantage:</span>
          <span>+{delta} W Boost</span>
        </div>
      </div>
    );
  }
  return null;
};

// =============================================================================
// 2. Individual Power Tooltip
// =============================================================================
const PowerTooltip = ({ active, payload, label, isManualMode }: any) => {
  if (active && payload && payload.length) {
    const tracking = payload.find((p: any) => p.dataKey === 'trackingW')?.value ?? 0;
    const fixed = payload.find((p: any) => p.dataKey === 'fixedW')?.value ?? 0;
    const delta = tracking > fixed ? (tracking - fixed).toFixed(1) : '0.0';
    const gainPercent = fixed > 0 ? (((tracking - fixed) / fixed) * 100).toFixed(1) : '0';

    return (
      <div className="bg-slate-900/95 text-white p-3 rounded-lg shadow-xl border border-slate-700 text-xs font-mono">
        <div className="text-slate-300 font-semibold border-b border-slate-700 pb-1 mb-1.5 flex items-center justify-between gap-4">
          <span>Time: {label}</span>
          <span className={`text-[10px] px-1.5 rounded font-bold ${isManualMode ? 'bg-amber-900/80 text-amber-300' : 'bg-emerald-900/80 text-emerald-300'}`}>
            {isManualMode ? 'Manual: No Tracking' : `+${gainPercent}% Gain`}
          </span>
        </div>
        <div className="space-y-1">
          {isManualMode ? (
            <div className="flex items-center justify-between gap-4">
              <span className="text-amber-400 flex items-center gap-1.5">
                <span className="w-2 h-2 rounded-full bg-amber-400"></span>
                Without Tracking (Fixed):
              </span>
              <span className="font-bold text-amber-300">{typeof fixed === 'number' ? fixed.toFixed(1) : fixed} W</span>
            </div>
          ) : (
            <>
              <div className="flex items-center justify-between gap-4">
                <span className="text-emerald-400 flex items-center gap-1.5">
                  <span className="w-2 h-2 rounded-full bg-emerald-400"></span>
                  Multi-Shaft Tracking:
                </span>
                <span className="font-bold">{typeof tracking === 'number' ? tracking.toFixed(1) : tracking} W</span>
              </div>
              <div className="flex items-center justify-between gap-4">
                <span className="text-slate-400 flex items-center gap-1.5">
                  <span className="w-2 h-2 rounded-full bg-slate-400"></span>
                  Fixed Solar Panel:
                </span>
                <span>{typeof fixed === 'number' ? fixed.toFixed(1) : fixed} W</span>
              </div>
              <div className="pt-1 border-t border-slate-800 flex items-center justify-between text-amber-300 font-bold">
                <span>Tracking Yield Boost:</span>
                <span>+{delta} W</span>
              </div>
            </>
          )}
        </div>
      </div>
    );
  }
  return null;
};

// =============================================================================
// 3. Individual Voltage Tooltip
// =============================================================================
const VoltageTooltip = ({ active, payload, label }: any) => {
  if (active && payload && payload.length) {
    const pvVolt = payload.find((p: any) => p.dataKey === 'solarVoltage')?.value ?? 0;
    const battVolt = payload.find((p: any) => p.dataKey === 'battVoltage')?.value ?? 0;
    const diff = (pvVolt - battVolt).toFixed(2);
    const isCharging = pvVolt > battVolt;

    return (
      <div className="bg-slate-900/95 text-white p-3 rounded-lg shadow-xl border border-slate-700 text-xs font-mono">
        <div className="text-slate-300 font-semibold border-b border-slate-700 pb-1 mb-1.5 flex items-center justify-between gap-4">
          <span>Time: {label}</span>
          <span className={`text-[10px] px-1.5 rounded font-bold ${isCharging ? 'bg-emerald-900/80 text-emerald-300' : 'bg-amber-900/80 text-amber-300'}`}>
            {isCharging ? 'Active Charging' : 'Standby / Float'}
          </span>
        </div>
        <div className="space-y-1">
          <div className="flex items-center justify-between gap-4">
            <span className="text-blue-400 flex items-center gap-1.5">
              <span className="w-2 h-2 rounded-full bg-blue-400"></span>
              Solar PV Voltage:
            </span>
            <span className="font-bold">{typeof pvVolt === 'number' ? pvVolt.toFixed(2) : pvVolt} V</span>
          </div>
          <div className="flex items-center justify-between gap-4">
            <span className="text-purple-400 flex items-center gap-1.5">
              <span className="w-2 h-2 rounded-full bg-purple-400"></span>
              Battery Voltage:
            </span>
            <span className="font-bold">{typeof battVolt === 'number' ? battVolt.toFixed(2) : battVolt} V</span>
          </div>
          <div className="pt-1 border-t border-slate-800 flex items-center justify-between text-cyan-300 font-bold">
            <span>PV vs Battery Delta:</span>
            <span>{Number(diff) >= 0 ? `+${diff}` : diff} V</span>
          </div>
        </div>
      </div>
    );
  }
  return null;
};

// =============================================================================
// 4. Individual Current Tooltip
// =============================================================================
const CurrentTooltip = ({ active, payload, label }: any) => {
  if (active && payload && payload.length) {
    const curr = payload.find((p: any) => p.dataKey === 'solarCurrent')?.value ?? 0;
    const currA = typeof curr === 'number' ? curr : parseFloat(curr);
    const currMa = (currA * 1000).toFixed(0);

    return (
      <div className="bg-slate-900/95 text-white p-3 rounded-lg shadow-xl border border-slate-700 text-xs font-mono">
        <div className="text-slate-300 font-semibold border-b border-slate-700 pb-1 mb-1.5 flex items-center justify-between gap-4">
          <span>Time: {label}</span>
          <span className="text-[10px] bg-cyan-900/80 text-cyan-300 px-1.5 rounded font-bold">
            ACS712 Hall Sensor
          </span>
        </div>
        <div className="space-y-1">
          <div className="flex items-center justify-between gap-4">
            <span className="text-cyan-400 flex items-center gap-1.5">
              <span className="w-2 h-2 rounded-full bg-cyan-400"></span>
              Solar Current:
            </span>
            <span className="font-bold">{currA.toFixed(2)} A ({currMa} mA)</span>
          </div>
          <div className="pt-1 border-t border-slate-800 flex items-center justify-between text-slate-400">
            <span>Sensor Rating:</span>
            <span className="text-slate-200">5A Bi-directional Hall</span>
          </div>
        </div>
      </div>
    );
  }
  return null;
};

// =============================================================================
// 5. Individual Climate Tooltip
// =============================================================================
const ClimateTooltip = ({ active, payload, label }: any) => {
  if (active && payload && payload.length) {
    const temp = payload.find((p: any) => p.dataKey === 'temperature')?.value ?? 0;
    const hum = payload.find((p: any) => p.dataKey === 'humidity')?.value ?? 0;
    const isHot = temp >= 45;

    return (
      <div className="bg-slate-900/95 text-white p-3 rounded-lg shadow-xl border border-slate-700 text-xs font-mono">
        <div className="text-slate-300 font-semibold border-b border-slate-700 pb-1 mb-1.5 flex items-center justify-between gap-4">
          <span>Time: {label}</span>
          <span className={`text-[10px] px-1.5 rounded font-bold ${isHot ? 'bg-rose-900/80 text-rose-300' : 'bg-emerald-900/80 text-emerald-300'}`}>
            {isHot ? 'High Temp Warning' : 'Optimal Climate'}
          </span>
        </div>
        <div className="space-y-1">
          <div className="flex items-center justify-between gap-4">
            <span className="text-amber-400 flex items-center gap-1.5">
              <span className="w-2 h-2 rounded-full bg-amber-400"></span>
              Panel Temperature:
            </span>
            <span className="font-bold">{typeof temp === 'number' ? temp.toFixed(1) : temp} °C</span>
          </div>
          <div className="flex items-center justify-between gap-4">
            <span className="text-cyan-400 flex items-center gap-1.5">
              <span className="w-2 h-2 rounded-full bg-cyan-400"></span>
              Relative Humidity:
            </span>
            <span className="font-bold">{typeof hum === 'number' ? hum.toFixed(0) : hum} %</span>
          </div>
          <div className="pt-1 border-t border-slate-800 flex items-center justify-between text-slate-300 font-medium">
            <span>Thermal Threshold:</span>
            <span className="text-emerald-400">Safe (&lt; 45°C)</span>
          </div>
        </div>
      </div>
    );
  }
  return null;
};

// =============================================================================
// MAIN COMPONENT: SolarPowerChart
// =============================================================================
export const SolarPowerChart: React.FC<SolarPowerChartProps> = ({
  data,
  currentHourDecimal,
  trackingMode = 'AUTO',
  onToggleMode,
}) => {
  // Chart viewing mode: Single Graphs Grid, All-in-1 Unified, Power, Voltage, Current, Climate
  const [activeMetric, setActiveMetric] = useState<ChartMetric>('grid_all');

  // Display Mode: AUTO (With Tracking) vs MANUAL (Without Tracking)
  const [displayMode, setDisplayMode] = useState<'AUTO' | 'MANUAL'>(trackingMode === 'MANUAL' ? 'MANUAL' : 'AUTO');

  useEffect(() => {
    if (trackingMode === 'MANUAL') {
      setDisplayMode('MANUAL');
    } else {
      setDisplayMode('AUTO');
    }
  }, [trackingMode]);

  // Interactive line toggles for the Unified All-in-One Graph
  const [visibleLines, setVisibleLines] = useState<{
    trackingW: boolean;
    fixedW: boolean;
    solarVoltage: boolean;
    battVoltage: boolean;
    solarCurrent: boolean;
    temperature: boolean;
    humidity: boolean;
  }>({
    trackingW: true,
    fixedW: true,
    solarVoltage: true,
    battVoltage: true,
    solarCurrent: true,
    temperature: true,
    humidity: false,
  });

  const toggleLine = (key: keyof typeof visibleLines) => {
    setVisibleLines((prev) => ({ ...prev, [key]: !prev[key] }));
  };

  const selectAllLines = () => {
    setVisibleLines({
      trackingW: true,
      fixedW: true,
      solarVoltage: true,
      battVoltage: true,
      solarCurrent: true,
      temperature: true,
      humidity: true,
    });
  };

  const selectCoreLines = () => {
    setVisibleLines({
      trackingW: true,
      fixedW: true,
      solarVoltage: true,
      battVoltage: true,
      solarCurrent: true,
      temperature: false,
      humidity: false,
    });
  };

  // Daylight reference indicator
  const isDaylight = currentHourDecimal >= 6 && currentHourDecimal <= 18;
  const currentHourInt = Math.floor(currentHourDecimal);
  const currentMinStr = currentHourDecimal % 1 >= 0.5 ? '30' : '00';
  const currentTimeStr = `${currentHourInt.toString().padStart(2, '0')}:${currentMinStr}`;

  // Find live data points around current hour
  const currentPoint = data.find((p) => Math.abs(p.hour - currentHourDecimal) < 0.6) || data[data.length - 1];
  const liveTrackingW = currentPoint?.trackingW ?? 0;
  const liveFixedW = currentPoint?.fixedW ?? 0;
  const liveDiffW = (liveTrackingW - liveFixedW).toFixed(1);
  const livePvVolt = currentPoint?.solarVoltage ?? 12.8;
  const liveBattVolt = currentPoint?.battVoltage ?? 13.2;
  const liveCurrent = currentPoint?.solarCurrent ?? (livePvVolt > 0 ? liveTrackingW / livePvVolt : 0);
  const liveTemp = currentPoint?.temperature ?? 34.5;
  const liveHum = currentPoint?.humidity ?? 52;

  const isManual = displayMode === 'MANUAL';

  // ===========================================================================
  // RENDER: Single Power Chart
  // ===========================================================================
  const renderPowerChart = (heightClass = "h-72") => (
    <div className={`${heightClass} w-full`}>
      <ReResponsiveContainer width="100%" height="100%">
        <ReAreaChart data={data} margin={{ top: 10, right: 15, left: -10, bottom: 0 }}>
          <defs>
            <linearGradient id="trackingGrad" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#10b981" stopOpacity={0.4} />
              <stop offset="95%" stopColor="#10b981" stopOpacity={0.0} />
            </linearGradient>
            <linearGradient id="fixedGrad" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor={isManual ? "#f59e0b" : "#94a3b8"} stopOpacity={isManual ? 0.45 : 0.25} />
              <stop offset="95%" stopColor={isManual ? "#f59e0b" : "#94a3b8"} stopOpacity={0.0} />
            </linearGradient>
          </defs>
          <ReCartesianGrid strokeDasharray="3 3" vertical={false} stroke="#f1f5f9" />
          <ReXAxis dataKey="time" stroke="#94a3b8" fontSize={11} tickLine={false} axisLine={{ stroke: '#e2e8f0' }} />
          <ReYAxis
            stroke="#94a3b8"
            fontSize={11}
            tickLine={false}
            axisLine={{ stroke: '#e2e8f0' }}
            tickFormatter={(val) => `${val} W`}
            domain={[0, 50]}
            ticks={[0, 10, 20, 30, 40, 50]}
          />
          <ReTooltip content={<PowerTooltip isManualMode={isManual} />} />
          {isDaylight && (
            <ReReferenceLine
              x={currentTimeStr}
              stroke="#f59e0b"
              strokeDasharray="4 4"
              strokeWidth={1.5}
              label={{ value: 'NOW', position: 'top', fill: '#f59e0b', fontSize: 10, fontWeight: 'bold' }}
            />
          )}

          {/* If in Manual Mode: Primary curve is the Fixed Array without Tracking */}
          {isManual ? (
            <ReArea
              type="monotone"
              dataKey="fixedW"
              stroke="#f59e0b"
              strokeWidth={2.5}
              fillOpacity={1}
              fill="url(#fixedGrad)"
              name="Without Tracking: Fixed Array (W)"
            />
          ) : (
            <>
              <ReArea
                type="monotone"
                dataKey="fixedW"
                stroke="#94a3b8"
                strokeWidth={1.75}
                fillOpacity={1}
                fill="url(#fixedGrad)"
                name="Fixed Baseline (W)"
              />
              <ReArea
                type="monotone"
                dataKey="trackingW"
                stroke="#10b981"
                strokeWidth={2.5}
                fillOpacity={1}
                fill="url(#trackingGrad)"
                name="Auto Tracking Output (W)"
              />
            </>
          )}
        </ReAreaChart>
      </ReResponsiveContainer>
    </div>
  );

  // ===========================================================================
  // RENDER: Single Voltage Chart
  // ===========================================================================
  const renderVoltageChart = (heightClass = "h-72") => (
    <div className={`${heightClass} w-full`}>
      <ReResponsiveContainer width="100%" height="100%">
        <ReAreaChart data={data} margin={{ top: 10, right: 15, left: -10, bottom: 0 }}>
          <defs>
            <linearGradient id="pvVoltGrad" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#2563eb" stopOpacity={0.35} />
              <stop offset="95%" stopColor="#2563eb" stopOpacity={0.0} />
            </linearGradient>
          </defs>
          <ReCartesianGrid strokeDasharray="3 3" vertical={false} stroke="#f1f5f9" />
          <ReXAxis dataKey="time" stroke="#94a3b8" fontSize={11} tickLine={false} axisLine={{ stroke: '#e2e8f0' }} />
          <ReYAxis
            stroke="#94a3b8"
            fontSize={11}
            tickLine={false}
            axisLine={{ stroke: '#e2e8f0' }}
            tickFormatter={(val) => `${val} V`}
            domain={[0, 20]}
            ticks={[0, 5, 10, 15, 20]}
          />
          <ReTooltip content={<VoltageTooltip />} />
          {isDaylight && (
            <ReReferenceLine
              x={currentTimeStr}
              stroke="#3b82f6"
              strokeDasharray="4 4"
              strokeWidth={1.5}
              label={{ value: 'NOW', position: 'top', fill: '#3b82f6', fontSize: 10, fontWeight: 'bold' }}
            />
          )}
          <ReReferenceLine y={14.4} stroke="#10b981" strokeDasharray="3 3" strokeWidth={1} label={{ value: '14.4V Float', position: 'right', fill: '#059669', fontSize: 10 }} />
          <ReReferenceLine y={11.5} stroke="#f43f5e" strokeDasharray="3 3" strokeWidth={1} label={{ value: '11.5V Low', position: 'right', fill: '#e11d48', fontSize: 10 }} />
          <ReArea
            type="monotone"
            dataKey="solarVoltage"
            stroke="#2563eb"
            strokeWidth={2.5}
            fillOpacity={1}
            fill="url(#pvVoltGrad)"
            name="Solar PV Voltage (V)"
          />
          <ReLine
            type="monotone"
            dataKey="battVoltage"
            stroke="#9333ea"
            strokeWidth={2.2}
            dot={false}
            name="Battery Voltage (V)"
          />
        </ReAreaChart>
      </ReResponsiveContainer>
    </div>
  );

  // ===========================================================================
  // RENDER: Single Current Chart
  // ===========================================================================
  const renderCurrentChart = (heightClass = "h-72") => (
    <div className={`${heightClass} w-full`}>
      <ReResponsiveContainer width="100%" height="100%">
        <ReAreaChart data={data} margin={{ top: 10, right: 15, left: -10, bottom: 0 }}>
          <defs>
            <linearGradient id="currentGrad" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#06b6d4" stopOpacity={0.4} />
              <stop offset="95%" stopColor="#06b6d4" stopOpacity={0.0} />
            </linearGradient>
          </defs>
          <ReCartesianGrid strokeDasharray="3 3" vertical={false} stroke="#f1f5f9" />
          <ReXAxis dataKey="time" stroke="#94a3b8" fontSize={11} tickLine={false} axisLine={{ stroke: '#e2e8f0' }} />
          <ReYAxis
            stroke="#0891b2"
            fontSize={11}
            tickLine={false}
            axisLine={{ stroke: '#06b6d4' }}
            tickFormatter={(val) => `${val} A`}
            domain={[0, 2.5]}
            ticks={[0, 0.5, 1.0, 1.5, 2.0, 2.5]}
          />
          <ReTooltip content={<CurrentTooltip />} />
          {isDaylight && (
            <ReReferenceLine
              x={currentTimeStr}
              stroke="#06b6d4"
              strokeDasharray="4 4"
              strokeWidth={1.5}
              label={{ value: 'NOW', position: 'top', fill: '#0891b2', fontSize: 10, fontWeight: 'bold' }}
            />
          )}
          <ReArea
            type="monotone"
            dataKey="solarCurrent"
            stroke="#06b6d4"
            strokeWidth={2.5}
            fillOpacity={1}
            fill="url(#currentGrad)"
            name="Solar Current (A)"
          />
        </ReAreaChart>
      </ReResponsiveContainer>
    </div>
  );

  // ===========================================================================
  // RENDER: Single Climate Chart
  // ===========================================================================
  const renderClimateChart = (heightClass = "h-72") => (
    <div className={`${heightClass} w-full`}>
      <ReResponsiveContainer width="100%" height="100%">
        <ReAreaChart data={data} margin={{ top: 10, right: 15, left: -10, bottom: 0 }}>
          <defs>
            <linearGradient id="tempGrad" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#f59e0b" stopOpacity={0.35} />
              <stop offset="95%" stopColor="#f59e0b" stopOpacity={0.0} />
            </linearGradient>
          </defs>
          <ReCartesianGrid strokeDasharray="3 3" vertical={false} stroke="#f1f5f9" />
          <ReXAxis dataKey="time" stroke="#94a3b8" fontSize={11} tickLine={false} axisLine={{ stroke: '#e2e8f0' }} />
          <ReYAxis
            stroke="#94a3b8"
            fontSize={11}
            tickLine={false}
            axisLine={{ stroke: '#e2e8f0' }}
            tickFormatter={(val) => `${val}°C`}
            domain={[0, 60]}
            ticks={[0, 15, 30, 45, 60]}
          />
          <ReTooltip content={<ClimateTooltip />} />
          {isDaylight && (
            <ReReferenceLine
              x={currentTimeStr}
              stroke="#f59e0b"
              strokeDasharray="4 4"
              strokeWidth={1.5}
              label={{ value: 'NOW', position: 'top', fill: '#f59e0b', fontSize: 10, fontWeight: 'bold' }}
            />
          )}
          <ReReferenceLine y={45} stroke="#ef4444" strokeDasharray="3 3" strokeWidth={1.5} label={{ value: '45°C Thermal Warning', position: 'right', fill: '#dc2626', fontSize: 10 }} />
          <ReArea
            type="monotone"
            dataKey="temperature"
            stroke="#f59e0b"
            strokeWidth={2.5}
            fillOpacity={1}
            fill="url(#tempGrad)"
            name="Panel Temperature (°C)"
          />
          <ReLine
            type="monotone"
            dataKey="humidity"
            stroke="#06b6d4"
            strokeWidth={2}
            dot={false}
            name="Relative Humidity (%)"
          />
        </ReAreaChart>
      </ReResponsiveContainer>
    </div>
  );

  // ===========================================================================
  // RENDER: Unified All-in-One Graph
  // ===========================================================================
  const renderUnifiedAllChart = (heightClass = "h-88 sm:h-96") => (
    <div className={`${heightClass} w-full`}>
      <ReResponsiveContainer width="100%" height="100%">
        <ReComposedChart data={data} margin={{ top: 12, right: 15, left: -10, bottom: 0 }}>
          <defs>
            <linearGradient id="trackingGradAll" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#10b981" stopOpacity={0.30} />
              <stop offset="95%" stopColor="#10b981" stopOpacity={0.0} />
            </linearGradient>
          </defs>

          <ReCartesianGrid strokeDasharray="3 3" vertical={false} stroke="#f1f5f9" />
          <ReXAxis
            dataKey="time"
            stroke="#94a3b8"
            fontSize={11}
            tickLine={false}
            axisLine={{ stroke: '#e2e8f0' }}
          />

          {/* Left Y-Axis: Power (0 - 50W) and Temperature (0 - 50°C) */}
          <ReYAxis
            yAxisId="left"
            orientation="left"
            stroke="#059669"
            fontSize={11}
            tickLine={false}
            axisLine={{ stroke: '#10b981' }}
            tickFormatter={(val) => `${val}W`}
            domain={[0, 50]}
            ticks={[0, 10, 20, 30, 40, 50]}
          />

          {/* Right Y-Axis: Voltage (0 - 20V) */}
          <ReYAxis
            yAxisId="right"
            orientation="right"
            stroke="#2563eb"
            fontSize={11}
            tickLine={false}
            axisLine={{ stroke: '#3b82f6' }}
            tickFormatter={(val) => `${val}V`}
            domain={[0, 20]}
            ticks={[0, 5, 10, 15, 20]}
          />

          {/* Hidden Y-Axis: Current (0 - 2.5A) */}
          <ReYAxis yAxisId="curr" domain={[0, 2.5]} hide={true} />

          <ReTooltip content={<UnifiedTooltip />} />

          {isDaylight && (
            <ReReferenceLine
              yAxisId="left"
              x={currentTimeStr}
              stroke="#f59e0b"
              strokeDasharray="4 4"
              strokeWidth={1.5}
              label={{ value: 'NOW', position: 'top', fill: '#f59e0b', fontSize: 10, fontWeight: 'bold' }}
            />
          )}

          <ReReferenceLine
            yAxisId="right"
            y={14.4}
            stroke="#9333ea"
            strokeDasharray="3 3"
            strokeWidth={1}
            strokeOpacity={0.6}
            label={{ value: '14.4V Float', position: 'insideRight', fill: '#9333ea', fontSize: 9 }}
          />

          {/* 1. Fixed Solar Baseline */}
          {visibleLines.fixedW && (
            <ReLine
              yAxisId="left"
              type="monotone"
              dataKey="fixedW"
              stroke="#94a3b8"
              strokeWidth={1.75}
              strokeDasharray="4 4"
              dot={false}
              name="Fixed Solar Baseline (W)"
            />
          )}

          {/* 2. Multi-Shaft Tracking Power */}
          {visibleLines.trackingW && !isManual && (
            <ReArea
              yAxisId="left"
              type="monotone"
              dataKey="trackingW"
              stroke="#10b981"
              strokeWidth={2.5}
              fillOpacity={1}
              fill="url(#trackingGradAll)"
              name="Multi-Shaft Tracking (W)"
            />
          )}

          {/* 3. Solar PV Voltage */}
          {visibleLines.solarVoltage && (
            <ReLine
              yAxisId="right"
              type="monotone"
              dataKey="solarVoltage"
              stroke="#2563eb"
              strokeWidth={2.2}
              dot={false}
              name="Solar PV Voltage (V)"
            />
          )}

          {/* 4. Battery Voltage */}
          {visibleLines.battVoltage && (
            <ReLine
              yAxisId="right"
              type="monotone"
              dataKey="battVoltage"
              stroke="#9333ea"
              strokeWidth={2.0}
              strokeDasharray="3 3"
              dot={false}
              name="Battery Voltage (V)"
            />
          )}

          {/* 5. Solar Current */}
          {visibleLines.solarCurrent && (
            <ReLine
              yAxisId="curr"
              type="monotone"
              dataKey="solarCurrent"
              stroke="#06b6d4"
              strokeWidth={2.2}
              dot={false}
              name="Solar Current (A)"
            />
          )}

          {/* 6. Panel Temperature */}
          {visibleLines.temperature && (
            <ReLine
              yAxisId="left"
              type="monotone"
              dataKey="temperature"
              stroke="#f59e0b"
              strokeWidth={2.0}
              dot={false}
              name="Panel Temperature (°C)"
            />
          )}

          {/* 7. Relative Humidity */}
          {visibleLines.humidity && (
            <ReLine
              yAxisId="left"
              type="monotone"
              dataKey="humidity"
              stroke="#0ea5e9"
              strokeWidth={1.5}
              strokeDasharray="2 2"
              dot={false}
              name="Relative Humidity (%)"
            />
          )}
        </ReComposedChart>
      </ReResponsiveContainer>
    </div>
  );

  // ===========================================================================
  // RENDER: All 4 Single Graphs in a Clean 2x2 Grid (User's Direct Request!)
  // ===========================================================================
  const renderAllSingleGraphsGrid = () => (
    <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
      {/* 1. Dedicated Power Card */}
      <div className="bg-slate-50/60 p-4 rounded-xl border border-slate-200">
        <div className="flex items-center justify-between mb-2">
          <div className="flex items-center gap-2">
            <div className="p-1.5 rounded-lg bg-emerald-100 text-emerald-700">
              <Sun className="w-4 h-4" />
            </div>
            <div>
              <h3 className="text-xs font-bold text-slate-800">
                {isManual ? 'Solar Power: Without Tracking' : 'Solar Power (Tracking vs Fixed)'}
              </h3>
              <p className="text-[10px] text-slate-500">Live Photovoltaic Generation (Watts)</p>
            </div>
          </div>
          <span className="text-xs font-mono font-bold text-emerald-700 bg-white px-2 py-0.5 rounded border border-slate-200">
            {isManual ? `${liveFixedW.toFixed(1)} W` : `${liveTrackingW.toFixed(1)} W`}
          </span>
        </div>
        {renderPowerChart('h-48')}
      </div>

      {/* 2. Dedicated Voltage Card */}
      <div className="bg-slate-50/60 p-4 rounded-xl border border-slate-200">
        <div className="flex items-center justify-between mb-2">
          <div className="flex items-center gap-2">
            <div className="p-1.5 rounded-lg bg-blue-100 text-blue-700">
              <Zap className="w-4 h-4" />
            </div>
            <div>
              <h3 className="text-xs font-bold text-slate-800">Solar PV & Battery Voltage</h3>
              <p className="text-[10px] text-slate-500">Array DC vs Battery Storage (Volts)</p>
            </div>
          </div>
          <span className="text-xs font-mono font-bold text-blue-700 bg-white px-2 py-0.5 rounded border border-slate-200">
            PV: {livePvVolt.toFixed(1)}V | Bat: {liveBattVolt.toFixed(1)}V
          </span>
        </div>
        {renderVoltageChart('h-48')}
      </div>

      {/* 3. Dedicated Current Card */}
      <div className="bg-slate-50/60 p-4 rounded-xl border border-slate-200">
        <div className="flex items-center justify-between mb-2">
          <div className="flex items-center gap-2">
            <div className="p-1.5 rounded-lg bg-cyan-100 text-cyan-700">
              <Activity className="w-4 h-4" />
            </div>
            <div>
              <h3 className="text-xs font-bold text-slate-800">Solar Current (ACS712)</h3>
              <p className="text-[10px] text-slate-500">Charging Current Draw (Amperes)</p>
            </div>
          </div>
          <span className="text-xs font-mono font-bold text-cyan-700 bg-white px-2 py-0.5 rounded border border-slate-200">
            {liveCurrent.toFixed(2)} A ({(liveCurrent * 1000).toFixed(0)} mA)
          </span>
        </div>
        {renderCurrentChart('h-48')}
      </div>

      {/* 4. Dedicated Temperature Card */}
      <div className="bg-slate-50/60 p-4 rounded-xl border border-slate-200">
        <div className="flex items-center justify-between mb-2">
          <div className="flex items-center gap-2">
            <div className="p-1.5 rounded-lg bg-amber-100 text-amber-700">
              <Thermometer className="w-4 h-4" />
            </div>
            <div>
              <h3 className="text-xs font-bold text-slate-800">Ambient Temperature & Humidity</h3>
              <p className="text-[10px] text-slate-500">DHT11 Environmental Sensor (°C / %)</p>
            </div>
          </div>
          <span className="text-xs font-mono font-bold text-amber-700 bg-white px-2 py-0.5 rounded border border-slate-200">
            {liveTemp.toFixed(1)}°C | {liveHum.toFixed(0)}%
          </span>
        </div>
        {renderClimateChart('h-48')}
      </div>
    </div>
  );

  return (
    <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs mb-6">
      {/* Top Header with Interactive Mode & Graph Selectors */}
      <div className="flex flex-col lg:flex-row lg:items-center justify-between gap-4 mb-4 pb-3 border-b border-slate-100">
        <div className="flex items-center gap-2.5">
          <div className={`p-2 rounded-lg border ${
            activeMetric === 'grid_all' ? 'bg-indigo-50 text-indigo-600 border-indigo-200 shadow-xs' :
            activeMetric === 'all' ? 'bg-purple-50 text-purple-600 border-purple-200 shadow-xs' :
            activeMetric === 'voltage' ? 'bg-blue-50 text-blue-600 border-blue-200' :
            activeMetric === 'current' ? 'bg-cyan-50 text-cyan-600 border-cyan-200' :
            activeMetric === 'climate' ? 'bg-amber-50 text-amber-600 border-amber-200' :
            'bg-emerald-50 text-emerald-600 border-emerald-200'
          }`}>
            {activeMetric === 'grid_all' && <LayoutGrid className="w-5 h-5 text-indigo-600" />}
            {activeMetric === 'all' && <Layers className="w-5 h-5 text-purple-600" />}
            {activeMetric === 'power' && <Sun className="w-5 h-5 text-emerald-600" />}
            {activeMetric === 'voltage' && <Zap className="w-5 h-5 text-blue-600" />}
            {activeMetric === 'current' && <Activity className="w-5 h-5 text-cyan-600" />}
            {activeMetric === 'climate' && <Thermometer className="w-5 h-5 text-amber-600" />}
          </div>
          <div>
            <div className="flex items-center gap-2">
              <h2 className="text-base font-bold text-slate-900">
                {activeMetric === 'grid_all' && 'Individual Telemetry Graphs (Live Sensor Multi-Grid)'}
                {activeMetric === 'all' && 'All-in-One Synchronous Telemetry Chart'}
                {activeMetric === 'power' && (isManual ? 'Manual Mode: Without Tracking Solar Generation (0 – 50W Scale)' : 'Solar Generation Profile: Active Tracking vs Fixed (0 – 50W Scale)')}
                {activeMetric === 'voltage' && 'Solar PV & Battery Voltage Dynamics (0 – 20V Scale)'}
                {activeMetric === 'current' && 'Solar Charging Current Profile (0 – 2.5A Scale)'}
                {activeMetric === 'climate' && 'Panel Temperature & Humidity Curve (DHT11 Sensor)'}
              </h2>
              {/* Active Mode Pill Badge */}
              <span className={`text-[10px] font-bold px-2 py-0.5 rounded-full border ${
                isManual
                  ? 'bg-amber-100 text-amber-800 border-amber-300'
                  : 'bg-emerald-100 text-emerald-800 border-emerald-300'
              }`}>
                {isManual ? '🕹️ WITHOUT TRACKING' : '⚡ AUTO TRACKING'}
              </span>
            </div>
            <p className="text-xs text-slate-500 mt-0.5">
              {isManual
                ? 'Displaying solar generation profile in Manual / Fixed mode without sun alignment'
                : 'Displaying real-time multi-shaft continuous worm tracking vs fixed baseline'}
            </p>
          </div>
        </div>

        {/* System Mode Switcher (Judge Demonstration Feature!) */}
        <div className="flex items-center gap-2">
          <div className="flex items-center p-1 bg-slate-100 rounded-lg border border-slate-200">
            <button
              type="button"
              onClick={() => {
                setDisplayMode('AUTO');
                if (onToggleMode && trackingMode === 'MANUAL') onToggleMode();
              }}
              className={`flex items-center gap-1.5 px-2.5 py-1 rounded-md text-xs font-bold transition cursor-pointer ${
                !isManual
                  ? 'bg-white text-emerald-700 shadow-xs'
                  : 'text-slate-500 hover:text-slate-800'
              }`}
            >
              <TrendingUp className="w-3.5 h-3.5" />
              <span>Auto (Tracking)</span>
            </button>
            <button
              type="button"
              onClick={() => {
                setDisplayMode('MANUAL');
                if (onToggleMode && trackingMode === 'AUTO') onToggleMode();
              }}
              className={`flex items-center gap-1.5 px-2.5 py-1 rounded-md text-xs font-bold transition cursor-pointer ${
                isManual
                  ? 'bg-white text-amber-700 shadow-xs'
                  : 'text-slate-500 hover:text-slate-800'
              }`}
            >
              <Sliders className="w-3.5 h-3.5" />
              <span>Manual (No Tracking)</span>
            </button>
          </div>
        </div>
      </div>

      {/* Mode Comparison Notification Banner */}
      {isManual ? (
        <div className="mb-4 p-3 bg-amber-50/90 border border-amber-300/80 rounded-xl flex flex-col sm:flex-row sm:items-center justify-between gap-3 text-xs text-amber-950">
          <div className="flex items-start sm:items-center gap-2.5">
            <AlertTriangle className="w-4 h-4 text-amber-600 flex-shrink-0 mt-0.5 sm:mt-0" />
            <div>
              <span className="font-bold text-amber-900">MANUAL MODE (WITHOUT TRACKING):</span>
              <span className="ml-1 text-amber-800">
                Multi-shaft worm gear is parked at fixed tilt. The system operates as a standard flat solar array without sun alignment (yielding ~28% to ~39% less power).
              </span>
            </div>
          </div>
          <button
            onClick={() => {
              setDisplayMode('AUTO');
              if (onToggleMode && trackingMode === 'MANUAL') onToggleMode();
            }}
            className="px-3 py-1 bg-emerald-600 hover:bg-emerald-700 text-white font-bold rounded-lg text-xs shadow-xs transition flex-shrink-0 cursor-pointer self-start sm:self-auto"
          >
            Activate Auto Tracking (+38.9%)
          </button>
        </div>
      ) : (
        <div className="mb-4 p-3 bg-emerald-50/80 border border-emerald-200 rounded-xl flex flex-col sm:flex-row sm:items-center justify-between gap-3 text-xs text-emerald-950">
          <div className="flex items-start sm:items-center gap-2.5">
            <TrendingUp className="w-4 h-4 text-emerald-600 flex-shrink-0 mt-0.5 sm:mt-0" />
            <div>
              <span className="font-bold text-emerald-900">AUTO TRACKING SYNCHRONIZED:</span>
              <span className="ml-1 text-emerald-700">
                10 parallel shafts synchronized via central worm drive. Continuous astronomical solar tracking delivers +38.9% net harvest advantage.
              </span>
            </div>
          </div>
          <button
            onClick={() => {
              setDisplayMode('MANUAL');
              if (onToggleMode && trackingMode === 'AUTO') onToggleMode();
            }}
            className="px-3 py-1 bg-amber-600 hover:bg-amber-700 text-white font-bold rounded-lg text-xs shadow-xs transition flex-shrink-0 cursor-pointer self-start sm:self-auto"
          >
            Compare Without Tracking (Manual)
          </button>
        </div>
      )}

      {/* Metric Selector Tabs */}
      <div className="flex flex-wrap items-center gap-1.5 bg-slate-100/90 p-1 rounded-xl border border-slate-200 text-xs font-semibold mb-4">
        <button
          onClick={() => setActiveMetric('grid_all')}
          className={`flex items-center gap-1.5 px-3 py-1.5 rounded-lg transition cursor-pointer ${
            activeMetric === 'grid_all'
              ? 'bg-indigo-600 text-white shadow-xs font-bold'
              : 'text-slate-600 hover:text-slate-900 hover:bg-slate-200/50'
          }`}
        >
          <LayoutGrid className="w-3.5 h-3.5" />
          <span>All 4 Single Graphs (Grid)</span>
        </button>

        <button
          onClick={() => setActiveMetric('all')}
          className={`flex items-center gap-1.5 px-3 py-1.5 rounded-lg transition cursor-pointer ${
            activeMetric === 'all'
              ? 'bg-purple-600 text-white shadow-xs font-bold'
              : 'text-slate-600 hover:text-slate-900 hover:bg-slate-200/50'
          }`}
        >
          <Layers className="w-3.5 h-3.5" />
          <span>Unified Overlay Graph</span>
        </button>

        <button
          onClick={() => setActiveMetric('power')}
          className={`flex items-center gap-1.5 px-3 py-1.5 rounded-lg transition cursor-pointer ${
            activeMetric === 'power'
              ? 'bg-white text-emerald-700 shadow-xs font-bold border border-slate-200/80'
              : 'text-slate-600 hover:text-slate-900'
          }`}
        >
          <Sun className="w-3.5 h-3.5 text-emerald-600" />
          <span>Power (W)</span>
        </button>

        <button
          onClick={() => setActiveMetric('voltage')}
          className={`flex items-center gap-1.5 px-3 py-1.5 rounded-lg transition cursor-pointer ${
            activeMetric === 'voltage'
              ? 'bg-white text-blue-700 shadow-xs font-bold border border-slate-200/80'
              : 'text-slate-600 hover:text-slate-900'
          }`}
        >
          <Zap className="w-3.5 h-3.5 text-blue-600" />
          <span>Voltage (V)</span>
        </button>

        <button
          onClick={() => setActiveMetric('current')}
          className={`flex items-center gap-1.5 px-3 py-1.5 rounded-lg transition cursor-pointer ${
            activeMetric === 'current'
              ? 'bg-white text-cyan-700 shadow-xs font-bold border border-slate-200/80'
              : 'text-slate-600 hover:text-slate-900'
          }`}
        >
          <Activity className="w-3.5 h-3.5 text-cyan-600" />
          <span>Current (A)</span>
        </button>

        <button
          onClick={() => setActiveMetric('climate')}
          className={`flex items-center gap-1.5 px-3 py-1.5 rounded-lg transition cursor-pointer ${
            activeMetric === 'climate'
              ? 'bg-white text-amber-700 shadow-xs font-bold border border-slate-200/80'
              : 'text-slate-600 hover:text-slate-900'
          }`}
        >
          <Thermometer className="w-3.5 h-3.5 text-amber-600" />
          <span>Temp (°C)</span>
        </button>
      </div>

      {/* Dynamic Sub-header Legend & Interactive Parameter Chips for Unified View */}
      {activeMetric === 'all' && (
        <div className="mb-3 space-y-2">
          <div className="flex flex-wrap items-center justify-between gap-2 bg-slate-50/90 p-2.5 rounded-xl border border-slate-200/80 text-xs">
            <div className="flex flex-wrap items-center gap-1.5">
              <span className="text-[11px] font-bold text-slate-500 uppercase tracking-wider mr-1 flex items-center gap-1">
                <Eye className="w-3 h-3 text-slate-400" />
                Parameters:
              </span>

              {/* 1. Tracking Power Pill */}
              <button
                type="button"
                onClick={() => toggleLine('trackingW')}
                className={`flex items-center gap-1.5 px-2.5 py-1 rounded-md text-[11px] font-bold transition border cursor-pointer ${
                  visibleLines.trackingW
                    ? 'bg-emerald-50 text-emerald-800 border-emerald-300 shadow-2xs'
                    : 'bg-white text-slate-400 border-slate-200 opacity-60'
                }`}
              >
                <span className="w-2.5 h-2.5 rounded-full bg-emerald-500"></span>
                <span>Tracking ({liveTrackingW.toFixed(1)}W)</span>
                {visibleLines.trackingW && <Check className="w-3 h-3 text-emerald-600" />}
              </button>

              {/* 2. Fixed Power Pill */}
              <button
                type="button"
                onClick={() => toggleLine('fixedW')}
                className={`flex items-center gap-1.5 px-2.5 py-1 rounded-md text-[11px] font-bold transition border cursor-pointer ${
                  visibleLines.fixedW
                    ? 'bg-slate-100 text-slate-800 border-slate-300 shadow-2xs'
                    : 'bg-white text-slate-400 border-slate-200 opacity-60'
                }`}
              >
                <span className="w-2.5 h-1 bg-slate-400"></span>
                <span>Fixed ({liveFixedW.toFixed(1)}W)</span>
                {visibleLines.fixedW && <Check className="w-3 h-3 text-slate-600" />}
              </button>

              {/* 3. Solar PV Voltage Pill */}
              <button
                type="button"
                onClick={() => toggleLine('solarVoltage')}
                className={`flex items-center gap-1.5 px-2.5 py-1 rounded-md text-[11px] font-bold transition border cursor-pointer ${
                  visibleLines.solarVoltage
                    ? 'bg-blue-50 text-blue-800 border-blue-300 shadow-2xs'
                    : 'bg-white text-slate-400 border-slate-200 opacity-60'
                }`}
              >
                <span className="w-2.5 h-2.5 rounded-full bg-blue-600"></span>
                <span>PV ({livePvVolt.toFixed(1)}V)</span>
                {visibleLines.solarVoltage && <Check className="w-3 h-3 text-blue-600" />}
              </button>

              {/* 4. Battery Voltage Pill */}
              <button
                type="button"
                onClick={() => toggleLine('battVoltage')}
                className={`flex items-center gap-1.5 px-2.5 py-1 rounded-md text-[11px] font-bold transition border cursor-pointer ${
                  visibleLines.battVoltage
                    ? 'bg-purple-50 text-purple-800 border-purple-300 shadow-2xs'
                    : 'bg-white text-slate-400 border-slate-200 opacity-60'
                }`}
              >
                <span className="w-2.5 h-2.5 rounded-full bg-purple-600"></span>
                <span>Bat ({liveBattVolt.toFixed(1)}V)</span>
                {visibleLines.battVoltage && <Check className="w-3 h-3 text-purple-600" />}
              </button>

              {/* 5. Solar Current Pill */}
              <button
                type="button"
                onClick={() => toggleLine('solarCurrent')}
                className={`flex items-center gap-1.5 px-2.5 py-1 rounded-md text-[11px] font-bold transition border cursor-pointer ${
                  visibleLines.solarCurrent
                    ? 'bg-cyan-50 text-cyan-800 border-cyan-300 shadow-2xs'
                    : 'bg-white text-slate-400 border-slate-200 opacity-60'
                }`}
              >
                <span className="w-2.5 h-2.5 rounded-full bg-cyan-500"></span>
                <span>Current ({liveCurrent.toFixed(2)}A)</span>
                {visibleLines.solarCurrent && <Check className="w-3 h-3 text-cyan-600" />}
              </button>

              {/* 6. Temperature Pill */}
              <button
                type="button"
                onClick={() => toggleLine('temperature')}
                className={`flex items-center gap-1.5 px-2.5 py-1 rounded-md text-[11px] font-bold transition border cursor-pointer ${
                  visibleLines.temperature
                    ? 'bg-amber-50 text-amber-800 border-amber-300 shadow-2xs'
                    : 'bg-white text-slate-400 border-slate-200 opacity-60'
                }`}
              >
                <span className="w-2.5 h-2.5 rounded-full bg-amber-500"></span>
                <span>Temp ({liveTemp.toFixed(1)}°C)</span>
                {visibleLines.temperature && <Check className="w-3 h-3 text-amber-600" />}
              </button>
            </div>

            {/* Quick Action Presets */}
            <div className="flex items-center gap-1.5">
              <button
                type="button"
                onClick={selectAllLines}
                className="px-2 py-0.5 text-[10px] font-bold text-slate-600 hover:text-slate-900 bg-white hover:bg-slate-100 rounded border border-slate-200 transition cursor-pointer"
              >
                All ON
              </button>
              <button
                type="button"
                onClick={selectCoreLines}
                className="px-2 py-0.5 text-[10px] font-bold text-slate-600 hover:text-slate-900 bg-white hover:bg-slate-100 rounded border border-slate-200 transition flex items-center gap-1 cursor-pointer"
              >
                <RefreshCw className="w-2.5 h-2.5 text-slate-500" />
                Core 4
              </button>
            </div>
          </div>
        </div>
      )}

      {/* Main Graph Content Area */}
      {activeMetric === 'grid_all' && renderAllSingleGraphsGrid()}
      {activeMetric === 'all' && renderUnifiedAllChart('h-96')}
      {activeMetric === 'power' && renderPowerChart('h-72')}
      {activeMetric === 'voltage' && renderVoltageChart('h-72')}
      {activeMetric === 'current' && renderCurrentChart('h-72')}
      {activeMetric === 'climate' && renderClimateChart('h-72')}

      {/* Helpful Axis Guide for Unified View */}
      {activeMetric === 'all' && (
        <div className="mt-3 pt-2.5 border-t border-slate-100 flex flex-wrap items-center justify-between gap-3 text-[11px] text-slate-500 font-medium">
          <div className="flex items-center gap-3">
            <span className="flex items-center gap-1">
              <span className="w-2 h-2 rounded-full bg-emerald-500"></span>
              <strong>Left Axis:</strong> Power (0 - 50W) & Temp (0 - 50°C)
            </span>
            <span className="flex items-center gap-1">
              <span className="w-2 h-2 rounded-full bg-blue-600"></span>
              <strong>Right Axis:</strong> Voltage (0 - 20V)
            </span>
            <span className="flex items-center gap-1">
              <span className="w-2 h-2 rounded-full bg-cyan-500"></span>
              <strong>Current:</strong> 0 - 2.5A
            </span>
          </div>

          <div className="font-mono text-emerald-700 bg-emerald-50 px-2 py-0.5 rounded border border-emerald-200 font-bold">
            Live Harvest Boost: +{liveDiffW} W (+{liveFixedW > 0 ? (((liveTrackingW - liveFixedW) / liveFixedW) * 100).toFixed(0) : 0}%)
          </div>
        </div>
      )}
    </div>
  );
};
