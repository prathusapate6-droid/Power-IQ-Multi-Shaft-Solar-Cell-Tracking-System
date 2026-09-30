import { useEffect, useState, useRef, useCallback } from 'react';
import mqtt from 'mqtt';

type MqttClient = ReturnType<typeof mqtt.connect>;

export interface HardwareTelemetry {
  angle: number;
  pot_angle?: number;
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

    // 1. Initial fetch from Firebase Realtime Database for instant data on page load
    fetch('https://engineering-project-hub-default-rtdb.firebaseio.com/power_iq/telemetry.json')
      .then((res) => res.json())
      .then((data) => {
        if (data && typeof data === 'object' && data.solar_voltage !== undefined) {
          if (!lastPacketRef.current || Date.now() - lastPacketRef.current > 4000) {
            setTelemetry(data as HardwareTelemetry);
            setIsHardwareOnline(data.stm32_online ?? true);
          }
        }
      })
      .catch(() => {});

    // 2. Hardware watchdog + Firebase Cloud fallback
    const watchdogTimer = setInterval(() => {
      const now = Date.now();
      // Only declare hardware offline if no packet from MQTT or Firebase for > 12 seconds
      if (lastPacketRef.current > 0 && now - lastPacketRef.current > 12000) {
        setIsHardwareOnline(false);
      }
      // If no recent MQTT packet in last 3.5 seconds, fetch live state from Firebase RTDB
      if (!lastPacketRef.current || now - lastPacketRef.current > 3500) {
        fetch('https://engineering-project-hub-default-rtdb.firebaseio.com/power_iq/telemetry.json')
          .then((res) => res.json())
          .then((data) => {
            if (data && typeof data === 'object' && data.solar_voltage !== undefined) {
              const fetchTime = Date.now();
              lastPacketRef.current = fetchTime;
              setLastPacketTime(fetchTime);
              setTelemetry(data as HardwareTelemetry);
              setIsHardwareOnline(data.stm32_online !== false);
            }
          })
          .catch(() => {});
      }
    }, 3000);

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
