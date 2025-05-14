#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H


// Pin definition
#define ADXL_X_PIN 32
#define ADXL_Y_PIN 33
#define ADXL_Z_PIN 34


void initAccelerometer();
uint16_t readAccelerometerX();
uint16_t readAccelerometerY();
uint16_t readAccelerometerZ();

#endif
