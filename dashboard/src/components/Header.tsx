import React, { useState, useEffect } from 'react';
import { 
  Sun, 
  Clock,
  Radio,
  Sliders,
  BarChart3,
  ShieldCheck,
  LayoutDashboard
} from 'lucide-react';

export type DashboardTab = 'overview' | 'analytics' | 'diagnostics' | 'control';

interface HeaderProps {
  onOpenConcept: () => void;
  isEmergencyStopped: boolean;
  isHardwareOnline?: boolean;
  isMqttConnected?: boolean;
  activeTab: DashboardTab;
  onSelectTab: (tab: DashboardTab) => void;
}

export const Header: React.FC<HeaderProps> = ({
  onOpenConcept,
  isEmergencyStopped,
  isMqttConnected = true,
  activeTab,
  onSelectTab,
}) => {
  const [currentTime, setCurrentTime] = useState<string>('');

  useEffect(() => {
    const updateTime = () => {
      const now = new Date();
      setCurrentTime(now.toLocaleTimeString('en-US', { hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: true }));
    };
    updateTime();
    const timer = setInterval(updateTime, 1000);
    return () => clearInterval(timer);
  }, []);

  return (
    <header className="bg-white border-b border-slate-200 sticky top-0 z-30 shadow-xs">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex items-center justify-between h-16 gap-4">
          
          {/* Brand & Subtitle */}
          <div className="flex items-center gap-3">
            <div className="h-9 w-9 rounded-lg bg-slate-900 flex items-center justify-center text-amber-400 shadow-xs">
              <Sun className="w-5 h-5" />
            </div>
            <div>
              <div className="flex items-center gap-2">
                <span className="text-lg font-black tracking-tight text-slate-900">
                  POWER<span className="text-amber-500">IQ</span>
                </span>
                <span className="text-[11px] font-semibold text-slate-500 border-l border-slate-200 pl-2 hidden sm:inline">
                  STM32 Multi-Shaft Solar Tracking System
                </span>
              </div>
            </div>
          </div>

          {/* Clean Navigation Tabs */}
          <nav className="hidden md:flex items-center gap-1 bg-slate-100/80 p-1 rounded-lg border border-slate-200">
            <button
              onClick={() => onSelectTab('overview')}
              className={`flex items-center gap-1.5 px-3 py-1.5 rounded-md text-xs font-semibold transition ${
                activeTab === 'overview'
                  ? 'bg-white text-slate-900 shadow-xs font-bold'
                  : 'text-slate-600 hover:text-slate-900'
              }`}
            >
              <LayoutDashboard className="w-3.5 h-3.5 text-slate-500" />
              <span>Overview</span>
            </button>

            <button
              onClick={() => onSelectTab('analytics')}
              className={`flex items-center gap-1.5 px-3 py-1.5 rounded-md text-xs font-semibold transition ${
                activeTab === 'analytics'
                  ? 'bg-white text-slate-900 shadow-xs font-bold'
                  : 'text-slate-600 hover:text-slate-900'
              }`}
            >
              <BarChart3 className="w-3.5 h-3.5 text-emerald-600" />
              <span>Analytics & History</span>
            </button>

            <button
              onClick={() => onSelectTab('diagnostics')}
              className={`flex items-center gap-1.5 px-3 py-1.5 rounded-md text-xs font-semibold transition ${
                activeTab === 'diagnostics'
                  ? 'bg-white text-slate-900 shadow-xs font-bold'
                  : 'text-slate-600 hover:text-slate-900'
              }`}
            >
              <ShieldCheck className="w-3.5 h-3.5 text-purple-600" />
              <span>Diagnostics</span>
            </button>

            <button
              onClick={() => onSelectTab('control')}
              className={`flex items-center gap-1.5 px-3 py-1.5 rounded-md text-xs font-semibold transition ${
                activeTab === 'control'
                  ? 'bg-white text-slate-900 shadow-xs font-bold'
                  : 'text-slate-600 hover:text-slate-900'
              }`}
            >
              <Sliders className="w-3.5 h-3.5 text-cyan-600" />
              <span>Controls</span>
            </button>
          </nav>

          {/* Right Status Bar */}
          <div className="flex items-center gap-2.5">
            {/* Live Link Dot */}
            <div className="flex items-center gap-1.5 px-2.5 py-1 rounded-md text-xs font-medium bg-slate-50 border border-slate-200">
              <span className={`w-2 h-2 rounded-full ${
                isEmergencyStopped ? 'bg-rose-500' : isMqttConnected ? 'bg-emerald-500 animate-pulse' : 'bg-amber-400'
              }`}></span>
              <span className="text-[11px] text-slate-700 font-mono">
                {isEmergencyStopped ? 'E-STOP' : isMqttConnected ? 'Live' : 'Standby'}
              </span>
            </div>

            {/* Cloud Status */}
            <div className="hidden sm:flex items-center gap-1.5 text-slate-500 font-mono text-[11px]">
              <Radio className="w-3 h-3 text-emerald-600" />
              <span>MQTT</span>
            </div>

            {/* Clock */}
            <div className="hidden lg:flex items-center gap-1 text-slate-600 font-mono text-xs pl-2 border-l border-slate-200">
              <Clock className="w-3.5 h-3.5 text-slate-400" />
              <span>{currentTime}</span>
            </div>

            {/* Subtle specs button */}
            <button
              onClick={onOpenConcept}
              className="text-xs text-slate-500 hover:text-slate-900 px-2 py-1 rounded border border-slate-200 hover:border-slate-300 font-medium transition"
            >
              Specs
            </button>
          </div>
        </div>

        {/* Mobile Navigation Row */}
        <div className="flex md:hidden items-center justify-around py-2 border-t border-slate-100 text-xs font-semibold">
          <button
            onClick={() => onSelectTab('overview')}
            className={`px-2 py-1 rounded ${activeTab === 'overview' ? 'text-slate-900 font-bold bg-slate-100' : 'text-slate-500'}`}
          >
            Overview
          </button>
          <button
            onClick={() => onSelectTab('analytics')}
            className={`px-2 py-1 rounded ${activeTab === 'analytics' ? 'text-slate-900 font-bold bg-slate-100' : 'text-slate-500'}`}
          >
            Analytics
          </button>
          <button
            onClick={() => onSelectTab('diagnostics')}
            className={`px-2 py-1 rounded ${activeTab === 'diagnostics' ? 'text-slate-900 font-bold bg-slate-100' : 'text-slate-500'}`}
          >
            Diagnostics
          </button>
          <button
            onClick={() => onSelectTab('control')}
            className={`px-2 py-1 rounded ${activeTab === 'control' ? 'text-slate-900 font-bold bg-slate-100' : 'text-slate-500'}`}
          >
            Controls
          </button>
        </div>
      </div>
    </header>
  );
};

