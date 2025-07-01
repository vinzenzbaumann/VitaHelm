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
  // Clamp value to [0…599], dann in 100er-Schritte zerlegen (0–5)
  int index = constrain(value, 0, 599) / 100;

  switch (index) {
    case 0: // 0–99   : kräftiges Blau
      moodR =   0; moodG =   0; moodB = 255;
      break;
    case 1: // 100–199: Magenta
      moodR = 255; moodG =   0; moodB = 255;
      break;
    case 2: // 200–299: Cyan
      moodR =   0; moodG = 255; moodB = 255;
      break;
    case 3: // 300–399: Grün
      moodR =   0; moodG = 255; moodB =   0;
      break;
    case 4: // 400–499: Gelb
      moodR = 255; moodG = 255; moodB =   0;
      break;
    case 5: // 500–599: Rot
    default:
      moodR = 255; moodG =   0; moodB =   0;
      break;
  }
}


// === LEDs-Effekt : Atmung + Herzschlag ===
void ledLoop(int moodValue) {
  unsigned long time = millis();
  //moodValue =  200;
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
  uint8_t r = (uint8_t)(moodG * totalBrightness); //rot und grün Werte invertiert
  uint8_t g = (uint8_t)(moodR * totalBrightness);
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

