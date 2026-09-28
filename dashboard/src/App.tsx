import React, { useState } from 'react';
import { useSolarSimulation } from './hooks/useSolarSimulation';
import { Header, type DashboardTab } from './components/Header';

import { KpiCards } from './components/KpiCards';
import { SolarPowerChart } from './components/SolarPowerChart';
import { TrackingDial } from './components/TrackingDial';
import { MultiShaftStatus } from './components/MultiShaftStatus';
import { MechanicalHealth } from './components/MechanicalHealth';
import { AiMaintenance } from './components/AiMaintenance';
import { ControlPanel } from './components/ControlPanel';
import { EnergyAnalytics } from './components/EnergyAnalytics';
import { SystemArchitecture } from './components/SystemArchitecture';
import { AlertsPanel } from './components/AlertsPanel';
import { ConceptModal } from './components/ConceptModal';
import { Footer } from './components/Footer';

export const App: React.FC = () => {
  const [activeTab, setActiveTab] = useState<DashboardTab>('overview');
  const [isConceptOpen, setIsConceptOpen] = useState<boolean>(false);

  const {
    hourDecimal,
    trackingMode,
    isEmergencyStopped,
    shafts,
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
            {/* Core Sensor KPI Cards */}
            <KpiCards
              solar={solar}
              motor={motor}
              tracking={tracking}
              ai={ai}
              faultInjected={ai.faultInjected}
            />

            {/* Generation Profile & Slat Tracking Dial */}
            <div className="grid grid-cols-1 lg:grid-cols-12 gap-6">
              <div className="lg:col-span-8">
                <SolarPowerChart
                  data={diurnalData}
                  currentHourDecimal={hourDecimal}
                />
              </div>
              <div className="lg:col-span-4">
                <TrackingDial tracking={tracking} />
              </div>
            </div>

            {/* Multi-Shaft Mechanical Synchronization */}
            <MultiShaftStatus
              shafts={shafts}
              motorMoving={motor.status === 'RUNNING'}
              actualAngle={tracking.actualShaftAngle}
            />
          </div>
        )}

        {/* Tab 2: Date-Wise Generation Analytics */}
        {activeTab === 'analytics' && (
          <div className="space-y-6">
            <EnergyAnalytics solar={solar} />
          </div>
        )}

        {/* Tab 3: System Health & Diagnostics */}
        {activeTab === 'diagnostics' && (
          <div className="space-y-6">
            <AiMaintenance
              ai={ai}
              onSelectScenario={setActiveScenario}
            />
            <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
              <MechanicalHealth
                motor={motor}
                faultInjected={ai.faultInjected}
              />
              <AlertsPanel alerts={alerts} />
            </div>
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
              isHardwareOnline={isHardwareOnline}
              onZeroCurrent={() => sendCommand('ZERO_CURR')}
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

