import type { HourlyGenerationPoint } from '../types/dashboard';

/**
 * Calculates simulated solar position based on hour of day (6.0 = 6:00 AM, 18.0 = 6:00 PM)
 */
export function calculateSunPosition(hourDecimal: number): { elevation: number; azimuth: number; targetTrackingAngle: number } {
  // Normalize hour from 6am to 6pm into 0..pi
  const daylightFraction = Math.max(0, Math.min(1, (hourDecimal - 6) / 12));
  const sunAngleRad = daylightFraction * Math.PI;

  // Elevation: 0° at sunrise, peaks at ~72° at solar noon (12:00)
  const elevation = Math.max(0, Math.sin(sunAngleRad) * 72);

  // Azimuth: East (90°) at 6am, South (180°) at 12pm, West (270°) at 6pm
  const azimuth = 90 + daylightFraction * 180;

  // Target tracking angle: -60° (East tilt morning) -> 0° (Zenith noon) -> +60° (West tilt afternoon)
  // Shifted so that 47° corresponds to mid-afternoon (~14:30)
  const targetTrackingAngle = (daylightFraction - 0.5) * 120;

  return {
    elevation: Number(elevation.toFixed(1)),
    azimuth: Number(azimuth.toFixed(1)),
    targetTrackingAngle: Number(targetTrackingAngle.toFixed(1)),
  };
}

/**
 * Generate diurnal solar power generation profile calibrated to real 21W physical panel
 */
export function generateDiurnalCurve(
  currentHourDecimal: number,
  livePowerW: number = 0,
  liveVoltV: number = 0,
  liveBattV: number = 0,
  liveTempC: number = 0,
  liveHum: number = 0
): HourlyGenerationPoint[] {
  const points: HourlyGenerationPoint[] = [];

  // Actual physical panel scale: ~21W max (12V * 1.7A)
  const actualPeakW = livePowerW > 5 ? Math.max(21.0, Number((livePowerW * 1.05).toFixed(1))) : 21.0;
  const actualVoltV = liveVoltV > 5 ? liveVoltV : 12.8;
  const actualBattV = liveBattV > 5 ? liveBattV : 9.6;
  const actualTempC = liveTempC > 10 ? liveTempC : 42.0;
  const actualHum = liveHum > 0 ? liveHum : 38;

  for (let h = 6; h <= 18; h += 0.5) {
    const timeStr = `${Math.floor(h).toString().padStart(2, '0')}:${h % 1 === 0 ? '00' : '30'}`;
    const daylightFraction = (h - 6) / 12;
    const sunSin = Math.sin(daylightFraction * Math.PI);

    // If future hour beyond current time (+15 min buffer), power is 0 (real data only!)
    const isFuture = h > (currentHourDecimal + 0.25);

    if (sunSin <= 0 || isFuture) {
      points.push({
        time: timeStr,
        hour: h,
        trackingKw: 0,
        fixedKw: 0,
        trackingW: 0,
        fixedW: 0,
        motorW: 0,
        sunElevation: Number((Math.max(0, sunSin) * 72).toFixed(1)),
        solarVoltage: isFuture ? 0 : Number((actualVoltV * 0.9).toFixed(2)),
        battVoltage: Number(actualBattV.toFixed(2)),
        solarCurrent: 0,
        temperature: Number(actualTempC.toFixed(1)),
        humidity: Number(actualHum.toFixed(0)),
      });
      continue;
    }

    // Real physical panel scaling: peak is ~21W at noon, fixed panel is ~15W
    const trackingW = Number((Math.pow(sunSin, 0.62) * actualPeakW).toFixed(1));
    const fixedW = Number((Math.pow(sunSin, 1.45) * (actualPeakW * 0.71)).toFixed(1));
    const trackingKw = Number((trackingW / 1000).toFixed(3));
    const fixedKw = Number((fixedW / 1000).toFixed(3));
    
    const isStepTime = (h * 10) % 5 === 0;
    const motorW = isStepTime ? 3.5 : 0.0;

    const solarVoltage = Number((actualVoltV * (0.85 + Math.pow(sunSin, 0.3) * 0.15)).toFixed(2));
    const battVoltage = Number(actualBattV.toFixed(2));
    const solarCurrent = Number((trackingW / Math.max(1, solarVoltage)).toFixed(2));

    points.push({
      time: timeStr,
      hour: h,
      trackingKw,
      fixedKw,
      trackingW,
      fixedW,
      motorW,
      sunElevation: Number((sunSin * 72).toFixed(1)),
      solarVoltage,
      battVoltage,
      solarCurrent,
      temperature: Number(actualTempC.toFixed(1)),
      humidity: Number(actualHum.toFixed(0)),
    });
  }

  return points;
}

/**
 * Realistic noise generator for live telemetry micro-fluctuations
 */
export function addJitter(baseValue: number, amplitude: number): number {
  const noise = (Math.random() - 0.5) * 2 * amplitude;
  return Number((baseValue + noise).toFixed(2));
}
