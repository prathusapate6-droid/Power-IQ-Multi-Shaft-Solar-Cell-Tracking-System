import React from 'react';
import { 
  Compass, 
  Timer
} from 'lucide-react';
import type { TrackingGeometry } from '../types/dashboard';

interface TrackingDialProps {
  tracking: TrackingGeometry;
}

export const TrackingDial: React.FC<TrackingDialProps> = ({ tracking }) => {
  // Convert angles to arc coordinates
  const getCoordinates = (angleDeg: number, radius: number = 85) => {
    // Map -70°..+70° to SVG polar angles: 0° is zenith (top), -70 is left, +70 is right
    // Top in SVG is angle -90° (or 270°)
    const rad = ((angleDeg - 90) * Math.PI) / 180;
    const cx = 110;
    const cy = 115;
    return {
      x: cx + radius * Math.cos(rad),
      y: cy + radius * Math.sin(rad),
    };
  };

  const sunCoords = getCoordinates(tracking.targetAngle, 85);
  const shaftCoords = getCoordinates(tracking.actualShaftAngle, 72);

  return (
    <div className="bg-white rounded-xl p-5 border border-slate-200/90 shadow-xs flex flex-col justify-between">
      <div>
        <div className="flex items-center justify-between mb-3">
          <div className="flex items-center gap-2">
            <div className="p-1.5 rounded-lg bg-cyan-50 text-cyan-700 border border-cyan-200/60">
              <Compass className="w-4 h-4" />
            </div>
            <div>
              <h2 className="text-base font-bold text-slate-900">Slat Tilt & Sun Position</h2>
              <p className="text-xs text-slate-500">Angle kinematics vs solar vector</p>
            </div>
          </div>
          <span className="text-[11px] font-mono bg-cyan-50 text-cyan-800 border border-cyan-200 px-2 py-0.5 rounded-md font-semibold">
            {tracking.trackingMode}
          </span>
        </div>


        {/* Visual Celestial Semi-Circular Gauge */}
        <div className="relative flex items-center justify-center my-1">
          <svg viewBox="0 0 220 135" className="w-56 h-36">
            {/* Horizon ground line */}
            <line x1="20" y1="115" x2="200" y2="115" stroke="#e2e8f0" strokeWidth="2" strokeDasharray="3 3" />
            <text x="22" y="128" fill="#94a3b8" fontSize="8" fontFamily="monospace">EAST (-60°)</text>
            <text x="160" y="128" fill="#94a3b8" fontSize="8" fontFamily="monospace">WEST (+60°)</text>
            <text x="96" y="16" fill="#94a3b8" fontSize="8" fontFamily="monospace">ZENITH (0°)</text>

            {/* Celestial Arc */}
            <path
              d="M 25 115 A 85 85 0 0 1 195 115"
              fill="none"
              stroke="#f1f5f9"
              strokeWidth="12"
              strokeLinecap="round"
            />
            {/* Optimal Range Arc */}
            <path
              d="M 38 100 A 85 85 0 0 1 182 100"
              fill="none"
              stroke="#e0f2fe"
              strokeWidth="4"
              strokeDasharray="2 4"
            />

            {/* Sun Ray vector to pivot */}
            <line
              x1={sunCoords.x}
              y1={sunCoords.y}
              x2="110"
              y2="115"
              stroke="#fde68a"
              strokeWidth="2"
              strokeDasharray="4 2"
            />

            {/* Sun Icon Indicator on Arc */}
            <circle
              cx={sunCoords.x}
              cy={sunCoords.y}
              r="10"
              fill="#fbbf24"
              stroke="#f59e0b"
              strokeWidth="2"
              className="animate-pulse"
            />
            <circle
              cx={sunCoords.x}
              cy={sunCoords.y}
              r="4"
              fill="#fff"
            />

            {/* Solar Cell Shaft Indicator (Pointer arm) */}
            <line
              x1="110"
              y1="115"
              x2={shaftCoords.x}
              y2={shaftCoords.y}
              stroke="#0f766e"
              strokeWidth="4"
              strokeLinecap="round"
            />
            {/* Solar cell slat crosshead */}
            <circle cx="110" cy="115" r="5" fill="#0f766e" />

            {/* Angular text readout */}
            <text x="110" y="105" textAnchor="middle" fill="#0f172a" fontSize="13" fontWeight="bold" fontFamily="monospace">
              {tracking.actualShaftAngle > 0 ? `+${tracking.actualShaftAngle}°` : `${tracking.actualShaftAngle}°`}
            </text>
          </svg>
        </div>
      </div>

      {/* Numerical Telemetry Metrics Grid */}
      <div className="grid grid-cols-2 gap-2 text-xs font-mono border-t border-slate-100 pt-3 mt-1">
        <div className="bg-slate-50 p-2 rounded-lg border border-slate-200/60">
          <div className="text-slate-500 text-[10px]">Sun Position</div>
          <div className="font-bold text-slate-800 text-xs">
            Elev: {tracking.sunElevation}° | Azim: {tracking.sunAzimuth}°
          </div>
        </div>

        <div className="bg-slate-50 p-2 rounded-lg border border-slate-200/60">
          <div className="text-slate-500 text-[10px]">
            {tracking.trackingMode === 'MANUAL' ? 'Manual Target vs Cell' : 'Target vs Actual'}
          </div>
          <div className="font-bold text-slate-800 text-xs flex items-center justify-between">
            {tracking.trackingMode === 'MANUAL' && tracking.potAngle !== undefined ? (
              <>
                <span className="text-purple-700">
                  Pot: {tracking.potAngle > 0 ? `+${tracking.potAngle}°` : `${tracking.potAngle}°`}
                  {tracking.potAngle === 0 ? ' [0°]' : tracking.potAngle > 0 ? ' [R]' : ' [L]'}
                </span>
                <span className="text-emerald-700">
                  Cell: {tracking.actualShaftAngle > 0 ? `+${tracking.actualShaftAngle}°` : `${tracking.actualShaftAngle}°`}
                </span>
              </>
            ) : (
              <>
                <span>T: {tracking.targetAngle > 0 ? `+${tracking.targetAngle}°` : `${tracking.targetAngle}°`}</span>
                <span className="text-emerald-700">A: {tracking.actualShaftAngle > 0 ? `+${tracking.actualShaftAngle}°` : `${tracking.actualShaftAngle}°`}</span>
              </>
            )}
          </div>
        </div>

        <div className="bg-slate-50 p-2 rounded-lg border border-slate-200/60">
          <div className="text-slate-500 text-[10px]">Tracking Error</div>
          <div className="font-bold text-xs flex items-center gap-1">
            <span className={tracking.trackingError < 0.25 ? 'text-emerald-600' : 'text-amber-600'}>
              Δ {tracking.trackingError}°
            </span>
            <span className="text-[10px] text-slate-400 font-normal">(&lt;0.5° spec)</span>
          </div>
        </div>

        <div className="bg-slate-50 p-2 rounded-lg border border-slate-200/60">
          <div className="text-slate-500 text-[10px]">Step Cycle Countdown</div>
          <div className="font-bold text-slate-800 text-xs flex items-center gap-1">
            <Timer className="w-3 h-3 text-cyan-600" />
            <span>
              {tracking.isAdjusting ? 'STEPPING NOW' : `${tracking.intermittentCountdownSec}s to next step`}
            </span>
          </div>
        </div>
      </div>
    </div>
  );
};
