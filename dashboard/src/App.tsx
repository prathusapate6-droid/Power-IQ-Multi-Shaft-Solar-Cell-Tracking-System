import React, { useState } from 'react';
import { useSolarSimulation } from './hooks/useSolarSimulation';
import { Header, type DashboardTab } from './components/Header';

import { KpiCards } from './components/KpiCards';
import { SolarPowerChart } from './components/SolarPowerChart';
import { TrackingDial } from './components/TrackingDial';
import { FaultDiagnostics } from './components/FaultDiagnostics';
import { ControlPanel } from './components/ControlPanel';
import { EnergyAnalytics } from './components/EnergyAnalytics';
import { SystemArchitecture } from './components/SystemArchitecture';
import { ConceptModal } from './components/ConceptModal';
import { Footer } from './components/Footer';

export const App: React.FC = () => {
  const [activeTab, setActiveTab] = useState<DashboardTab>('overview');
  const [isConceptOpen, setIsConceptOpen] = useState<boolean>(false);

  const {
    hourDecimal,
    trackingMode,
    isEmergencyStopped,
    motor,
    solar,
    tracking,
    ai,
    alerts,
    diurnalData,
    handleAutoToggle,
    handleJogAngle,
    handleHomePosition,
    handleEmergencyStop,
    isHardwareOnline,
    isMqttConnected,
    sendCommand,
    activeScenario,
    setActiveScenario,
  } = useSolarSimulation();

  return (
    <div className="min-h-screen bg-slate-50 text-slate-900 flex flex-col antialiased">
      {/* Top Header with Clean Tabbed Navigation */}
      <Header
        onOpenConcept={() => setIsConceptOpen(true)}
        isEmergencyStopped={isEmergencyStopped}
        isHardwareOnline={isHardwareOnline}
        isMqttConnected={isMqttConnected}
        activeTab={activeTab}
        onSelectTab={setActiveTab}
      />

      {/* Main Content Area */}
      <main className="flex-1 max-w-7xl w-full mx-auto px-4 sm:px-6 lg:px-8 py-6">
        {/* Tab 1: Live Overview */}
        {activeTab === 'overview' && (
          <div className="space-y-6">
            {/* Core Sensor KPI Cards with Large Typography */}
            <KpiCards
              solar={solar}
              motor={motor}
              tracking={tracking}
              ai={ai}
              faultInjected={ai.faultInjected}
            />

            {/* Hardware Operational Mode Strip & Slat Tilt Dial */}
            <div className="grid grid-cols-1 lg:grid-cols-12 gap-6 items-stretch">
              <div className="lg:col-span-8 flex flex-col justify-between p-5 bg-white rounded-2xl border border-slate-200 shadow-xs">
                <div>
                  <div className="flex items-center justify-between mb-3">
                    <span className="text-xs font-bold uppercase tracking-wider text-slate-500 flex items-center gap-1.5">
                      <span className={`w-2.5 h-2.5 rounded-full ${trackingMode === 'MANUAL' ? 'bg-amber-500 animate-pulse' : 'bg-emerald-500 animate-pulse'}`}></span>
                      Hardware Switch & Operational Status
                    </span>
                    <span className={`px-3 py-1 rounded-full text-xs font-black border ${
                      trackingMode === 'MANUAL'
                        ? 'bg-amber-100 text-amber-800 border-amber-300'
                        : 'bg-emerald-100 text-emerald-800 border-emerald-300'
                    }`}>
                      {trackingMode === 'MANUAL' ? '🕹️ MANUAL MODE: WITHOUT TRACKING' : '⚡ AUTO MODE: WITH TRACKING (+38.9%)'}
                    </span>
                  </div>
                  <h3 className="text-base font-black text-slate-900 mb-1">
                    {trackingMode === 'MANUAL'
                      ? 'System Operating in Manual Mode — Fixed Horizontal Baseline'
                      : 'Closed-Loop Multi-Shaft Sun Tracking Synchronized'}
                  </h3>
                  <p className="text-xs text-slate-600 leading-relaxed mb-3">
                    {trackingMode === 'MANUAL'
                      ? 'The toggle button on your hardware is set to MANUAL. Multi-shaft sun tracking is inactive. Slats are held stationary at default horizontal tilt, generating standard fixed panel output (~15W max, suffering ~38.9% harvest loss without tracking).'
                      : 'The toggle button on your hardware is set to AUTO. Continuous astronomical tracking actively keeps all 10 solar rows aligned with the sun vector, harvesting +38.9% more electrical energy than a fixed panel.'}
                  </p>
                </div>

                {/* Real-time Slat Tilt Kinematics Track */}
                <div className="my-2 p-3 bg-slate-50/90 rounded-xl border border-slate-200">
                  <div className="flex items-center justify-between text-xs font-mono mb-1.5">
                    <span className="font-bold text-slate-700 flex items-center gap-1.5">
                      <span className={`w-2 h-2 rounded-full ${trackingMode === 'MANUAL' ? 'bg-amber-500' : 'bg-emerald-500 animate-pulse'}`}></span>
                      10-Shaft Mechanical Tilt Track:
                    </span>
                    <span className="font-bold text-slate-900 bg-white px-2 py-0.5 rounded border border-slate-200">
                      Current Angle: <strong className={trackingMode === 'MANUAL' ? 'text-amber-600' : 'text-emerald-700'}>{tracking.actualShaftAngle}°</strong>
                    </span>
                  </div>
                  {/* Slider Track from -35° to +35° */}
                  <div className="relative w-full h-3 bg-slate-200 rounded-full overflow-hidden my-1.5 border border-slate-300">
                    <div 
                      className={`h-full transition-all duration-500 rounded-full ${
                        trackingMode === 'MANUAL' 
                          ? 'bg-gradient-to-r from-amber-400 to-amber-600' 
                          : 'bg-gradient-to-r from-emerald-400 via-teal-500 to-emerald-600'
                      }`}
                      style={{ 
                        width: `${Math.max(4, Math.min(100, ((tracking.actualShaftAngle + 35) / 70) * 100))}%` 
                      }}
                    />
                  </div>
                  <div className="flex justify-between text-[10px] font-mono text-slate-500 pt-0.5">
                    <span>-35° (East / Morning)</span>
                    <span>-17.5°</span>
                    <span className="font-bold text-slate-700">0° (Zenith / Noon)</span>
                    <span>+17.5°</span>
                    <span>+35° (West / Evening)</span>
                  </div>
                </div>

                {/* 4 Real-time Engineering Telemetry Parameter Tiles */}
                <div className="grid grid-cols-2 sm:grid-cols-4 gap-2 my-2 font-mono text-xs">
                  <div className="p-2.5 bg-slate-50 rounded-xl border border-slate-200">
                    <div className="text-[10px] text-slate-500">Transmission</div>
                    <div className="font-bold text-slate-900 text-xs mt-0.5">40:1 Worm Gear</div>
                    <div className="text-[9px] text-emerald-700 font-semibold mt-0.5">Self-locking</div>
                  </div>
                  <div className="p-2.5 bg-slate-50 rounded-xl border border-slate-200">
                    <div className="text-[10px] text-slate-500">Synchronized</div>
                    <div className="font-bold text-slate-900 text-xs mt-0.5">10 Shaft Rows</div>
                    <div className="text-[9px] text-emerald-700 font-semibold mt-0.5">Continuous Axle</div>
                  </div>
                  <div className="p-2.5 bg-slate-50 rounded-xl border border-slate-200">
                    <div className="text-[10px] text-slate-500">Accuracy Spec</div>
                    <div className="font-bold text-slate-900 text-xs mt-0.5">±0.1° Variance</div>
                    <div className="text-[9px] text-emerald-700 font-semibold mt-0.5">&lt;0.5° Deadband</div>
                  </div>
                  <div className="p-2.5 bg-slate-50 rounded-xl border border-slate-200">
                    <div className="text-[10px] text-slate-500">Energy Advantage</div>
                    <div className={`font-bold text-xs mt-0.5 ${trackingMode === 'MANUAL' ? 'text-amber-700' : 'text-emerald-700'}`}>
                      {trackingMode === 'MANUAL' ? '0.0% (Fixed)' : '+38.9% Boost'}
                    </div>
                    <div className="text-[9px] text-slate-500 mt-0.5">
                      {trackingMode === 'MANUAL' ? 'Loss: -38.9%' : 'vs Flat Array'}
                    </div>
                  </div>
                </div>

                <div className="pt-3 border-t border-slate-100 flex flex-wrap items-center justify-between gap-3 text-xs">
                  <div className="flex items-center gap-4 font-mono">
                    <span className="text-slate-500">Shaft Tilt: <strong className="text-slate-900">{tracking.actualShaftAngle}°</strong></span>
                    <span className="text-slate-500">Target Angle: <strong className="text-slate-900">{tracking.targetAngle}°</strong></span>
                    <span className="text-slate-500">Tracking Delta: <strong className="text-emerald-700">±{tracking.trackingError}°</strong></span>
                  </div>
                  <button
                    onClick={handleAutoToggle}
                    className={`px-3.5 py-1.5 rounded-lg text-xs font-black transition shadow-xs cursor-pointer ${
                      trackingMode === 'MANUAL'
                        ? 'bg-emerald-600 hover:bg-emerald-700 text-white'
                        : 'bg-amber-600 hover:bg-amber-700 text-white'
                    }`}
                  >
                    {trackingMode === 'MANUAL' ? 'Activate Auto Tracking (+38.9%)' : 'Test Without Tracking (Manual)'}
                  </button>
                </div>
              </div>

              <div className="lg:col-span-4">
                <TrackingDial tracking={tracking} />
              </div>
            </div>

            {/* FULL-WIDTH LARGE DEDICATED GRAPHS (Takes entire bottom screen!) */}
            <div className="w-full">
              <SolarPowerChart
                data={diurnalData}
                currentHourDecimal={hourDecimal}
                trackingMode={trackingMode}
                onToggleMode={handleAutoToggle}
                solar={solar}
              />
            </div>
          </div>
        )}

        {/* Tab 2: Date-Wise Generation Analytics */}
        {activeTab === 'analytics' && (
          <div className="space-y-6">
            <EnergyAnalytics solar={solar} />
          </div>
        )}

        {/* Tab 3: System Health & Fault Diagnostics */}
        {activeTab === 'diagnostics' && (
          <div className="space-y-6">
            <FaultDiagnostics
              ai={ai}
              alerts={alerts}
              isHardwareOnline={isHardwareOnline}
              activeScenario={activeScenario}
              onSelectScenario={setActiveScenario}
            />
            <SystemArchitecture />
          </div>
        )}

        {/* Tab 4: Control & Calibration Panel */}
        {activeTab === 'control' && (
          <div className="space-y-6">
            <ControlPanel
              trackingMode={trackingMode}
              onToggleAuto={handleAutoToggle}
              onJogAngle={handleJogAngle}
              onHomePosition={handleHomePosition}
              onEmergencyStop={handleEmergencyStop}
              isEmergencyStopped={isEmergencyStopped}
              actualAngle={tracking.actualShaftAngle}
              potAngle={tracking.potAngle}
              isHardwareOnline={isHardwareOnline}
              onZeroCurrent={() => sendCommand('ZERO_CURR')}
              onInvertMotor={() => sendCommand('INVERT')}
            />
          </div>
        )}
      </main>

      {/* Engineering Concept Deep-Dive Modal */}
      <ConceptModal
        isOpen={isConceptOpen}
        onClose={() => setIsConceptOpen(false)}
      />

      {/* Professional Industrial Footer */}
      <Footer />
    </div>
  );
};

export default App;
