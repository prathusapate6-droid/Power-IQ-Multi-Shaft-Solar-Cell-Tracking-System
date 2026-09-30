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
                  <p className="text-xs text-slate-600 leading-relaxed mb-4">
                    {trackingMode === 'MANUAL'
                      ? 'The toggle button on your hardware is set to MANUAL. Multi-shaft sun tracking is inactive. Slats are held stationary at default horizontal tilt, generating standard fixed panel output (~15W max, suffering ~38.9% harvest loss without tracking).'
                      : 'The toggle button on your hardware is set to AUTO. Continuous astronomical tracking actively keeps all 10 solar rows aligned with the sun vector, harvesting +38.9% more electrical energy than a fixed panel.'}
                  </p>
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
