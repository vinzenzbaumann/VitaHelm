#include <Arduino.h>
#include "Microphone.h"

static unsigned long lastTriggerTime = 0;
static unsigned long prevTriggerTime = 0;
static float breathRateBPM = 0.0;

bool  breathRisingEdge = false;
bool breathFallingEdge = false;



static bool lastMicState = LOW;

void initMicrophone() {
  pinMode(MICROPHONE_DIGITAL_PIN, INPUT);
  // Für Analog-Pin muss nichts besonderes
  lastTriggerTime = 0;
  prevTriggerTime = 0;
  breathRateBPM = 0.0;
  lastMicState = digitalRead(MICROPHONE_DIGITAL_PIN);
}


void updateBreathDetection() {

  
  bool currentState = digitalRead(MICROPHONE_DIGITAL_PIN);



  // Trigger bei steigendem Signal (LOW->HIGH)
  if (lastMicState == LOW && currentState == HIGH) {
    breathRisingEdge = true;
    prevTriggerTime = lastTriggerTime;
    lastTriggerTime = millis();

    if (prevTriggerTime != 0) {
      unsigned long interval = lastTriggerTime - prevTriggerTime;

      if (interval > 300 && interval < 5000) {
        breathRateBPM = 60000.0f / interval;
      }
    }
  }

  if (lastMicState == HIGH && currentState == LOW) {
  breathFallingEdge = true;  // Hier passiert etwas bei der fallenden Flanke
}
  lastMicState = currentState;

  


}




float getBreathRateBPM() {
  return breathRateBPM;
}

int readMicrophoneAnalog() {
  return analogRead(MICROPHONE_ANALOG_PIN);
}
