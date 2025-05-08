#include <Arduino.h>
#include "Microphone.h"

#define MICROPHONE_DIGITAL_PIN 25

void initMicrophone() {
  pinMode(MICROPHONE_DIGITAL_PIN, INPUT);
}

int readMicrophone() {
  return digitalRead(MICROPHONE_DIGITAL_PIN);
}
