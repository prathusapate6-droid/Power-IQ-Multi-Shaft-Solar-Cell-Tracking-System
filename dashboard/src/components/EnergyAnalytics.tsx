import React from 'react';
import { 
  Leaf, 
  BarChart3, 
  Scale, 
  Calendar
} from 'lucide-react';
import { 
  BarChart, 
  Bar, 
  XAxis, 
  YAxis, 
  Tooltip, 
  ResponsiveContainer, 
  Legend 
} from 'recharts';

export const EnergyAnalytics: React.FC = () => {
  const comparisonData = [
    { period: 'Today', tracking: 14.6, fixed: 11.2, motorCost: 0.11, netGain: '+30.4%' },
    { period: 'This Week', tracking: 98.4, fixed: 76.1, motorCost: 0.77, netGain: '+29.3%' },
    { period: 'This Month', tracking: 412.0, fixed: 321.0, motorCost: 3.25, netGain: '+28.3%' },
  ];

  return (
    <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs mb-6">
      {/* Header */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 mb-4">
        <div>
          <div className="flex items-center gap-2">
            <div className="p-1.5 rounded-lg bg-emerald-50 text-emerald-700 border border-emerald-200/60">
              <BarChart3 className="w-4 h-4" />
            </div>
            <div>
              <h2 className="text-base font-bold text-slate-900">
                Energy Harvesting Analytics & Net Gain Comparison
              </h2>
              <p className="text-xs text-slate-500">
                Evaluating Additional Harvest vs Stepper Tracking Parasitic Overhead
              </p>
            </div>
          </div>
        </div>

        <div className="flex items-center gap-2">
          <div className="bg-slate-100 px-3 py-1 rounded-md text-xs font-mono text-slate-700 flex items-center gap-1.5">
            <Scale className="w-3.5 h-3.5 text-slate-500" />
            <span>Formula: <strong>Net Gain = ΔSolar - E(motor)</strong></span>
          </div>
        </div>
      </div>

      {/* 3 Metric Cards: Today, This Week, This Month */}
      <div className="grid grid-cols-1 md:grid-cols-3 gap-3 mb-4">
        {/* Today */}
        <div className="p-3.5 bg-gradient-to-br from-emerald-50/50 to-slate-50 rounded-xl border border-emerald-200/60">
          <div className="flex items-center justify-between text-xs text-slate-600 mb-1">
            <span className="font-semibold flex items-center gap-1">
              <Calendar className="w-3.5 h-3.5 text-emerald-600" />
              Today's Generation
            </span>
            <span className="font-bold text-emerald-700 bg-emerald-100/80 px-1.5 py-0.2 rounded font-mono text-[11px]">
              +30.4% NET
            </span>
          </div>
          <div className="flex items-baseline gap-2 mt-1">
            <span className="text-2xl font-black font-mono text-slate-900">14.6</span>
            <span className="text-xs font-bold text-slate-500">kWh harvested</span>
          </div>
          <div className="mt-2 text-[11px] font-mono text-slate-600 space-y-0.5 border-t border-slate-200/60 pt-1.5">
            <div className="flex justify-between">
              <span>Fixed PV Baseline:</span>
              <span className="text-slate-800">11.2 kWh</span>
            </div>
            <div className="flex justify-between text-emerald-700 font-bold">
              <span>Gross Solar Gain:</span>
              <span>+3.4 kWh</span>
            </div>
            <div className="flex justify-between text-slate-500 text-[10px]">
              <span>Motor Tracking Energy:</span>
              <span>-0.11 kWh (0.7%)</span>
            </div>
          </div>
        </div>

        {/* This Week */}
        <div className="p-3.5 bg-slate-50 rounded-xl border border-slate-200/60">
          <div className="flex items-center justify-between text-xs text-slate-600 mb-1">
            <span className="font-semibold flex items-center gap-1">
              <Calendar className="w-3.5 h-3.5 text-indigo-600" />
              This Week
            </span>
            <span className="font-bold text-indigo-700 bg-indigo-100/80 px-1.5 py-0.2 rounded font-mono text-[11px]">
              +29.3% NET
            </span>
          </div>
          <div className="flex items-baseline gap-2 mt-1">
            <span className="text-2xl font-black font-mono text-slate-900">98.4</span>
            <span className="text-xs font-bold text-slate-500">kWh harvested</span>
          </div>
          <div className="mt-2 text-[11px] font-mono text-slate-600 space-y-0.5 border-t border-slate-200/60 pt-1.5">
            <div className="flex justify-between">
              <span>Fixed PV Baseline:</span>
              <span className="text-slate-800">76.1 kWh</span>
            </div>
            <div className="flex justify-between text-emerald-700 font-bold">
              <span>Gross Solar Gain:</span>
              <span>+22.3 kWh</span>
            </div>
            <div className="flex justify-between text-slate-500 text-[10px]">
              <span>Motor Tracking Energy:</span>
              <span>-0.77 kWh (0.8%)</span>
            </div>
          </div>
        </div>

        {/* This Month */}
        <div className="p-3.5 bg-slate-50 rounded-xl border border-slate-200/60">
          <div className="flex items-center justify-between text-xs text-slate-600 mb-1">
            <span className="font-semibold flex items-center gap-1">
              <Calendar className="w-3.5 h-3.5 text-cyan-600" />
              This Month
            </span>
            <span className="font-bold text-cyan-800 bg-cyan-100/80 px-1.5 py-0.2 rounded font-mono text-[11px]">
              +28.3% NET
            </span>
          </div>
          <div className="flex items-baseline gap-2 mt-1">
            <span className="text-2xl font-black font-mono text-slate-900">412.0</span>
            <span className="text-xs font-bold text-slate-500">kWh harvested</span>
          </div>
          <div className="mt-2 text-[11px] font-mono text-slate-600 space-y-0.5 border-t border-slate-200/60 pt-1.5">
            <div className="flex justify-between">
              <span>Fixed PV Baseline:</span>
              <span className="text-slate-800">321.0 kWh</span>
            </div>
            <div className="flex justify-between text-emerald-700 font-bold">
              <span>Gross Solar Gain:</span>
              <span>+91.0 kWh</span>
            </div>
            <div className="flex justify-between text-slate-500 text-[10px]">
              <span>Motor Tracking Energy:</span>
              <span>-3.25 kWh (0.8%)</span>
            </div>
          </div>
        </div>
      </div>

      {/* Comparative Bar Chart & Ecological Offsets */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-4 items-center">
        {/* Recharts Bar Chart */}
        <div className="lg:col-span-2 h-48 w-full bg-slate-50/50 p-2 rounded-xl border border-slate-100">
          <ResponsiveContainer width="100%" height="100%">
            <BarChart data={comparisonData} margin={{ top: 10, right: 10, left: -20, bottom: 0 }}>
              <XAxis dataKey="period" stroke="#94a3b8" fontSize={11} tickLine={false} />
              <YAxis stroke="#94a3b8" fontSize={11} tickLine={false} tickFormatter={(val) => `${val} kWh`} />
              <Tooltip 
                contentStyle={{ backgroundColor: '#0f172a', border: 'none', borderRadius: '8px', color: '#fff', fontSize: '11px', fontFamily: 'monospace' }}
              />
              <Legend wrapperStyle={{ fontSize: '11px' }} />
              <Bar dataKey="tracking" name="Multi-Shaft Tracking (kWh)" fill="#10b981" radius={[4, 4, 0, 0]} />
              <Bar dataKey="fixed" name="Fixed PV Array (kWh)" fill="#94a3b8" radius={[4, 4, 0, 0]} />
            </BarChart>
          </ResponsiveContainer>
        </div>

        {/* Environmental & Economic Benefit Summary */}
        <div className="p-3.5 bg-slate-50 rounded-xl border border-slate-200/70 text-xs space-y-2.5">
          <div className="font-bold text-slate-900 flex items-center gap-1.5">
            <Leaf className="w-4 h-4 text-emerald-600" />
            <span>Ecological & Economic Impact</span>
          </div>

          <div className="space-y-1.5 text-slate-600">
            <div className="flex justify-between items-center">
              <span>CO₂ Emissions Avoided Today:</span>
              <span className="font-mono font-bold text-slate-900">~10.2 kg CO₂</span>
            </div>
            <div className="flex justify-between items-center">
              <span>Cumulative Monthly Avoided:</span>
              <span className="font-mono font-bold text-slate-900">~288 kg CO₂</span>
            </div>
            <div className="flex justify-between items-center">
              <span>Equivalent EV Mileage:</span>
              <span className="font-mono font-bold text-emerald-700">~86 km clean charge</span>
            </div>
          </div>

          <div className="p-2 bg-emerald-50/80 rounded border border-emerald-200/60 text-[11px] text-emerald-900">
            💡 <strong>Efficiency Rationale:</strong> By utilizing intermittent micro-stepping rather than continuous servo motion, motor power overhead consumes &lt;1% of the newly harvested energy.
          </div>
        </div>
      </div>
    </div>
  );
};
