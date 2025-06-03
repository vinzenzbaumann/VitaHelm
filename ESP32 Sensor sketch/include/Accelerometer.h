#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <Arduino.h>

// Struktur zum Speichern der Achsdaten
struct AccelData {
  int16_t x;
  int16_t y;
  int16_t z;
};

// Initialisiert den ADXL345
void initAccelerometer();

// Liest aktuelle Beschleunigungsdaten (x, y, z) in g * 100
AccelData getAccelerometerData();

#endif
