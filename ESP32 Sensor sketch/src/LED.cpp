#include "LED.h"

// === Initialisierungen ===

Adafruit_NeoPixel strip(NUM_LEDS, PIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel front(FRONT_LEDS,FRONT_LED_PIN,NEO_RBG + NEO_KHZ800);

const int respirationCycle = 2000;
float respPhase = 0.0;

const int heartbeatInterval = 1000;
unsigned long lastHeartbeat = 0;
bool heartbeatActive = false;

int moodValue = 0;
uint8_t moodR = 0, moodG = 0, moodB = 255;

void initLed() {
strip.begin();
strip.setBrightness(BRIGHTNESS);
strip.show();
front.begin();
front.setBrightness(BRIGHTNESS);
front.show();
}
// === Farbe je nach Stimmungswert festlegen ===
void updateMoodColor(int value) {
  if (value < 150) {
    moodR = 0; moodG = 0; moodB = 255;
  } else if (value < 300) {
    moodR = 0; moodG = 255; moodB = 0;
  } else if (value < 450) {
    moodR = 255; moodG = 165; moodB = 0;
  } else {
    moodR = 255; moodG = 0; moodB = 0;
  }
}

// === LEDs-Effekt : Atmung + Herzschlag ===
void ledLoop() {
  unsigned long time = millis();
  moodValue = random(0, 600);
  updateMoodColor(moodValue);

  // Atmung
  float respPos = (float)(time % respirationCycle) / respirationCycle;
  float baseBrightness = (sin(respPos * TWO_PI - PI / 2) + 1.0) / 2.0;

  // Herzschlag
  if (time - lastHeartbeat > heartbeatInterval) {
    lastHeartbeat = time;
    heartbeatActive = true;
  }

  float totalBrightness = baseBrightness;
  if (heartbeatActive) {
    totalBrightness += 0.25;
    if (totalBrightness > 1.0) totalBrightness = 1.0;
  }

  // Farbe + Helligkeit
  uint8_t r = (uint8_t)(moodR * totalBrightness);
  uint8_t g = (uint8_t)(moodG * totalBrightness);
  uint8_t b = (uint8_t)(moodB * totalBrightness);

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, r, g, b);
  }
  strip.show();
  
 // Front_Leds
   for(int i = 0; i < FRONT_LEDS; i++){
        front.setPixelColor(i, 255, 255, 255);
    }
    front.show();

  // Ende des Herzschlages
  if (heartbeatActive && (time - lastHeartbeat > 100)) {
    heartbeatActive = false;
  }
}

