#include "LED.h"
#include "oxymeter.h"  // enthält heartbeatDetected
#include "Microphone.h"
#include "MAX30105.h"


Adafruit_NeoPixel strip(NUM_LEDS, PIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel front(FRONT_LEDS, FRONT_LED_PIN, NEO_RBG + NEO_KHZ800);

//Brightnesswerte
int LED_Num = 0;
float globalBrightness = 0.0;
float baseBrightness = 0.2;
float heartBrightness = 0.0;

long heartValue = 0.0;

unsigned long heartbeatTime   =   0.0;
unsigned long currentTime     =   0.0;
unsigned long breathStartTime =   0.0;
unsigned long breathEndTime   =   0.0;
unsigned long breathTime      =   0.0;
const unsigned long fadeDuration = 500; // to do abhängig von BPM


//led lauflicht
int currentLedIndex = 0;
unsigned long lastAnimationUpdate = 0;
const unsigned long animationStepDuration = 100; //
bool animationRewind = false;  // 
bool newBreathStatePending = false; // 
bool targetBreathState = true;  // 
bool animationInProgress = false;

//entprellen
unsigned long lastInputChangeTime = 0;
const unsigned long debounceDelay = 500;



//local bools
bool einatmen = false;
bool ausatmen = true;
bool breathStateEinatmen = true;  // Start mit Einatmen
bool lastMicState = LOW;



int moodValue = 0;

uint8_t moodR = 0, moodG = 0, moodB = 255;


void updateMoodColor(int value) {
  int index = constrain(value, 0, 599) / 100;
  switch (index) {
    case 0: moodR =   0; moodG =   0; moodB = 255; break;                 //blau
    case 1: moodR = 255; moodG =   0; moodB = 255; break;                //magenta
    case 2: moodR =   0; moodG = 255; moodB = 255; break;               //cyan
    case 3: moodR =   0; moodG = 255; moodB =   0; break;              //grün
    case 4: moodR = 255; moodG = 255; moodB =   0; break;             //gelb
    case 5: default: moodR = 255; moodG =   0; moodB =   0; break;   //rot
  }
}

void ledLoop(int moodValue){





  unsigned long time = millis();
  updateMoodColor(moodValue);

  //Herzschlöag blitzen alssne

//Serial.println(heartbeatDetected);

 if (heartbeatDetected) {
    heartbeatDetected = false;
    heartBrightness = 1.0;
    heartbeatTime = time;
  }

  // Sanftes Abfallen
  unsigned long timeSinceBeat = time - heartbeatTime;
  if (timeSinceBeat < fadeDuration) {
    float fadeFactor = 1.0 - (float)timeSinceBeat / fadeDuration;
    heartBrightness = fadeFactor;
  } else {
    heartBrightness = 0.0;
  }

globalBrightness = heartBrightness + baseBrightness;

  uint8_t r = (uint8_t)(moodR * globalBrightness);
  uint8_t g = (uint8_t)(moodG * globalBrightness);
  uint8_t b = (uint8_t)(moodB * globalBrightness);

// atemlogik
int currentMicState = digitalRead(MICROPHONE_DIGITAL_PIN);

if (lastMicState == LOW && currentMicState == HIGH) {
    unsigned long now = millis();
    if (now - lastInputChangeTime > debounceDelay) {
        lastInputChangeTime = now;

        if (!animationInProgress) {
            // Animation starten
            targetBreathState = !breathStateEinatmen;
            animationRewind = true;
            animationInProgress = true;
            lastAnimationUpdate = now;
            newBreathStatePending = true;
        } else {
            // Animation läuft schon, Richtung wechseln und Ziel anpassen
            animationRewind = !animationRewind;
            targetBreathState = !targetBreathState;
            lastAnimationUpdate = now; // Optional: Timer resetten
        }
    }
}
lastMicState = currentMicState;


//Serial.println(breathRisingEdge);

//breathcycle
/*
if(breathRisingEdge && !einatmen){
  if(ausatmen)
  {
    breathStartTime = time;
    ausatmen = false;
  }
  if(breathFallingEdge)
  {
    breathRisingEdge = false;
    breathFallingEdge = false;
    einatmen = true;
  }

if(breathRisingEdge && einatmen){
  if(breathFallingEdge)
  {
    einatmen = false;
    breathRisingEdge = false;
    breathFallingEdge = false;
    breathEndTime = time ;
    ausatmen = true;

  }
}
 breathTime = breathEndTime-breathStartTime;
}
*/


  unsigned long now = millis();

if (now - lastAnimationUpdate > animationStepDuration) {
    lastAnimationUpdate = now;

    if (animationRewind) {
        if (currentLedIndex > 0) {
            currentLedIndex--;
        } else {
            // Am Ende der Rückwärtsanimation
            animationRewind = false;
            breathStateEinatmen = targetBreathState;  // neuen Zustand übernehmen
            newBreathStatePending = false;
        }
    } else {
        if (currentLedIndex < NUM_LEDS) {
            currentLedIndex++;
        } else {
            // Animation komplett fertig
            animationInProgress = false;
        }
    }
}



// LEDs setzen
for (int i = 0; i < NUM_LEDS; i++) {
    if (breathStateEinatmen) {
        // Einatmen: LEDs ab hinten an, ab Index NUM_LEDS - currentLedIndex
        if (i >= NUM_LEDS - currentLedIndex) {
            strip.setPixelColor(i, r, g, b);
        } else {
            strip.setPixelColor(i, 0, 0, 0);
        }
    } else {
        // Ausatmen: LEDs ab vorne aus, also LEDs mit Index < currentLedIndex aus
        if (i < currentLedIndex) {
            strip.setPixelColor(i, 0, 0, 0);
        } else {
            strip.setPixelColor(i, r, g, b);
        }
    }
}


  strip.show();

  // Front-LEDs konstant weiß
  for (int i = 0; i < FRONT_LEDS; i++) {
    front.setPixelColor(i, 255, 255, 255);
  }
  front.show();
}
