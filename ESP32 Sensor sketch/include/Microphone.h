#ifndef MICROPHONE_H
#define MICROPHONE_H
// Mikrofon-Pegeltrigger (D0 vom KY-038 → z. B. GPIO 25)
#define MICROPHONE_DIGITAL_PIN 25

extern unsigned long expirationStart;
extern unsigned long expirationDuration; 
extern bool inExpiration;
extern int respiratoryRate;

void initMicrophone();
int readMicrophone();
int resp_rate(int Trigger);

#endif
