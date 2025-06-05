#include <Arduino.h>
#include "Microphone.h"

#define MICROPHONE_DIGITAL_PIN 25

void initMicrophone() {
  pinMode(MICROPHONE_DIGITAL_PIN, INPUT);
}

int readMicrophone() {
  return digitalRead(MICROPHONE_DIGITAL_PIN);
}


unsigned long expirationStart = 0;    
unsigned long expirationDuration = 0;   
bool inExpiration = false;
int respiratoryRate = 0;

int resp_rate(int Trigger){
 
  // Detect the start of expiration
  if (Trigger == HIGH && !inExpiration) {
    inExpiration = true;
    expirationStart = millis();
  }
  
  // Detect the end of expiration
  if (Trigger == LOW && inExpiration) {
    expirationDuration = millis() - expirationStart;
    inExpiration = false;

    unsigned long cycleDuration = expirationDuration * 2; 
    
   
    if (cycleDuration > 0) {
   
       respiratoryRate = 30000 / cycleDuration;
    
   }
  }
  return respiratoryRate;
}

