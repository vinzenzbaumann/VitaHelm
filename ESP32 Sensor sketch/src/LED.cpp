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

  // Flankenerkennung: steigende Flanke (LOW → HIGH)
  if (lastMicState == LOW && currentMicState == HIGH) {
    
    if (breathStateEinatmen) {
      Serial.println("Einatmen erkannt");
      LED_Num = 2;
      breathStateEinatmen = false;  // Als nächstes kommt Ausatmen
    } else {
      Serial.println("Ausatmen erkannt");
      LED_Num = 15;
    
  
      breathStateEinatmen = true;   // Als nächstes kommt wieder Einatmen
    }
  }

  lastMicState = currentMicState;



/*
//breath in
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
    LED_Num = 5;
    Serial.print("Einatmen");

  }
// breath out
if(breathRisingEdge && einatmen){
  if(breathFallingEdge)
  {
    einatmen = false;
    breathRisingEdge = false;
    breathFallingEdge = false;
    breathEndTime = time;
    ausatmen = true;
    LED_Num = 10;
    Serial.print("Ausatmen");

  }
  
  }
}

*/
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


  // farbe setzen
     for (int i = LED_Num; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, r, g, b);
  }

    for (int i = 0; i < LED_Num; i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  
  strip.show();

  // Front-LEDs konstant weiß
  for (int i = 0; i < FRONT_LEDS; i++) {
    front.setPixelColor(i, 255, 255, 255);
  }
  front.show();
}
