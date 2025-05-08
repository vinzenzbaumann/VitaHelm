#include <Wire.h>
#include "MX30102.h"

#define MX30102_ADDR 0xAE  // Adresse des MAX30102

void initMX30102() {
  Wire.begin();
}

int readPulse() {
  uint8_t data[6] = {0};
  Wire.beginTransmission(MX30102_ADDR);
  Wire.write(0x00);  // Standardbefehl (prüfen im Datenblatt)
  Wire.endTransmission();
  delay(100);
  Wire.requestFrom(MX30102_ADDR, 6);
  if (Wire.available() >= 6) {
    for (int i = 0; i < 6; i++) {
      data[i] = Wire.read();
    }
    return data[0];  // Pulswert
  }
  return -1;
}

int readOxygenSaturation() {
  uint8_t data[6] = {0};
  Wire.beginTransmission(MX30102_ADDR);
  Wire.write(0x00);  // Standardbefehl (prüfen im Datenblatt)
  Wire.endTransmission();
  delay(100);
  Wire.requestFrom(MX30102_ADDR, 6);
  if (Wire.available() >= 6) {
    for (int i = 0; i < 6; i++) {
      data[i] = Wire.read();
    }
    return data[1];  // Sauerstoffsättigungswert
  }
  return -1;
}
