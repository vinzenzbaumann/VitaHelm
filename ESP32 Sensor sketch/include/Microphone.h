#ifndef MICROPHONE_H
#define MICROPHONE_H
// Mikrofon-Pegeltrigger (D0 vom KY-038 → z. B. GPIO 25)
#define MICROPHONE_DIGITAL_PIN 25

void initMicrophone();
int readMicrophone();

#endif
