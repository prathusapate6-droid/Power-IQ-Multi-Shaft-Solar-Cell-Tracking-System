import React, { useState } from 'react';
import { useSolarSimulation } from './hooks/useSolarSimulation';
import { Header } from './components/Header';
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
      {/* Top Header with Live Hardware Link Status */}
      <Header
        onOpenConcept={() => setIsConceptOpen(true)}
        isEmergencyStopped={isEmergencyStopped}
        isHardwareOnline={isHardwareOnline}
        isMqttConnected={isMqttConnected}
      />

      {/* Main Content Area */}
      <main className="flex-1 max-w-7xl w-full mx-auto px-4 sm:px-6 lg:px-8 py-6">
        {/* 1. System Overview KPI Cards (All Physical Sensors: Solar, Batt, Temp, Hall 0°, AI) */}
        <KpiCards
          solar={solar}
          motor={motor}
          tracking={tracking}
          ai={ai}
          faultInjected={ai.faultInjected}
        />

        {/* 2. Generation Profile & Tracking Geometry Dial */}
        <div className="grid grid-cols-1 lg:grid-cols-12 gap-6 mb-6">
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

        {/* 3. Multi-Shaft Mechanical Synchronization (Core Innovation) */}
        <MultiShaftStatus
          shafts={shafts}
          motorMoving={motor.status === 'RUNNING'}
          actualAngle={tracking.actualShaftAngle}
        />

        {/* 4. Motor Diagnostics & AI Predictive Maintenance */}
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 mb-6">
          <MechanicalHealth
            motor={motor}
            faultInjected={ai.faultInjected}
          />
          <AiMaintenance
            ai={ai}
            onSelectScenario={setActiveScenario}
          />
        </div>

        {/* 5. Control Panel (Bidirectional Physical Hardware Link) */}
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

        {/* 6. Energy Harvesting Analytics & Date-Wise History */}
        <EnergyAnalytics solar={solar} />


        {/* 7. System Architecture Mini-View (Actuation & IoT Flows) */}
        <SystemArchitecture />

        {/* 8. Live Alerts & Telemetry Event Log */}
        <AlertsPanel alerts={alerts} />
      </main>

      {/* Engineering Concept Deep-Dive Modal */}
      <ConceptModal
        isOpen={isConceptOpen}
        onClose={() => setIsConceptOpen(false)}
      />

      {/* Professional Footer */}
      <Footer />
    </div>
  );
};

export default App;
