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
 * Generate 24-hour / diurnal solar power generation profile
 */
export function generateDiurnalCurve(currentHourDecimal: number): HourlyGenerationPoint[] {
  const points: HourlyGenerationPoint[] = [];

  for (let h = 6; h <= 18; h += 0.5) {
    const timeStr = `${Math.floor(h).toString().padStart(2, '0')}:${h % 1 === 0 ? '00' : '30'}`;
    const daylightFraction = (h - 6) / 12;
    const sunSin = Math.sin(daylightFraction * Math.PI);

    if (sunSin <= 0) {
      points.push({
        time: timeStr,
        hour: h,
        trackingKw: 0,
        fixedKw: 0,
        trackingW: 0,
        fixedW: 0,
        motorW: 0,
        sunElevation: 0,
      });
      continue;
    }

    // Solar tracking maintains normal incidence -> broader, fuller generation curve
    // Fixed PV suffers cosine loss in morning/afternoon -> narrower peak curve
    const trackingKw = Number((Math.pow(sunSin, 0.62) * 3.12).toFixed(2));
    const fixedKw = Number((Math.pow(sunSin, 1.45) * 2.38).toFixed(2));
    const trackingW = Number((trackingKw * 1000).toFixed(1));
    const fixedW = Number((fixedKw * 1000).toFixed(1));
    
    // Intermittent motor active during small adjustment windows (e.g. 40W active, 4W standby)
    const isStepTime = (h * 10) % 5 === 0;
    const motorW = isStepTime ? 38 : 3.8;

    points.push({
      time: timeStr,
      hour: h,
      trackingKw: h <= currentHourDecimal ? trackingKw : Number((trackingKw * 0.98).toFixed(2)),
      fixedKw: h <= currentHourDecimal ? fixedKw : Number((fixedKw * 0.98).toFixed(2)),
      trackingW: h <= currentHourDecimal ? trackingW : Number((trackingW * 0.98).toFixed(1)),
      fixedW: h <= currentHourDecimal ? fixedW : Number((fixedW * 0.98).toFixed(1)),
      motorW,
      sunElevation: Number((sunSin * 72).toFixed(1)),
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
