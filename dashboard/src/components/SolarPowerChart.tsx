import React from 'react';
import {
  AreaChart,
  Area,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
  ReferenceLine,
} from 'recharts';
import { Sun, TrendingUp, Info } from 'lucide-react';
import type { HourlyGenerationPoint } from '../types/dashboard';

interface SolarPowerChartProps {
  data: HourlyGenerationPoint[];
  currentHourDecimal: number;
}

const CustomTooltip = ({ active, payload, label }: any) => {
  if (active && payload && payload.length) {
    const tracking = payload.find((p: any) => p.dataKey === 'trackingKw')?.value || 0;
    const fixed = payload.find((p: any) => p.dataKey === 'fixedKw')?.value || 0;
    const delta = tracking > fixed ? (tracking - fixed).toFixed(2) : '0.00';
    const gainPercent = fixed > 0 ? (((tracking - fixed) / fixed) * 100).toFixed(1) : '0';

    return (
      <div className="bg-slate-900/95 text-white p-3 rounded-lg shadow-xl border border-slate-700 text-xs font-mono">
        <div className="text-slate-300 font-semibold border-b border-slate-700 pb-1 mb-1.5 flex items-center justify-between gap-4">
          <span>Time: {label}</span>
          <span className="text-[10px] bg-emerald-900/80 text-emerald-300 px-1.5 rounded">
            +{gainPercent}%
          </span>
        </div>
        <div className="space-y-1">
          <div className="flex items-center justify-between gap-4">
            <span className="text-emerald-400 flex items-center gap-1.5">
              <span className="w-2 h-2 rounded-full bg-emerald-400"></span>
              Multi-Shaft Tracking:
            </span>
            <span className="font-bold">{tracking} kW</span>
          </div>
          <div className="flex items-center justify-between gap-4">
            <span className="text-slate-400 flex items-center gap-1.5">
              <span className="w-2 h-2 rounded-full bg-slate-400"></span>
              Fixed Solar Panel:
            </span>
            <span>{fixed} kW</span>
          </div>
          <div className="pt-1 border-t border-slate-800 flex items-center justify-between text-amber-300 font-bold">
            <span>Instant Energy Boost:</span>
            <span>+{delta} kW</span>
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
  // Format current hour label for reference line (e.g. 14:30)
  const currentHourInt = Math.floor(currentHourDecimal);
  const currentMinStr = currentHourDecimal % 1 >= 0.5 ? '30' : '00';
  const currentTimeStr = `${currentHourInt.toString().padStart(2, '0')}:${currentMinStr}`;

  return (
    <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs mb-6">
      {/* Chart Header */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 mb-4">
        <div>
          <div className="flex items-center gap-2">
            <div className="p-1.5 rounded-lg bg-amber-50 text-amber-600 border border-amber-200/60">
              <Sun className="w-4 h-4" />
            </div>
            <h2 className="text-base font-bold text-slate-900">
              Diurnal Solar Power Generation Profile
            </h2>
          </div>
          <p className="text-xs text-slate-500 mt-0.5">
            Real-time power curve comparing <strong>Multi-Shaft Cell Tracking</strong> vs Conventional Fixed Solar Array
          </p>
        </div>

        {/* Legend and Metrics badge */}
        <div className="flex flex-wrap items-center gap-4 text-xs">
          <div className="flex items-center gap-1.5">
            <span className="w-3 h-3 rounded-xs bg-emerald-500"></span>
            <span className="font-medium text-slate-700">Multi-Shaft Tracking (kW)</span>
          </div>
          <div className="flex items-center gap-1.5">
            <span className="w-3 h-0.5 bg-slate-400"></span>
            <span className="font-medium text-slate-500">Fixed PV Baseline (kW)</span>
          </div>
          <div className="bg-emerald-50 text-emerald-800 px-2 py-1 rounded-md border border-emerald-200 flex items-center gap-1 font-semibold text-[11px]">
            <TrendingUp className="w-3 h-3 text-emerald-600" />
            <span>Avg Harvest Gain: +28.4%</span>
          </div>
        </div>
      </div>

      {/* Recharts Area Chart */}
      <div className="h-72 w-full">
        <ResponsiveContainer width="100%" height="100%">
          <AreaChart
            data={data}
            margin={{ top: 10, right: 15, left: -15, bottom: 0 }}
          >
            <defs>
              <linearGradient id="trackingGradient" x1="0" y1="0" x2="0" y2="1">
                <stop offset="5%" stopColor="#10b981" stopOpacity={0.4} />
                <stop offset="95%" stopColor="#10b981" stopOpacity={0.0} />
              </linearGradient>
              <linearGradient id="fixedGradient" x1="0" y1="0" x2="0" y2="1">
                <stop offset="5%" stopColor="#94a3b8" stopOpacity={0.2} />
                <stop offset="95%" stopColor="#94a3b8" stopOpacity={0.0} />
              </linearGradient>
            </defs>

            <CartesianGrid strokeDasharray="3 3" vertical={false} stroke="#f1f5f9" />

            <XAxis
              dataKey="time"
              stroke="#94a3b8"
              fontSize={11}
              tickLine={false}
              axisLine={{ stroke: '#e2e8f0' }}
            />

            <YAxis
              stroke="#94a3b8"
              fontSize={11}
              tickLine={false}
              axisLine={{ stroke: '#e2e8f0' }}
              tickFormatter={(val) => `${val} kW`}
              domain={[0, 3.5]}
            />

            <Tooltip content={<CustomTooltip />} />

            {/* Current Time Indicator */}
            <ReferenceLine
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

            {/* Fixed Tilt Array Curve */}
            <Area
              type="monotone"
              dataKey="fixedKw"
              stroke="#94a3b8"
              strokeWidth={1.75}
              fillOpacity={1}
              fill="url(#fixedGradient)"
              name="Fixed Array"
            />

            {/* Multi-Shaft Tracking Curve */}
            <Area
              type="monotone"
              dataKey="trackingKw"
              stroke="#10b981"
              strokeWidth={2.5}
              fillOpacity={1}
              fill="url(#trackingGradient)"
              name="Multi-Shaft Tracking"
            />
          </AreaChart>
        </ResponsiveContainer>
      </div>

      {/* Chart Footer Technical Note */}
      <div className="mt-3 pt-3 border-t border-slate-100 flex flex-wrap items-center justify-between text-[11px] text-slate-500 gap-2">
        <span className="flex items-center gap-1.5">
          <Info className="w-3.5 h-3.5 text-slate-400" />
          <span>Notice the expanded morning (08:00–11:00) and afternoon (14:00–17:00) "shoulder hours" where cell rotation eliminates oblique cosine reflection.</span>
        </span>
        <span className="font-mono text-slate-400">Peak System Capacity: 3.2 kWp</span>
      </div>
    </div>
  );
};
