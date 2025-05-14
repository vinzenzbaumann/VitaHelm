#ifndef OXYMETER_H
#define OXYMETER_H

#include <Wire.h>
#include "MAX30105.h"

// Constants and sensor objects
#define MX30102_ADDR 0x57


// Function declarations
void oxymeterSetup();
void oxymeterLoop();
void oxymeterSendData();

#endif
 