#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// Konfiguration
#define PIN         5
#define PIXELCOUNT  32
#define BRIGHTNESS  255
#define HEARTBEAT_MS 1000

// Öffentliche Funktionen
void initLEDs();
void updateMoodValue(int value);
void pulseAnimation();

#endif