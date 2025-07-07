#include "accelerometer.h"
#include <Adafruit_ADXL345_U.h>
#include <Wire.h>

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// Filter-Variablen für gleitenden Mittelwert
#define FILTER_SIZE 5
static AccelData filterBuffer[FILTER_SIZE];
static uint8_t filterIndex = 0;
static bool filterBufferFilled = false;

void initAccelerometer() {
  if (!accel.begin()) {
    Serial.println("ADXL345 nicht gefunden – bitte Verkabelung prüfen!");
    while (true); // Endlosschleife bei Fehler
  }

  Serial.println("ADXL345 erfolgreich initialisiert.");
  accel.setRange(ADXL345_RANGE_8_G);

  // Filter-Buffer initialisieren mit 0
  for (int i = 0; i < FILTER_SIZE; i++) {
    filterBuffer[i] = {0, 0, 0};
  }
  filterIndex = 0;
  filterBufferFilled = false;
}

AccelData getAccelerometerData() {
  sensors_event_t event;
  accel.getEvent(&event);

  AccelData data;
  data.x = (int16_t)(event.acceleration.x * 100); // m/s² * 100 als int
  data.y = (int16_t)(event.acceleration.y * 100);
  data.z = (int16_t)(event.acceleration.z * 100);

  return data;
}

AccelData getFilteredAccelerometerData() {
  AccelData newData = getAccelerometerData();

  // Filter-Buffer mit neuen Daten füllen
  filterBuffer[filterIndex] = newData;
  filterIndex = (filterIndex + 1) % FILTER_SIZE;
  if (filterIndex == 0) filterBufferFilled = true;

  // Durchschnitt berechnen
  int32_t sumX = 0, sumY = 0, sumZ = 0;
  int count = filterBufferFilled ? FILTER_SIZE : filterIndex;

  for (int i = 0; i < count; i++) {
    sumX += filterBuffer[i].x;
    sumY += filterBuffer[i].y;
    sumZ += filterBuffer[i].z;
  }

  AccelData avgData;
  if (count > 0) {
    avgData.x = (int16_t)(sumX / count);
    avgData.y = (int16_t)(sumY / count);
    avgData.z = (int16_t)(sumZ / count);
  } else {
    avgData = newData; // fallback
  }

  return avgData;
}
