import React from 'react';
import { 
  X, 
  Cog, 
  Layers, 
  Cpu, 
  CheckCircle2, 
  ShieldAlert
} from 'lucide-react';

interface ConceptModalProps {
  isOpen: boolean;
  onClose: () => void;
}

export const ConceptModal: React.FC<ConceptModalProps> = ({ isOpen, onClose }) => {
  if (!isOpen) return null;

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-slate-900/60 backdrop-blur-xs animate-fadeIn">
      <div className="bg-white rounded-2xl max-w-3xl w-full max-h-[90vh] overflow-y-auto shadow-2xl border border-slate-200">
        {/* Modal Header */}
        <div className="sticky top-0 bg-white/95 backdrop-blur-xs border-b border-slate-200 px-6 py-4 flex items-center justify-between z-10">
          <div>
            <div className="flex items-center gap-2">
              <span className="bg-amber-100 text-amber-900 text-[10px] font-bold px-2 py-0.5 rounded uppercase tracking-wider border border-amber-300">
                Engineering Concept
              </span>
              <span className="text-xs font-mono text-slate-500">Student Innovation Project</span>
            </div>
            <h2 className="text-xl font-black text-slate-900 mt-1">
              Multi-Shaft Solar Cell Tracking System
            </h2>
          </div>
          <button
            onClick={onClose}
            className="p-1.5 rounded-lg text-slate-400 hover:text-slate-700 hover:bg-slate-100 transition"
          >
            <X className="w-5 h-5" />
          </button>
        </div>

        {/* Modal Body Content */}
        <div className="p-6 space-y-6 text-sm text-slate-700">
          {/* Core Motto Banner */}
          <div className="p-4 bg-gradient-to-r from-amber-500/10 via-emerald-500/10 to-cyan-500/10 rounded-xl border border-emerald-200/80">
            <div className="text-xs font-bold text-emerald-800 uppercase tracking-wider mb-1">
              Core Design Philosophy
            </div>
            <div className="text-xl font-black text-slate-900 tracking-tight">
              “ROTATE THE CELLS, NOT THE PANEL.”
            </div>
            <p className="text-xs text-slate-600 mt-1">
              Conventional trackers rotate entire bulky panel assemblies with large structural wind loads.
              This system rotates only individual rows of PV cells on lightweight parallel shafts while the primary supporting chassis remains stationary.
            </p>
          </div>

          {/* Key Advantages Grid */}
          <div>
            <h3 className="text-sm font-bold text-slate-900 uppercase tracking-wider mb-3 flex items-center gap-2">
              <CheckCircle2 className="w-4 h-4 text-emerald-600" />
              <span>Core Technological Innovations</span>
            </h3>
            <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
              <div className="p-3 bg-slate-50 rounded-lg border border-slate-200/70">
                <div className="font-bold text-slate-900 text-xs mb-1 flex items-center gap-1.5">
                  <Cog className="w-3.5 h-3.5 text-indigo-600" />
                  Single Motor for Multiple Shafts
                </div>
                <p className="text-xs text-slate-600">
                  A single stepper motor drives 8 parallel shafts through a common worm-drive transmission, eliminating 7 motors and drastically reducing cost and weight.
                </p>
              </div>

              <div className="p-3 bg-slate-50 rounded-lg border border-slate-200/70">
                <div className="font-bold text-slate-900 text-xs mb-1 flex items-center gap-1.5">
                  <ShieldAlert className="w-3.5 h-3.5 text-amber-600" />
                  Self-Locking Worm Gear
                </div>
                <p className="text-xs text-slate-600">
                  The high mechanical advantage worm gear prevents back-driving from wind loads, holding the cell rows rigidly without consuming holding current.
                </p>
              </div>

              <div className="p-3 bg-slate-50 rounded-lg border border-slate-200/70">
                <div className="font-bold text-slate-900 text-xs mb-1 flex items-center gap-1.5">
                  <Cpu className="w-3.5 h-3.5 text-cyan-600" />
                  Intermittent Tracking Control
                </div>
                <p className="text-xs text-slate-600">
                  Sun movement is slow; the STM32 microcontroller triggers micro-steps only every few minutes, reducing motor parasitic energy to &lt;1% of generation.
                </p>
              </div>

              <div className="p-3 bg-slate-50 rounded-lg border border-slate-200/70">
                <div className="font-bold text-slate-900 text-xs mb-1 flex items-center gap-1.5">
                  <Layers className="w-3.5 h-3.5 text-emerald-600" />
                  Flexible Torsion Wiring Loops
                </div>
                <p className="text-xs text-slate-600">
                  Special fatigue-tested service loops route cell-string conductors to avoid twisting, wire fatigue, or conductor breakage during continuous daily cycling.
                </p>
              </div>
            </div>
          </div>

          {/* AI Predictive Layer */}
          <div className="p-4 bg-purple-50/50 rounded-xl border border-purple-200/60">
            <h3 className="text-sm font-bold text-purple-900 uppercase tracking-wider mb-1.5">
              AI Analysis & Predictive Maintenance (Future Scope)
            </h3>
            <p className="text-xs text-slate-600 leading-relaxed">
              The project incorporates a supervisory AI telemetry layer designed to detect subtle mechanical degradation before physical failure occurs. By monitoring motor current signatures, worm-gear backlash, and angular shaft deviation, the system flags bearing friction, tooth wear, or cable fatigue proactively.
            </p>
          </div>

          {/* Realistic Disclaimer */}
          <div className="p-3 bg-amber-50 rounded-lg border border-amber-200 text-xs text-amber-900 space-y-1">
            <div className="font-bold">Academic Hackathon Prototype Disclaimer:</div>
            <p className="text-[11px] text-amber-800 leading-relaxed">
              This web interface is a high-fidelity front-end demonstration running physics-based simulation models. It does not connect to live hardware in this demonstration environment. Sensor readings, angles, and AI alerts are generated dynamically to demonstrate the supervisory IoT interface.
            </p>
          </div>
        </div>

        {/* Modal Footer */}
        <div className="bg-slate-50 border-t border-slate-200 px-6 py-3 flex items-center justify-between">
          <span className="text-xs font-mono text-slate-500">
            POWER IQ Prototype • Project ID: MS-ST-2026
          </span>
          <button
            onClick={onClose}
            className="px-4 py-2 bg-slate-900 hover:bg-slate-800 text-white text-xs font-semibold rounded-lg shadow-sm transition"
          >
            Close Window
          </button>
        </div>
      </div>
    </div>
  );
};
