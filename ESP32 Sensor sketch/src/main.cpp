//predef Bibs
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

//Selfmade Bibs
#include "oxymeter.h"
#include "network.h"
#include "accelerometer.h"
#include "Microphone.h"



// Timer
hw_timer_t *timer = NULL;
volatile bool sendD = false;

void IRAM_ATTR onTimer() {
  sendD = true;
}

// I2C-Scanner
void scanI2CDevices() {
  Serial.println("I2C-Scan startet...");
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
  WiFi.begin("TI Roboter", "ITRobot!");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("Verbunden mit WLAN");

  // ADXL335 vorbereiten
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  pinMode(MICROPHONE_DIGITAL_PIN, INPUT);

  // I2C starten (ESP32: SDA = 21, SCL = 22)
  Wire.begin(21, 22);
  scanI2CDevices();  // Scan beim Start durchführen

  // Timer auf 100 Hz
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 10000, true);
  timerAlarmEnable(timer);

  // MAX30105 Setup
  oxymeterSetup();

  Serial.println("Programm läuft...");
}

void loop() {
  static unsigned long lastSendTime = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastSendTime >= 1000) {
    udp.beginPacket(pcIP, udpPort);
    udp.write((const uint8_t*)"1", 1);
    udp.endPacket();
    Serial.println("Sende: 1");
    lastSendTime = currentMillis;
  }

  if (sendData) {
    uint16_t x = analogRead(ADXL_X_PIN);
    uint16_t y = analogRead(ADXL_Y_PIN);
    uint16_t z = analogRead(ADXL_Z_PIN);
    
    int micTrigger = digitalRead(MICROPHONE_DIGITAL_PIN);

    // Call Oxymeter loop to read data
    oxymeterLoop();
    
    // Send data via UDP
    oxymeterSendData();

    char packet[128];
    snprintf(packet, sizeof(packet), "X:%u,Y:%u,Z:%u,MicTrigger:%d\n", x, y, z, micTrigger);

    udp.beginPacket(pcIP, udpPort);
    udp.write((uint8_t*)packet, strlen(packet));
    udp.endPacket();

    Serial.printf("Sende Daten: X=%u, Y=%u, Z=%u, Mikrofon-Trigger: %d\n",
                  x, y, z, micTrigger);

    sendD = false;
  }
}
