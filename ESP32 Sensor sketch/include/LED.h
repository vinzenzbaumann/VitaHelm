#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// === Konstanten ===
#define PIN           18
#define FRONT_LED_PIN 17
#define NUM_LEDS      18
#define FRONT_LEDS    3
#define BRIGHTNESS    255

// === Globale Objekte ===
extern Adafruit_NeoPixel strip;

// === Stimmung ===
extern int moodValue;

// === Funktionen ===
void updateMoodColor(int value);
void ledLoop(int moodValue);
void initLed();

#endif
