
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define PIN         5
#define NUM_LEDS    50
#define BRIGHTNESS  255

Adafruit_NeoPixel strip(NUM_LEDS, PIN, NEO_RGB + NEO_KHZ800);

// === Respiration (simulée) ===
const int respirationCycle = 2000;
float respPhase = 0.0;

// === Battement de cœur (simulé) ===
const int heartbeatInterval = 1000;
unsigned long lastHeartbeat = 0;
bool heartbeatActive = false;

// === Valeur d'humeur reçue du PC ===
int moodValue = 0;
uint8_t moodR = 0, moodG = 0, moodB = 255; // couleur par défaut

void setup() {
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();

  Serial.begin(9600);
  while (!Serial);
  Serial.println("LEDs prêtes (respiration + cœur + humeur).");
}

void loop() {
  unsigned long time = millis();

  // === 1. Lire la valeur d’humeur du PC ===
  //if (Serial.available()) {
    moodValue = 100;
    Serial.print("Humeur reçue : "); Serial.println(moodValue);
    updateMoodColor(moodValue);
  //}

  // === 2. Effet respiration (sinus) ===
  float respPos = (float)(time % respirationCycle) / respirationCycle;
  float baseBrightness = (sin(respPos * TWO_PI - PI / 2) + 1.0) / 2.0;

  // === 3. Effet pulsation (cœur) ===
  if (time - lastHeartbeat > heartbeatInterval) {
    lastHeartbeat = time;
    heartbeatActive = true;
  }

  float totalBrightness = baseBrightness;
  if (heartbeatActive) {
    totalBrightness += 0.25;
   if (totalBrightness > 1.0) totalBrightness = 1.0;
  }

  // === 4. Appliquer couleur d’humeur * luminosité actuelle ===
  uint8_t r = (uint8_t)(moodR * totalBrightness);
  uint8_t g = (uint8_t)(moodG * totalBrightness);
  uint8_t b = (uint8_t)(moodB * totalBrightness);

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, r, g, b);
  }
  strip.show();

  // === 5. Fin de pulsation cœur (flash bref 100 ms) ===
  if (heartbeatActive && (time - lastHeartbeat > 100)) {
    heartbeatActive = false;
  }

  delay(50); // Animation fluide
}

// === Définir couleur selon humeur reçue (0 à 599) ===
void updateMoodColor(int value) {
  if (value < 150) {
    moodR = 0; moodG = 0; moodB = 255;    // Calme → Bleu
  } else if (value < 300) {
    moodR = 0; moodG = 255; moodB = 0;    // Moyenne excitation → Vert
  } else if (value < 450) {
    moodR = 255; moodG = 165; moodB = 0;  // Excité → Orange
  } else {
    moodR = 255; moodG = 0; moodB = 0;    // Très excité → Rouge
  }
}