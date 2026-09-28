import { useEffect, useState, useRef, useCallback } from 'react';
import mqtt from 'mqtt';

type MqttClient = ReturnType<typeof mqtt.connect>;

export interface HardwareTelemetry {
  angle: number;
  mode: string;
  homed: boolean | number;
  solar_voltage: number;
  solar_current: number;
  solar_power: number;
  batt_voltage: number;
  temperature: number;
  humidity: number;
  energy_wh: number;
  stm32_online: boolean;
  uptime?: number;
}

export function useHardwareMqtt() {
  const [isMqttConnected, setIsMqttConnected] = useState<boolean>(false);
  const [isHardwareOnline, setIsHardwareOnline] = useState<boolean>(false);
  const [telemetry, setTelemetry] = useState<HardwareTelemetry | null>(null);
  const [lastPacketTime, setLastPacketTime] = useState<number>(0);

  const clientRef = useRef<MqttClient | null>(null);
  const lastPacketRef = useRef<number>(0);

  useEffect(() => {
    // HiveMQ Dedicated Cloud MQTT Broker (Secure WebSocket on port 8884 with TLS)
    const brokerUrl = 'wss://e5c6d611df63436992755767b6967071.s1.eu.hivemq.cloud:8884/mqtt';
    const clientId = `power_iq_web_${Math.random().toString(16).substring(2, 8)}`;

    const client = mqtt.connect(brokerUrl, {
      clientId,
      username: 'smartwater',
      password: 'SmartWater2026!',
      clean: true,
      connectTimeout: 10000,
      reconnectPeriod: 4000,
    });

    clientRef.current = client;

    client.on('connect', () => {
      console.log('[MQTT] Connected to HiveMQ Broker (wss://broker.hivemq.com:8884/mqtt)');
      setIsMqttConnected(true);

      // Subscribe to real-time physical telemetry
      client.subscribe('power_iq_sih2026/telemetry', (err) => {
        if (err) {
          console.error('[MQTT] Subscription error on telemetry topic:', err);
        } else {
          console.log('[MQTT] Successfully subscribed to power_iq_sih2026/telemetry');
        }
      });
    });

    client.on('message', (topic: string, message: { toString: () => string }) => {
      if (topic === 'power_iq_sih2026/telemetry') {
        try {
          const payload = JSON.parse(message.toString()) as HardwareTelemetry;
          const now = Date.now();
          lastPacketRef.current = now;
          setLastPacketTime(now);
          setTelemetry(payload);
          setIsHardwareOnline(payload.stm32_online ?? true);
        } catch (e) {
          console.error('[MQTT] Failed to parse hardware packet:', e);
        }
      }
    });

    client.on('close', () => {
      setIsMqttConnected(false);
    });

    client.on('error', (err: unknown) => {
      console.error('[MQTT] Broker connection error:', err);
      setIsMqttConnected(false);
    });

    // Hardware watchdog: marks offline if no packet received within 6 seconds
    const watchdogTimer = setInterval(() => {
      if (lastPacketRef.current > 0 && Date.now() - lastPacketRef.current > 6000) {
        setIsHardwareOnline(false);
      }
    }, 2000);

    return () => {
      clearInterval(watchdogTimer);
      client.end(true);
    };
  }, []);

  const sendCommand = useCallback((cmd: string): boolean => {
    if (clientRef.current && clientRef.current.connected) {
      console.log('[MQTT] Dispatched remote command:', cmd);
      clientRef.current.publish('power_iq_sih2026/commands', cmd);
      return true;
    }
    console.warn('[MQTT] Client not connected. Cannot publish command:', cmd);
    return false;
  }, []);

  return {
    isMqttConnected,
    isHardwareOnline,
    telemetry,
    lastPacketTime,
    sendCommand,
  };
}
