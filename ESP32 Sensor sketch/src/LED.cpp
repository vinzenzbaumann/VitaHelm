#include "LED.h"
#include "oxymeter.h"  // enthält heartbeatDetected
#include "Microphone.h"

Adafruit_NeoPixel strip(NUM_LEDS, PIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel front(FRONT_LEDS, FRONT_LED_PIN, NEO_RBG + NEO_KHZ800);

float breathBrightness = 0.2;  // Startwert (Minimum)
unsigned long lastBeatTime = 0;

// Atemsteuerung
const float minBrightness = 0.2f;
const float maxBreathBrightness = 0.8f;
const float inhaleStep = 0.01f;
const float exhaleStep = 0.03f;
bool lastMicState = LOW;

int moodValue = 0;
uint8_t moodR = 0, moodG = 0, moodB = 255;

void updateMoodColor(int value) {
  int index = constrain(value, 0, 599) / 100;
  switch (index) {
    case 0: moodR =   0; moodG =   0; moodB = 255; break;
    case 1: moodR = 255; moodG =   0; moodB = 255; break;
    case 2: moodR =   0; moodG = 255; moodB = 255; break;
    case 3: moodR =   0; moodG = 255; moodB =   0; break;
    case 4: moodR = 255; moodG = 255; moodB =   0; break;
    case 5: default: moodR = 255; moodG =   0; moodB =   0; break;
  }
}

void ledLoop(int moodValue) {
  unsigned long time = millis();
  updateMoodColor(moodValue);

  // Herzschlag erkannt → Helligkeit kurz auf 1.0 setzen (kurzer Puls)
  if (heartbeatDetected) {
    breathBrightness = 1.0;
    lastBeatTime = time;
    heartbeatDetected = false;
  }

  // Herzschlag-Fading (300ms lang runter zu Min 0.2)
  unsigned long timeSinceBeat = time - lastBeatTime;
  if (timeSinceBeat < 300) {
    float fade = 1.0 - (timeSinceBeat / 300.0f);
    float heartBrightness = minBrightness + 0.8f * fade;

    // BreathBrightness wird unten noch angepasst, also Merken:
    // Wir kombinieren Herzschlag und Atmung, Herzschlag hat Priorität
    if (heartBrightness > breathBrightness) {
      breathBrightness = heartBrightness;
    }
  }

  // --- ATMUNGSLLOGIK (digitaler Mikrofoneingang) ---
  bool currentMicState = digitalRead(MICROPHONE_DIGITAL_PIN);

  if (currentMicState == HIGH) {
    // Einatmen: Helligkeit langsam hoch bis max 0.8
    if (breathBrightness < maxBreathBrightness) {
      breathBrightness += inhaleStep;
      if (breathBrightness > maxBreathBrightness)
        breathBrightness = maxBreathBrightness;
    }
  } else {
    // Ausatmen: Helligkeit runter bis mind. 0.2
    if (breathBrightness > minBrightness) {
      breathBrightness -= exhaleStep;
      if (breathBrightness < minBrightness)
        breathBrightness = minBrightness;
    }
  }
  lastMicState = currentMicState;

  // Farbe mit aktueller Helligkeit setzen
  uint8_t r = (uint8_t)(moodR * breathBrightness);
  uint8_t g = (uint8_t)(moodG * breathBrightness);
  uint8_t b = (uint8_t)(moodB * breathBrightness);

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, r, g, b);
  }
  strip.show();

  // Front-LEDs konstant weiß
  for (int i = 0; i < FRONT_LEDS; i++) {
    front.setPixelColor(i, 255, 255, 255);
  }
  front.show();
}
