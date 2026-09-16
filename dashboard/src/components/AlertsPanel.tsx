import React from 'react';
import { 
  Bell, 
  CheckCircle2, 
  AlertTriangle, 
  Info, 
  AlertCircle, 
  Clock, 
  CheckCheck
} from 'lucide-react';
import type { SystemAlert } from '../types/dashboard';

interface AlertsPanelProps {
  alerts: SystemAlert[];
}

export const AlertsPanel: React.FC<AlertsPanelProps> = ({ alerts }) => {
  return (
    <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs mb-6">
      <div className="flex items-center justify-between mb-3">
        <div className="flex items-center gap-2">
          <div className="p-1.5 rounded-lg bg-slate-100 text-slate-700 border border-slate-200">
            <Bell className="w-4 h-4" />
          </div>
          <div>
            <h2 className="text-base font-bold text-slate-900">
              System Alerts & Diagnostics Feed
            </h2>
            <p className="text-xs text-slate-500">Live operational telemetry status notifications</p>
          </div>
        </div>

        <div className="flex items-center gap-2 text-xs">
          <span className="inline-flex items-center gap-1 text-emerald-700 font-semibold bg-emerald-50 px-2 py-0.5 rounded border border-emerald-200 font-mono text-[11px]">
            <CheckCheck className="w-3.5 h-3.5" />
            0 Critical Faults
          </span>
        </div>
      </div>

      {/* Quick Status Chips */}
      <div className="grid grid-cols-2 sm:grid-cols-4 gap-2 mb-3 text-xs">
        <div className="bg-slate-50 px-2.5 py-1.5 rounded-lg border border-slate-200/60 flex items-center gap-1.5 text-slate-700">
          <CheckCircle2 className="w-3.5 h-3.5 text-emerald-600 flex-shrink-0" />
          <span className="truncate">Motor Temp Normal (38°C)</span>
        </div>

        <div className="bg-slate-50 px-2.5 py-1.5 rounded-lg border border-slate-200/60 flex items-center gap-1.5 text-slate-700">
          <CheckCircle2 className="w-3.5 h-3.5 text-emerald-600 flex-shrink-0" />
          <span className="truncate">Shaft Sync Normal (±0.1°)</span>
        </div>

        <div className="bg-slate-50 px-2.5 py-1.5 rounded-lg border border-slate-200/60 flex items-center gap-1.5 text-slate-700">
          <CheckCircle2 className="w-3.5 h-3.5 text-emerald-600 flex-shrink-0" />
          <span className="truncate">Worm Backlash &lt; 0.05°</span>
        </div>

        <div className="bg-slate-50 px-2.5 py-1.5 rounded-lg border border-slate-200/60 flex items-center gap-1.5 text-slate-700">
          <CheckCircle2 className="w-3.5 h-3.5 text-emerald-600 flex-shrink-0" />
          <span className="truncate">Tracking Operating Normal</span>
        </div>
      </div>

      {/* Scrollable Event Feed */}
      <div className="space-y-2 max-h-48 overflow-y-auto pr-1">
        {alerts.map((alert) => {
          let badgeColor = 'bg-blue-50 text-blue-700 border-blue-200';
          let icon = <Info className="w-4 h-4 text-blue-600" />;

          if (alert.type === 'success') {
            badgeColor = 'bg-emerald-50 text-emerald-700 border-emerald-200';
            icon = <CheckCircle2 className="w-4 h-4 text-emerald-600" />;
          } else if (alert.type === 'warning') {
            badgeColor = 'bg-amber-50 text-amber-800 border-amber-200';
            icon = <AlertTriangle className="w-4 h-4 text-amber-600" />;
          } else if (alert.type === 'error') {
            badgeColor = 'bg-rose-50 text-rose-800 border-rose-200';
            icon = <AlertCircle className="w-4 h-4 text-rose-600" />;
          }

          return (
            <div
              key={alert.id}
              className={`p-2.5 rounded-lg border flex items-start gap-2.5 text-xs transition ${badgeColor}`}
            >
              <div className="flex-shrink-0 mt-0.5">{icon}</div>
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
              <span className="text-[9px] font-mono font-semibold bg-white/70 px-1.5 py-0.5 rounded border border-slate-200/60 text-slate-600 hidden sm:inline">
                {alert.component}
              </span>
            </div>
          );
        })}
      </div>
    </div>
  );
};
