#include <Arduino.h>
#include "Accelerometer.h"

#define ADXL_X_PIN 32
#define ADXL_Y_PIN 33
#define ADXL_Z_PIN 34

void initAccelerometer() {
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

uint16_t readAccelerometerX() {
  return analogRead(ADXL_X_PIN);
}

uint16_t readAccelerometerY() {
  return analogRead(ADXL_Y_PIN);
}

uint16_t readAccelerometerZ() {
  return analogRead(ADXL_Z_PIN);
}
