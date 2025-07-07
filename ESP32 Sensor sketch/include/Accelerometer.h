#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <Arduino.h>

// Struktur zum Speichern der Achsdaten (in 0.01 g Einheiten)
struct AccelData {
  int16_t x;
  int16_t y;
  int16_t z;
};

// Initialisiert den ADXL345
void initAccelerometer();

// Liest aktuelle Roh-Beschleunigungsdaten (x,y,z)
AccelData getAccelerometerData();

// Liest gefilterte Beschleunigungsdaten (gleitender Mittelwert)
AccelData getFilteredAccelerometerData();

#endif
