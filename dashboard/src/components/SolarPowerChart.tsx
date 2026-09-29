import React, { useState } from 'react';
import { Sun, TrendingUp, Zap, Thermometer, Layers } from 'lucide-react';
import {
  AreaChart as ReAreaChart,
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

type ChartMetric = 'power' | 'voltage' | 'climate' | 'all';

// 1. Tooltip for Power Metric
const PowerTooltip = ({ active, payload, label }: any) => {
  if (active && payload && payload.length) {
    const tracking = payload.find((p: any) => p.dataKey === 'trackingW')?.value 
      ?? (payload.find((p: any) => p.dataKey === 'trackingKw')?.value !== undefined ? (payload.find((p: any) => p.dataKey === 'trackingKw')?.value * 1000) : 0);
    const fixed = payload.find((p: any) => p.dataKey === 'fixedW')?.value 
      ?? (payload.find((p: any) => p.dataKey === 'fixedKw')?.value !== undefined ? (payload.find((p: any) => p.dataKey === 'fixedKw')?.value * 1000) : 0);
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

// 2. Tooltip for Voltage Metric (Solar PV & Battery)
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

// 3. Tooltip for Climate Metric (DHT11 Temperature & Humidity)
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

export const SolarPowerChart: React.FC<SolarPowerChartProps> = ({
  data,
  currentHourDecimal,
}) => {
  const [activeMetric, setActiveMetric] = useState<ChartMetric>('power');

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
  const liveTemp = currentPoint?.temperature ?? 34.5;
  const liveHum = currentPoint?.humidity ?? 52;

  // Render individual Power Chart
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

  // Render individual Voltage Chart
  const renderVoltageChart = (heightClass = "h-72") => (
    <div className={`${heightClass} w-full`}>
      <ReResponsiveContainer width="100%" height="100%">
        <ReAreaChart data={data} margin={{ top: 10, right: 15, left: -10, bottom: 0 }}>
          <defs>
            <linearGradient id="pvVoltGradient" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#2563eb" stopOpacity={0.35} />
              <stop offset="95%" stopColor="#2563eb" stopOpacity={0.0} />
            </linearGradient>
            <linearGradient id="battVoltGradient" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#9333ea" stopOpacity={0.2} />
              <stop offset="95%" stopColor="#9333ea" stopOpacity={0.0} />
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

  // Render individual Climate / Temperature Chart
  const renderClimateChart = (heightClass = "h-72") => (
    <div className={`${heightClass} w-full`}>
      <ReResponsiveContainer width="100%" height="100%">
        <ReAreaChart data={data} margin={{ top: 10, right: 15, left: -10, bottom: 0 }}>
          <defs>
            <linearGradient id="tempGradient" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#f59e0b" stopOpacity={0.35} />
              <stop offset="95%" stopColor="#f59e0b" stopOpacity={0.0} />
            </linearGradient>
            <linearGradient id="humGradient" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#06b6d4" stopOpacity={0.2} />
              <stop offset="95%" stopColor="#06b6d4" stopOpacity={0.0} />
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
            activeMetric === 'voltage' ? 'bg-blue-50 text-blue-600 border-blue-200' :
            activeMetric === 'climate' ? 'bg-amber-50 text-amber-600 border-amber-200' :
            activeMetric === 'all' ? 'bg-purple-50 text-purple-600 border-purple-200' :
            'bg-emerald-50 text-emerald-600 border-emerald-200'
          }`}>
            {activeMetric === 'voltage' && <Zap className="w-5 h-5 text-blue-600" />}
            {activeMetric === 'climate' && <Thermometer className="w-5 h-5 text-amber-600" />}
            {activeMetric === 'all' && <Layers className="w-5 h-5 text-purple-600" />}
            {activeMetric === 'power' && <Sun className="w-5 h-5 text-emerald-600" />}
          </div>
          <div>
            <h2 className="text-base font-bold text-slate-900">
              {activeMetric === 'voltage' && 'Solar PV & Battery Voltage Dynamics (0 – 20V Scale)'}
              {activeMetric === 'climate' && 'Panel Temperature & Humidity Curve (DHT11 Sensor)'}
              {activeMetric === 'all' && 'Comprehensive Telemetry Dynamics (Power, Voltage & Thermal)'}
              {activeMetric === 'power' && 'Solar Generation Profile (0 – 50W Scale)'}
            </h2>
            <p className="text-xs text-slate-500 mt-0.5">
              {activeMetric === 'voltage' && 'Comparative DC voltage monitoring: Solar array generation vs Battery charge level'}
              {activeMetric === 'climate' && 'Thermal environment monitoring: Ambient solar cell temperature vs Relative humidity'}
              {activeMetric === 'all' && 'All 3 synchronous metrics: 50W Generation, 20V Voltage rails, and Thermal health'}
              {activeMetric === 'power' && 'Comparative analysis: Multi-Shaft Dynamic Tracking vs Fixed Panel Baseline'}
            </p>
          </div>
        </div>

        {/* Metric Selector Buttons */}
        <div className="flex flex-wrap items-center gap-1.5 bg-slate-100/90 p-1 rounded-xl border border-slate-200 text-xs font-semibold">
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

          <button
            onClick={() => setActiveMetric('all')}
            className={`flex items-center gap-1.5 px-3 py-1.5 rounded-lg transition ${
              activeMetric === 'all'
                ? 'bg-white text-purple-700 shadow-xs font-bold border border-slate-200/80'
                : 'text-slate-600 hover:text-slate-900'
            }`}
          >
            <Layers className="w-3.5 h-3.5 text-purple-600" />
            <span>View All 3</span>
          </button>
        </div>
      </div>

      {/* Dynamic Sub-header Legend & Live Badges */}
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

        {activeMetric === 'all' && (
          <div className="flex items-center gap-2 text-xs font-semibold text-purple-700">
            <span className="w-2 h-2 rounded-full bg-purple-500 animate-pulse"></span>
            <span>Simultaneous 3-Axis Multi-Metric Timeline Active</span>
          </div>
        )}
      </div>

      {/* Main Graph Content Area */}
      {activeMetric === 'power' && renderPowerChart('h-72')}
      {activeMetric === 'voltage' && renderVoltageChart('h-72')}
      {activeMetric === 'climate' && renderClimateChart('h-72')}

      {activeMetric === 'all' && (
        <div className="space-y-6 pt-2">
          {/* 1. Solar Generation Power Chart */}
          <div className="p-3 rounded-xl border border-slate-100 bg-slate-50/50">
            <div className="flex items-center justify-between text-xs font-bold text-slate-800 mb-2">
              <span className="flex items-center gap-1.5 text-emerald-700">
                <Sun className="w-3.5 h-3.5 text-emerald-600" />
                1. Solar Generation Profile (0 - 50W Scale)
              </span>
              <span className="font-mono text-emerald-600">Peak: {liveTrackingW.toFixed(1)} W</span>
            </div>
            {renderPowerChart('h-52')}
          </div>

          {/* 2. Solar & Battery Voltage Chart */}
          <div className="p-3 rounded-xl border border-slate-100 bg-slate-50/50">
            <div className="flex items-center justify-between text-xs font-bold text-slate-800 mb-2">
              <span className="flex items-center gap-1.5 text-blue-700">
                <Zap className="w-3.5 h-3.5 text-blue-600" />
                2. Solar PV & Battery Voltage Dynamics (0 - 20V Scale)
              </span>
              <span className="font-mono text-blue-600">Bus: {livePvVolt.toFixed(2)} V</span>
            </div>
            {renderVoltageChart('h-52')}
          </div>

          {/* 3. Panel Temperature & Humidity Chart */}
          <div className="p-3 rounded-xl border border-slate-100 bg-slate-50/50">
            <div className="flex items-center justify-between text-xs font-bold text-slate-800 mb-2">
              <span className="flex items-center gap-1.5 text-amber-700">
                <Thermometer className="w-3.5 h-3.5 text-amber-600" />
                3. Ambient Thermal & Humidity Profile (DHT11 PB5)
              </span>
              <span className="font-mono text-amber-600">Temp: {liveTemp.toFixed(1)} °C</span>
            </div>
            {renderClimateChart('h-52')}
          </div>
        </div>
      )}
    </div>
  );
};
