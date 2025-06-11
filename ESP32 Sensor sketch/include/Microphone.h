#ifndef MICROPHONE_H
#define MICROPHONE_H

#define MICROPHONE_DIGITAL_PIN 25   // Digitaler Pin für Trigger (KY-038 D0)
#define MICROPHONE_ANALOG_PIN 34    // Neuer analoger Pin für Signalpegel (GPIO 34 ist ADC1)

void initMicrophone();

// Muss in loop() regelmäßig aufgerufen werden, um Atemfrequenz zu aktualisieren
void updateBreathDetection();

// Liefert die aktuelle Atemfrequenz (Breaths Per Minute)
float getBreathRateBPM();

// Liefert aktuellen analogen Mikrofonwert (0-4095)
int readMicrophoneAnalog();

#endif
