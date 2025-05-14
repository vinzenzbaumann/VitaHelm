#include <Arduino.h>
#include "accelerometer.h"

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
