// Predefined Libraries
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

// Selfmade Libraries
#include "oxymeter.h"
#include "network.h"
#include "accelerometer.h"
#include "Microphone.h"

// Timer
hw_timer_t *timer = NULL;
volatile bool sendD = false;

// Timer-Interrupt-Funktion
void IRAM_ATTR onTimer() {
  sendD = true;
}

// I2C-Geräte-Scanner
void scanI2CDevices() {
  Serial.println("I2C-Scan gestartet...");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("I2C-Gerät gefunden bei Adresse 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
    }
  }
  Serial.println("I2C-Scan abgeschlossen.\n");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // WLAN verbinden
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nVerbunden mit WLAN");

  // Mikrofon vorbereiten
  pinMode(MICROPHONE_DIGITAL_PIN, INPUT);

  // I2C starten (für ESP32: SDA = 21, SCL = 22)
  Wire.begin(21, 22);
  scanI2CDevices();

  // Sensoren initialisieren
  initAccelerometer();
  initMicrophone();
  oxymeterSetup();

  // Timer für 100 Hz
  timer = timerBegin(0, 80, true); // 80 MHz / 80 = 1 MHz
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 10000, true); // 10 ms
  timerAlarmEnable(timer);

  Serial.println("Programm läuft...");
}

void loop() {
  static unsigned long lastHeartbeatTime = 0;
  static unsigned long lastOxyTime = 0;
  static unsigned long lastAccTime = 0;
  static unsigned long lastMicTime = 0;
  static unsigned long lastBreathEvalTime = 0;
  static unsigned long lastDataSendTime = 0;

  static AccelData acc = {0, 0, 0};
  static int micAnalog = 0;
  static int micTrigger = 0;
  static float breathRate = 0;

  unsigned long currentMillis = millis();

  // Heartbeat alle 1000ms
  if (currentMillis - lastHeartbeatTime >= 1000) {
    udp.beginPacket(pcIP, udpPort);
    udp.write((const uint8_t *)"1", 1);
    udp.endPacket();
    lastHeartbeatTime = currentMillis;
  }

  if (sendD) {
    sendD = false;

    // Oximeter alle 100 ms
    if (currentMillis - lastOxyTime >= 100) {
      oxymeterLoop();
      lastOxyTime = currentMillis;
    }

    // Accelerometer alle 20 ms
    if (currentMillis - lastAccTime >= 20) {
      acc = getFilteredAccelerometerData();
      lastAccTime = currentMillis;
    }

    // Mikrofon analog alle 10 ms
    if (currentMillis - lastMicTime >= 10) {
      micAnalog = readMicrophoneAnalog();
      micTrigger = digitalRead(MICROPHONE_DIGITAL_PIN);
      lastMicTime = currentMillis;
    }

    // Atemfrequenz-Auswertung alle 500 ms
    if (currentMillis - lastBreathEvalTime >= 500) {
      breathRate = getBreathRateBPM();
      lastBreathEvalTime = currentMillis;
    }

    // UDP-Datenpaket nur alle 100ms senden
    if (currentMillis - lastDataSendTime >= 100) {
      char packet[128];
      snprintf(packet, sizeof(packet), "X:%d,Y:%d,Z:%d,MicTrigger:%d,MicAnalog:%d,BreathRate:%.1f,BPM:%d,AvgBPM:%d\r",
               acc.x, acc.y, acc.z, micTrigger, micAnalog, breathRate, (int)beatsPerMinute, beatAvg);

      udp.beginPacket(pcIP, udpPort);
      udp.write((uint8_t *)packet, strlen(packet));
      udp.endPacket();

      Serial.printf("\rSende Daten: X=%d, Y=%d, Z=%d, Mikrofon-Trigger: %d, Analog: %d, Atemfrequenz: %.1f, BPM=%d, Avg BPM=%d   ",
                    acc.x, acc.y, acc.z, micTrigger, micAnalog, breathRate, (int)beatsPerMinute, beatAvg);

      lastDataSendTime = currentMillis;
    }
  }
}
