import React, { useState } from 'react';
import { Sun, TrendingUp, Zap, Thermometer, Layers, Check, RefreshCw, Eye } from 'lucide-react';
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

import type { HourlyGenerationPoint } from '../types/dashboard';

interface SolarPowerChartProps {
  data: HourlyGenerationPoint[];
  currentHourDecimal: number;
}

type ChartMetric = 'all' | 'power' | 'voltage' | 'climate';

// =============================================================================
// 1. Unified All-in-One Tooltip (Shows All 7 Parameters Simultaneously)
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
const PowerTooltip = ({ active, payload, label }: any) => {
  if (active && payload && payload.length) {
    const tracking = payload.find((p: any) => p.dataKey === 'trackingW')?.value ?? 0;
    const fixed = payload.find((p: any) => p.dataKey === 'fixedW')?.value ?? 0;
    const delta = tracking > fixed ? (tracking - fixed).toFixed(1) : '0.0';
    const gainPercent = fixed > 0 ? (((tracking - fixed) / fixed) * 100).toFixed(1) : '0';

    return (
      <div className="bg-slate-900/95 text-white p-3 rounded-lg shadow-xl border border-slate-700 text-xs font-mono">
        <div className="text-slate-300 font-semibold border-b border-slate-700 pb-1 mb-1.5 flex items-center justify-between gap-4">
          <span>Time: {label}</span>
          <span className="text-[10px] bg-emerald-900/80 text-emerald-300 px-1.5 rounded">
            +{gainPercent}% Gain
          </span>
        </div>
        <div className="space-y-1">
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
// 4. Individual Climate Tooltip
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
}) => {
  // Default to 'all' for the unified single graph experience!
  const [activeMetric, setActiveMetric] = useState<ChartMetric>('all');

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
    humidity: false, // Default off to keep the graph super crisp; click to enable!
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

  // ===========================================================================
  // RENDER: Unified All-in-One Graph (ALL Parameters in 1 Single Graph!)
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

          {/* Hidden Y-Axis: Current (0 - 2.5A) scaled elegantly to full height */}
          <ReYAxis
            yAxisId="curr"
            domain={[0, 2.5]}
            hide={true}
          />

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

          {/* Reference benchmark line: 14.4V Float Battery Voltage */}
          <ReReferenceLine
            yAxisId="right"
            y={14.4}
            stroke="#9333ea"
            strokeDasharray="3 3"
            strokeWidth={1}
            strokeOpacity={0.6}
            label={{ value: '14.4V Float', position: 'insideRight', fill: '#9333ea', fontSize: 9 }}
          />

          {/* 1. Fixed Solar Baseline (Line) */}
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

          {/* 2. Multi-Shaft Tracking Power (Area) */}
          {visibleLines.trackingW && (
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

          {/* 3. Solar PV Voltage (Line) */}
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

          {/* 4. Battery Voltage (Line) */}
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

          {/* 5. Solar Current (Line) */}
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

          {/* 6. Panel Temperature (Line) */}
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

          {/* 7. Relative Humidity (Line) */}
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
  // RENDER: Individual Power Chart
  // ===========================================================================
  const renderPowerChart = (heightClass = "h-72") => (
    <div className={`${heightClass} w-full`}>
      <ReResponsiveContainer width="100%" height="100%">
        <ReAreaChart data={data} margin={{ top: 10, right: 15, left: -10, bottom: 0 }}>
          <defs>
            <linearGradient id="trackingGradient" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#10b981" stopOpacity={0.4} />
              <stop offset="95%" stopColor="#10b981" stopOpacity={0.0} />
            </linearGradient>
            <linearGradient id="fixedGradient" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#94a3b8" stopOpacity={0.25} />
              <stop offset="95%" stopColor="#94a3b8" stopOpacity={0.0} />
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
          <ReTooltip content={<PowerTooltip />} />
          {isDaylight && (
            <ReReferenceLine
              x={currentTimeStr}
              stroke="#f59e0b"
              strokeDasharray="4 4"
              strokeWidth={1.5}
              label={{ value: 'NOW', position: 'top', fill: '#f59e0b', fontSize: 10, fontWeight: 'bold' }}
            />
          )}
          <ReArea
            type="monotone"
            dataKey="fixedW"
            stroke="#94a3b8"
            strokeWidth={1.75}
            fillOpacity={1}
            fill="url(#fixedGradient)"
            name="Fixed Array (W)"
          />
          <ReArea
            type="monotone"
            dataKey="trackingW"
            stroke="#10b981"
            strokeWidth={2.5}
            fillOpacity={1}
            fill="url(#trackingGradient)"
            name="Tracking Output (W)"
          />
        </ReAreaChart>
      </ReResponsiveContainer>
    </div>
  );

  // ===========================================================================
  // RENDER: Individual Voltage Chart
  // ===========================================================================
  const renderVoltageChart = (heightClass = "h-72") => (
    <div className={`${heightClass} w-full`}>
      <ReResponsiveContainer width="100%" height="100%">
        <ReAreaChart data={data} margin={{ top: 10, right: 15, left: -10, bottom: 0 }}>
          <defs>
            <linearGradient id="pvVoltGradient" x1="0" y1="0" x2="0" y2="1">
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
            fill="url(#pvVoltGradient)"
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
  // RENDER: Individual Climate Chart
  // ===========================================================================
  const renderClimateChart = (heightClass = "h-72") => (
    <div className={`${heightClass} w-full`}>
      <ReResponsiveContainer width="100%" height="100%">
        <ReAreaChart data={data} margin={{ top: 10, right: 15, left: -10, bottom: 0 }}>
          <defs>
            <linearGradient id="tempGradient" x1="0" y1="0" x2="0" y2="1">
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
            fill="url(#tempGradient)"
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

  return (
    <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs mb-6">
      {/* Chart Header with Interactive Metric Selector */}
      <div className="flex flex-col lg:flex-row lg:items-center justify-between gap-4 mb-4 pb-3 border-b border-slate-100">
        <div className="flex items-center gap-2.5">
          <div className={`p-2 rounded-lg border ${
            activeMetric === 'all' ? 'bg-purple-50 text-purple-600 border-purple-200 shadow-xs' :
            activeMetric === 'voltage' ? 'bg-blue-50 text-blue-600 border-blue-200' :
            activeMetric === 'climate' ? 'bg-amber-50 text-amber-600 border-amber-200' :
            'bg-emerald-50 text-emerald-600 border-emerald-200'
          }`}>
            {activeMetric === 'all' && <Layers className="w-5 h-5 text-purple-600" />}
            {activeMetric === 'voltage' && <Zap className="w-5 h-5 text-blue-600" />}
            {activeMetric === 'climate' && <Thermometer className="w-5 h-5 text-amber-600" />}
            {activeMetric === 'power' && <Sun className="w-5 h-5 text-emerald-600" />}
          </div>
          <div>
            <h2 className="text-base font-bold text-slate-900 flex items-center gap-2">
              {activeMetric === 'all' && 'सर्व पॅरामीटर्स एकत्र (All-in-One Synchronous Chart)'}
              {activeMetric === 'power' && 'Solar Generation Profile (0 – 50W Scale)'}
              {activeMetric === 'voltage' && 'Solar PV & Battery Voltage Dynamics (0 – 20V Scale)'}
              {activeMetric === 'climate' && 'Panel Temperature & Humidity Curve (DHT11 Sensor)'}
              {activeMetric === 'all' && (
                <span className="text-[10px] bg-purple-100 text-purple-700 px-2 py-0.5 rounded-full font-bold border border-purple-200">
                  Single Unified Graph
                </span>
              )}
            </h2>
            <p className="text-xs text-slate-500 mt-0.5">
              {activeMetric === 'all' && 'Power (W), Voltage (V), Current (A), and Temp (°C) — All synchronized in 1 single graph'}
              {activeMetric === 'power' && 'Comparative analysis: Multi-Shaft Dynamic Tracking vs Fixed Panel Baseline'}
              {activeMetric === 'voltage' && 'Comparative DC voltage monitoring: Solar array generation vs Battery charge level'}
              {activeMetric === 'climate' && 'Thermal environment monitoring: Ambient solar cell temperature vs Relative humidity'}
            </p>
          </div>
        </div>

        {/* Metric Selector Buttons */}
        <div className="flex flex-wrap items-center gap-1.5 bg-slate-100/90 p-1 rounded-xl border border-slate-200 text-xs font-semibold">
          <button
            onClick={() => setActiveMetric('all')}
            className={`flex items-center gap-1.5 px-3 py-1.5 rounded-lg transition ${
              activeMetric === 'all'
                ? 'bg-purple-600 text-white shadow-xs font-bold'
                : 'text-slate-600 hover:text-slate-900 hover:bg-slate-200/50'
            }`}
          >
            <Layers className="w-3.5 h-3.5" />
            <span>📊 All-in-1 Unified</span>
          </button>

          <button
            onClick={() => setActiveMetric('power')}
            className={`flex items-center gap-1.5 px-3 py-1.5 rounded-lg transition ${
              activeMetric === 'power'
                ? 'bg-white text-emerald-700 shadow-xs font-bold border border-slate-200/80'
                : 'text-slate-600 hover:text-slate-900'
            }`}
          >
            <Sun className="w-3.5 h-3.5 text-emerald-600" />
            <span>Power (0-50W)</span>
          </button>

          <button
            onClick={() => setActiveMetric('voltage')}
            className={`flex items-center gap-1.5 px-3 py-1.5 rounded-lg transition ${
              activeMetric === 'voltage'
                ? 'bg-white text-blue-700 shadow-xs font-bold border border-slate-200/80'
                : 'text-slate-600 hover:text-slate-900'
            }`}
          >
            <Zap className="w-3.5 h-3.5 text-blue-600" />
            <span>Voltage (0-20V)</span>
          </button>

          <button
            onClick={() => setActiveMetric('climate')}
            className={`flex items-center gap-1.5 px-3 py-1.5 rounded-lg transition ${
              activeMetric === 'climate'
                ? 'bg-white text-amber-700 shadow-xs font-bold border border-slate-200/80'
                : 'text-slate-600 hover:text-slate-900'
            }`}
          >
            <Thermometer className="w-3.5 h-3.5 text-amber-600" />
            <span>Temp (°C)</span>
          </button>
        </div>
      </div>

      {/* Dynamic Sub-header Legend & Interactive Parameter Chips */}
      {activeMetric === 'all' ? (
        <div className="mb-3 space-y-2">
          {/* Interactive Line Filters */}
          <div className="flex flex-wrap items-center justify-between gap-2 bg-slate-50/90 p-2.5 rounded-xl border border-slate-200/80 text-xs">
            <div className="flex flex-wrap items-center gap-1.5">
              <span className="text-[11px] font-bold text-slate-500 uppercase tracking-wider mr-1 flex items-center gap-1">
                <Eye className="w-3 h-3 text-slate-400" />
                पॅरामीटर्स:
              </span>

              {/* 1. Tracking Power Pill */}
              <button
                type="button"
                onClick={() => toggleLine('trackingW')}
                className={`flex items-center gap-1.5 px-2.5 py-1 rounded-md text-[11px] font-bold transition border ${
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
                className={`flex items-center gap-1.5 px-2.5 py-1 rounded-md text-[11px] font-bold transition border ${
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
                className={`flex items-center gap-1.5 px-2.5 py-1 rounded-md text-[11px] font-bold transition border ${
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
                className={`flex items-center gap-1.5 px-2.5 py-1 rounded-md text-[11px] font-bold transition border ${
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
                className={`flex items-center gap-1.5 px-2.5 py-1 rounded-md text-[11px] font-bold transition border ${
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
                className={`flex items-center gap-1.5 px-2.5 py-1 rounded-md text-[11px] font-bold transition border ${
                  visibleLines.temperature
                    ? 'bg-amber-50 text-amber-800 border-amber-300 shadow-2xs'
                    : 'bg-white text-slate-400 border-slate-200 opacity-60'
                }`}
              >
                <span className="w-2.5 h-2.5 rounded-full bg-amber-500"></span>
                <span>Temp ({liveTemp.toFixed(1)}°C)</span>
                {visibleLines.temperature && <Check className="w-3 h-3 text-amber-600" />}
              </button>

              {/* 7. Humidity Pill */}
              <button
                type="button"
                onClick={() => toggleLine('humidity')}
                className={`flex items-center gap-1.5 px-2.5 py-1 rounded-md text-[11px] font-bold transition border ${
                  visibleLines.humidity
                    ? 'bg-sky-50 text-sky-800 border-sky-300 shadow-2xs'
                    : 'bg-white text-slate-400 border-slate-200 opacity-60'
                }`}
              >
                <span className="w-2.5 h-2.5 rounded-full bg-sky-500"></span>
                <span>Hum ({liveHum.toFixed(0)}%)</span>
                {visibleLines.humidity && <Check className="w-3 h-3 text-sky-600" />}
              </button>
            </div>

            {/* Quick Action Presets */}
            <div className="flex items-center gap-1.5">
              <button
                type="button"
                onClick={selectAllLines}
                className="px-2 py-0.5 text-[10px] font-bold text-slate-600 hover:text-slate-900 bg-white hover:bg-slate-100 rounded border border-slate-200 transition"
              >
                सर्व ऑन
              </button>
              <button
                type="button"
                onClick={selectCoreLines}
                className="px-2 py-0.5 text-[10px] font-bold text-slate-600 hover:text-slate-900 bg-white hover:bg-slate-100 rounded border border-slate-200 transition flex items-center gap-1"
              >
                <RefreshCw className="w-2.5 h-2.5 text-slate-500" />
                मुख्य 4
              </button>
            </div>
          </div>
        </div>
      ) : (
        <div className="flex flex-wrap items-center justify-between gap-4 text-xs font-medium text-slate-600 mb-3">
          {activeMetric === 'power' && (
            <>
              <div className="flex items-center gap-4">
                <div className="flex items-center gap-1.5">
                  <span className="w-3 h-3 rounded-xs bg-emerald-500"></span>
                  <span className="font-semibold text-slate-800">Tracking (Watts)</span>
                </div>
                <div className="flex items-center gap-1.5">
                  <span className="w-3 h-1 bg-slate-400"></span>
                  <span>Fixed Baseline (Watts)</span>
                </div>
              </div>
              {liveTrackingW > 0 && (
                <div className="flex items-center gap-1 bg-emerald-50 text-emerald-800 px-2 py-0.5 rounded-md border border-emerald-200 font-mono text-[11px] font-bold">
                  <TrendingUp className="w-3 h-3 text-emerald-600" />
                  <span>+{liveDiffW} W Harvest Boost</span>
                </div>
              )}
            </>
          )}

          {activeMetric === 'voltage' && (
            <>
              <div className="flex items-center gap-4">
                <div className="flex items-center gap-1.5">
                  <span className="w-3 h-3 rounded-xs bg-blue-600"></span>
                  <span className="font-semibold text-slate-800">Solar PV Voltage (V)</span>
                </div>
                <div className="flex items-center gap-1.5">
                  <span className="w-3 h-1 bg-purple-600"></span>
                  <span>Battery Voltage (V)</span>
                </div>
              </div>
              <div className="flex items-center gap-2 font-mono text-[11px] font-bold">
                <span className="bg-blue-50 text-blue-800 px-2 py-0.5 rounded-md border border-blue-200">
                  PV: {livePvVolt.toFixed(2)} V
                </span>
                <span className="bg-purple-50 text-purple-800 px-2 py-0.5 rounded-md border border-purple-200">
                  Bat: {liveBattVolt.toFixed(2)} V
                </span>
              </div>
            </>
          )}

          {activeMetric === 'climate' && (
            <>
              <div className="flex items-center gap-4">
                <div className="flex items-center gap-1.5">
                  <span className="w-3 h-3 rounded-xs bg-amber-500"></span>
                  <span className="font-semibold text-slate-800">Temperature (°C)</span>
                </div>
                <div className="flex items-center gap-1.5">
                  <span className="w-3 h-1 bg-cyan-500"></span>
                  <span>Humidity (% RH)</span>
                </div>
              </div>
              <div className="flex items-center gap-2 font-mono text-[11px] font-bold">
                <span className="bg-amber-50 text-amber-800 px-2 py-0.5 rounded-md border border-amber-200">
                  Temp: {liveTemp.toFixed(1)} °C
                </span>
                <span className="bg-cyan-50 text-cyan-800 px-2 py-0.5 rounded-md border border-cyan-200">
                  Hum: {liveHum.toFixed(0)} %
                </span>
              </div>
            </>
          )}
        </div>
      )}

      {/* Main Graph Content Area */}
      {activeMetric === 'all' && renderUnifiedAllChart('h-96')}
      {activeMetric === 'power' && renderPowerChart('h-72')}
      {activeMetric === 'voltage' && renderVoltageChart('h-72')}
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
