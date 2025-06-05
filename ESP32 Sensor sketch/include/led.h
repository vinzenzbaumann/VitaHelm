#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// === Constantes ===
#define PIN         5
#define NUM_LEDS    50
#define BRIGHTNESS  255

// === Globale Objekte ===
extern Adafruit_NeoPixel strip;

// === Atmung ===
extern const int respirationCycle;
extern float respPhase;

// === Herzschlag ===
extern const int heartbeatInterval;
extern unsigned long lastHeartbeat;
extern bool heartbeatActive;

// === Stimmung ===
extern int moodValue;
extern int moodR, moodG, moodB;

// === Funktionen ===
void updateMoodColor(int value);
void updateLEDs();

#endif