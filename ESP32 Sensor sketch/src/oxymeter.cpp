#include "oxymeter.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include "heartRate.h"
#include "network.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE]; 
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

void oxymeterSetup() {
  // Initialize sensor
  Serial.println("Initializing MAX30105...");

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
  }

  particleSensor.setup(); 
  particleSensor.setPulseAmplitudeRed(0x0A); 
  particleSensor.setPulseAmplitudeGreen(0); 

  Serial.println("Place your finger on the sensor.");
}

void oxymeterLoop() {
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute; 
      rateSpot %= RATE_SIZE; 

      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++) {
        beatAvg += rates[x];
      }
      beatAvg /= RATE_SIZE;
    }
  }

  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

  if (irValue < 50000)
    Serial.print(" No finger?");
  
  Serial.println();
}

void oxymeterSendData() {
  char packet[128];
  snprintf(packet, sizeof(packet), "BPM:%d,AvgBPM:%d\n", (int)beatsPerMinute, beatAvg);

  udp.beginPacket(pcIP, udpPort);
  udp.write((uint8_t*)packet, strlen(packet));
  udp.endPacket();
  
  Serial.printf("Sending Data: BPM=%d, Avg BPM=%d\n", (int)beatsPerMinute, beatAvg);
}
