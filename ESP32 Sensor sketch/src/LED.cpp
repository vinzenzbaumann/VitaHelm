#include "LED.h"
#include "Microphone.h"

Adafruit_NeoPixel strip(NUM_LEDS, PIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel front(FRONT_LEDS, FRONT_LED_PIN, NEO_RBG + NEO_KHZ800);

// Mikrofon into atemlogik
const int micPin = digitalRead(MICROPHONE_DIGITAL_PIN);
bool isInhaling = false;
bool lastMicTrigger = false;
unsigned long lastSwitchTime = 0;
const unsigned long minSwitchInterval = 300;

float breathBrightness = 0.0;  // 0.0 bis 1.0
float breathSpeed = 0.01;      // 

int moodValue = 0;
uint8_t moodR = 0, moodG = 0, moodB = 255;

// === LED-Initialisierung ===
void initLed() {
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();
  front.begin();
  front.setBrightness(BRIGHTNESS);
  front.show();
}

// === Stimmung zu Farbe ===
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

// === Haupt-LED-Loop ===
void ledLoop(int moodValue) {
  unsigned long time = millis();
  updateMoodColor(moodValue);

  // Mikrofon lesen & Flanke erkennen
  bool micTrigger = digitalRead(micPin);
  if (micTrigger && !lastMicTrigger && time - lastSwitchTime > minSwitchInterval) {
    isInhaling = !isInhaling; // Zustand wechseln
    lastSwitchTime = time;
  }
  lastMicTrigger = micTrigger;

  // atmungsdarstellung
  if (isInhaling) {
    Serial.printf("                                                         %d",isInhaling);
    
    breathBrightness += breathSpeed;
    if (breathBrightness > 1.0) breathBrightness = 1.0;
  } else {
    breathBrightness -= breathSpeed;
    if (breathBrightness < 0.0) breathBrightness = 0.2;
  }

  // LED-Farbe entsprechend Helligkeit setzen
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
