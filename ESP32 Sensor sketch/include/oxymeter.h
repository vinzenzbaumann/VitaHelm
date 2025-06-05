#ifndef OXYMETER_H
#define OXYMETER_H

#include <Wire.h>
#include "MAX30105.h"

#define MX30102_ADDR 0x57
#define RATE_SIZE 4 // 

// Funktionsdeklarationen
void oxymeterSetup();
void oxymeterLoop();
void oxymeterSendData();

extern byte rates[RATE_SIZE];
extern byte rateSpot;
extern long lastBeat;
extern float beatsPerMinute;
extern int beatAvg;

#endif
