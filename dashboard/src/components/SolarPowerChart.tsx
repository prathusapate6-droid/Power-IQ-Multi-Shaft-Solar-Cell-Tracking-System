import React from 'react';
import { Sun, TrendingUp } from 'lucide-react';
import {
  AreaChart as ReAreaChart,
  Area as ReArea,
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

const CustomTooltip = ({ active, payload, label }: any) => {
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

export const SolarPowerChart: React.FC<SolarPowerChartProps> = ({
  data,
  currentHourDecimal,
}) => {
  // Daylight reference indicator
  const isDaylight = currentHourDecimal >= 6 && currentHourDecimal <= 18;
  const currentHourInt = Math.floor(currentHourDecimal);
  const currentMinStr = currentHourDecimal % 1 >= 0.5 ? '30' : '00';
  const currentTimeStr = `${currentHourInt.toString().padStart(2, '0')}:${currentMinStr}`;

  // Find maximum power currently in dataset to show instant comparison
  const currentPoint = data.find((p) => Math.abs(p.hour - currentHourDecimal) < 0.6) || data[data.length - 1];
  const liveTrackingW = currentPoint?.trackingW ?? 0;
  const liveFixedW = currentPoint?.fixedW ?? 0;
  const liveDiffW = (liveTrackingW - liveFixedW).toFixed(1);

  return (
    <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs mb-6">
      {/* Chart Header */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 mb-4">
        <div>
          <div className="flex items-center gap-2">
            <div className="p-1.5 rounded-lg bg-amber-50 text-amber-600 border border-amber-200">
              <Sun className="w-4 h-4" />
            </div>
            <div>
              <h2 className="text-base font-bold text-slate-900">
                Solar Generation Profile (0 – 200W Scale)
              </h2>
              <p className="text-xs text-slate-500 mt-0.5">
                Comparative analysis: Multi-Shaft Dynamic Tracking vs Fixed Panel Baseline
              </p>
            </div>
          </div>
        </div>

        {/* Legend & Real-Time Boost Indicator */}
        <div className="flex flex-wrap items-center gap-4 text-xs font-medium text-slate-600">
          <div className="flex items-center gap-1.5">
            <span className="w-3 h-3 rounded-xs bg-emerald-500"></span>
            <span className="font-semibold text-slate-800">Tracking (Watts)</span>
          </div>
          <div className="flex items-center gap-1.5">
            <span className="w-3 h-1 bg-slate-400"></span>
            <span>Fixed Baseline (Watts)</span>
          </div>
          {liveTrackingW > 0 && (
            <div className="hidden md:flex items-center gap-1 bg-emerald-50 text-emerald-800 px-2 py-0.5 rounded-md border border-emerald-200 font-mono text-[11px] font-bold">
              <TrendingUp className="w-3 h-3 text-emerald-600" />
              <span>+{liveDiffW} W Harvest Boost</span>
            </div>
          )}
        </div>
      </div>

      {/* Recharts Area Chart */}
      <div className="h-72 w-full">
        <ReResponsiveContainer width="100%" height="100%">
          <ReAreaChart
            data={data}
            margin={{ top: 10, right: 15, left: -10, bottom: 0 }}
          >
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

            <ReXAxis
              dataKey="time"
              stroke="#94a3b8"
              fontSize={11}
              tickLine={false}
              axisLine={{ stroke: '#e2e8f0' }}
            />

            {/* Y-Axis calibrated precisely up to 200 Watts */}
            <ReYAxis
              stroke="#94a3b8"
              fontSize={11}
              tickLine={false}
              axisLine={{ stroke: '#e2e8f0' }}
              tickFormatter={(val) => `${val} W`}
              domain={[0, 200]}
              ticks={[0, 50, 100, 150, 200]}
            />

            <ReTooltip content={<CustomTooltip />} />

            {/* Current Time Indicator */}
            {isDaylight && (
              <ReReferenceLine
                x={currentTimeStr}
                stroke="#f59e0b"
                strokeDasharray="4 4"
                strokeWidth={1.5}
                label={{
                  value: 'NOW',
                  position: 'top',
                  fill: '#f59e0b',
                  fontSize: 10,
                  fontWeight: 'bold',
                }}
              />
            )}

            {/* Fixed Tilt Array Curve (Without Tracking Baseline) */}
            <ReArea
              type="monotone"
              dataKey="fixedW"
              stroke="#94a3b8"
              strokeWidth={1.75}
              fillOpacity={1}
              fill="url(#fixedGradient)"
              name="Fixed Array (W)"
            />

            {/* Multi-Shaft Tracking Curve (Active Tracking) */}
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
    </div>
  );
};
