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

// UDP


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

  // Beschleunigungssensor initialisieren
  initAccelerometer();

  // Timer für 100 Hz
  timer = timerBegin(0, 80, true);             // 80 MHz / 80 = 1 MHz → 1 tick = 1 µs
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 10000, true);         // 10.000 µs = 10 ms → 100 Hz
  timerAlarmEnable(timer);

  // Optional: oxymeterSetup();

  Serial.println("Programm läuft...");
}

void loop() {
  static unsigned long lastSendTime = 0;
  unsigned long currentMillis = millis();

  // Regelmäßiger Heartbeat an PC (alle 1000ms)
  if (currentMillis - lastSendTime >= 1000) {
    udp.beginPacket(pcIP, udpPort);
    udp.write((const uint8_t *)"1", 1);
    udp.endPacket();
    Serial.println("Sende: 1");
    lastSendTime = currentMillis;
  }

  // Hauptdatenversand bei Timer-Trigger
  if (sendD) {
    sendD = false;

    AccelData acc = getAccelerometerData();  // Digital auslesen
    int micTrigger = digitalRead(MICROPHONE_DIGITAL_PIN);

    char packet[128];
    snprintf(packet, sizeof(packet), "X:%d,Y:%d,Z:%d,MicTrigger:%d\n",
             acc.x, acc.y, acc.z, micTrigger);

    udp.beginPacket(pcIP, udpPort);
    udp.write((uint8_t *)packet, strlen(packet));
    udp.endPacket();

    Serial.printf("Sende Daten: X=%d, Y=%d, Z=%d, Mikrofon-Trigger: %d\n",
                  acc.x, acc.y, acc.z, micTrigger);
  }
}
