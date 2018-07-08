#ifndef EFFECT_H
#define EFFECT_H

#include "FastLED.h"

#define FRAMES_PER_SECOND  50
#define NUM_CANDLES (100) // number of different candles used in simulation NUM_CANDLES <= 255
#define MAX_LED_NUM    200 // number of leds in the strip
#define NUM_SPARK   1 // number of sparks per frame
#define LUMU_MAX_PHASE_BITS (9) // >= 8
#define LUMU_MAX_PHASE_SHIFT (16-LUMU_MAX_PHASE_BITS)
#define LUMU_MAX_PHASE (((unsigned long)1<<LUMU_MAX_PHASE_BITS)-1) // LUMU maximum phase 
#define NUM_LINES (10) // number of lines displayed at the same time in the line effect
#define MAX_LINE_LENGTH (50)


#define BLUR_SIDE_LENGTH 2

#define BLUR_WIDTH (BLUR_SIDE_LENGTH * 2 + 1)

extern CRGB leds[MAX_LED_NUM];
extern int numLeds;

typedef struct Effect {
  // inits the effect
  void(* init)();
  // updates the effect
  // if stop = true, the effect is entering the stop mode (gradual darkening)
  // returns true, if the effect has stopped
  boolean (*update)(boolean stop);
};


struct BLUR_EFFECT_DATA {
  CRGB buffer[BLUR_SIDE_LENGTH];
  boolean stopFlag;
};

struct LINE_DATA {
  CRGB color; // color of line
  long position; // 16.16 position of line
  long length; // 16.16 length of line
  long speed; // 16.16 speed
};

struct LINE_EFFECT_DATA {
  boolean stopFlag;
  // data for each candle
  struct LINE_DATA lines[NUM_LINES];
};

struct CANDLE_DATA {
  // start level
  byte attackLevel;
  // 8.8 increase of attack phase
  unsigned int attackInc;
  // high level
  byte sustainLevel;
  // 8.8 decrease of release phase
  unsigned int releaseDec;
  // level of delay phase
  byte delayLevel;
  // phase counter of end of attack
  unsigned int attackEndPhase;
  // phase counter of end of sustain phase
  unsigned int sustainEndPhase;
  // phase counter of end of release phase
  unsigned int releaseEndPhase;
  // phase counter of end of delay phase (phase counter restarts to 0)
  unsigned int delayEndPhase;
  // current phase
  unsigned int phase;
  // counter inside the phase, reseted in every phase change
  byte phaseCounter;
  // level 8.8
  unsigned int level;
};

struct CANDLE_EFFECT_DATA {
  // data for each candle
  struct CANDLE_DATA candle[NUM_CANDLES];
  // map of colors (r,g,b) for each heat value
  byte candleMap[MAX_LED_NUM];
};

struct TRAIL_EFFECT_DATA {
  unsigned int phase;
};

struct FIRE_EFFECT_DATA {
  // Array of temperature readings at each simulation cell
  byte heat[MAX_LED_NUM + 3];
  unsigned int phase;

};


struct SET_EFFECT_DATA {
  unsigned int phase;
};

struct BLINK_EFFECT_DATA {
  unsigned int phase;
  boolean stopFlag;
  byte red;
  byte green;
  byte blue;

  byte nextRed;
  byte nextGreen;
  byte nextBlue;
};

struct LUMU_EFFECT_DATA {
  unsigned long phase;
  boolean stopFlag;
  byte red;
  byte green;
  byte blue;

  byte nextRed;
  byte nextGreen;
  byte nextBlue;
  unsigned long paletteIndex;

};

union EffectData {
  struct BLUR_EFFECT_DATA blurEffectData;
  struct FIRE_EFFECT_DATA fireEffectData;
  struct BLINK_EFFECT_DATA blinkEffectData;
  struct LUMU_EFFECT_DATA lumuEffectData;
  struct TRAIL_EFFECT_DATA trailEffectData;
  struct CANDLE_EFFECT_DATA candleEffectData;
  struct LINE_EFFECT_DATA lineEffectData;
  struct SET_EFFECT_DATA setEffectData;
  EffectData() {
    memset( this, 0, sizeof( EffectData ) );
  }
};

static EffectData u;

boolean updateBlur(boolean stop);
void initBlur();

boolean updateSet(boolean stop);
void initSet();

boolean updateWhite(boolean stop);
void initWhite();

boolean updateLine(boolean stop);
void initLine();

// Fire
boolean updateFire(boolean stop);
void initFire();

// Blink
boolean updateBlink(boolean stop);
void initBlink();


// Blink
boolean updateLUMU(boolean stop);
void initLUMU();

// Spark
boolean updateSpark(boolean stop);
void initSpark();

// Trail1
boolean updateTrail1(boolean stop);
void initTrail1();

// Candle
boolean updateCandle(boolean stop);
void initCandle();
void randomizeCandle(struct CANDLE_DATA* c);

// Utils
unsigned long sumLeds();
void print(char* text, long value);
void darker(byte value);
void shiftDown();

#define NUM_EFFECTS 8
extern Effect effects[NUM_EFFECTS];

void initEffects();

#endif


