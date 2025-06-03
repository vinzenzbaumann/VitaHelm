#include <Arduino.h>
#include "accelerometer.h"
#include "accelerometer.h"
#include <Adafruit_ADXL345_U.h>
#include <Wire.h>

// Globale Instanz des Sensors
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

void initAccelerometer() {
  if (!accel.begin()) {
    Serial.println("ADXL345 nicht gefunden – bitte Verkabelung prüfen!");
    while (true); // Endlosschleife bei Fehler
  }

  Serial.println("ADXL345 erfolgreich initialisiert.");

  // Messbereich auf +/-16g setzen
  accel.setRange(ADXL345_RANGE_16_G);
}

AccelData getAccelerometerData() {
  sensors_event_t event;
  accel.getEvent(&event);

  AccelData data;
  data.x = (int16_t)(event.acceleration.x * 100); // Skalierung: m/s² → *100 für int
  data.y = (int16_t)(event.acceleration.y * 100);
  data.z = (int16_t)(event.acceleration.z * 100);

  return data;
}
