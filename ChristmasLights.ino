//#include "Arduino.h"
#include "Effect.h"
#include <FastLED.h>
#include <IRremote.h>
#include "RemoteCodes.h"
//#include <Streaming.h>

#define SW1_PIN 8
#define SW2_PIN 9
#define SW4_PIN 10
#define SW8_PIN 11


// LED String Data PIN
#define LED_PIN     5
#define COLOR_ORDER RGB
#define CHIPSET     WS2812

#define BRIGHTNESS        0x80
#define FRAMES_PER_SECOND  50

// RED LED
#define RED_PIN    7

// PIR Sensor's pin
#define PIR_PIN    6
#define PIR_AHEAD_TIME 5000;

// UNCOMMENT THIS IF PIR SENSOR IS USED
//#define USE_PIR

// Auto off delay time after the last PIR/IR event
#define AUTO_POWER_OFF_MS (4* 60L * 60L * 1000L)

// IR Receiver's Serial port
#define IR_SERIAL (Serial2)

// Effect States

#define STATE_STOPPING     101
#define STATE_INIT         103
#define STATE_RUN          104

// Application state
#define APP_START          200
#define APP_RUN            201
#define APP_STOP           202
#define APP_STOPPING       203

#define DEBUG_ENABLED


#ifdef DEBUG_ENABLED
#define DEBUGT(text) Serial.print(F(text));
#define DEBUGTLN(text) Serial.println(F(text));
#define DEBUGV(text, value) Serial.print(F(text)); Serial.print(": "); Serial.println(value);
#define DEBUGVO(value) Serial.print(value);
#else
#define DEBUGT(text)
#define DEBUGTLN(text)
#define DEBUGV(text, value)
#define DEBUGVO(value)
#endif

#define MIN_BRIGHTNESS 5
#define MAX_BRIGHTNESS 255
#define BRIGHTNESS_STEP 5


CRGB leds[MAX_LED_NUM];

int numLeds = 0;

int effectIndex = 0;

int appState = APP_STOP;

int effectState = STATE_INIT;



boolean oldPIR = false;
boolean newPIR = false;
boolean pirMotionOn = false;

unsigned long onLastPushed = 0;

unsigned long nextEffectLastPushed = 0;
unsigned long prevEffectLastPushed = 0;


// time should pass between the pressing of ON/OFF button
unsigned long ON_OFF_PUSH_TIME_TRESHOLD = 1000;

int lastButtonCode = -1;

int nextEffectIndex = -1;
int switchToEffect = -1;

boolean on = false;

int brightness = BRIGHTNESS;

unsigned long lastPIREvent = 0;
unsigned long lastIREvent = 0;

// red status led brightness value is determined using the maximum of the three levels (standby, pir, ir)
long standbyLevel; // 8.8 level of standby
long standbySpeed; // -STANDBY_SPEED, 0, +STANDBY_SPEED
long pirLevel; // 8.8 level of pir
long irLevel; // 8.8 level of ir
long irCounter; // frame counter for ir on/off

// all off levels are 0
#define STANDBY_LEVEL ((long)63 << 8) // level of standby level when on
#define STANDBY_LEVEL_SPEED ((long)5 << 7) // speed of standby level change
#define PIR_LEVEL ((long)1 << 8) // level of PIR sensor when on
#define IR_LEVEL ((long)63 << 8) // level of IR pulse when on
#define IR_FRAMES (long)(1) // number of frames when the IR pulse is displayed


bool autoPowerOffEnabled = false;

void initRedLed();
void autoPowerOff();

void initSwitch() {
  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);
  pinMode(SW4_PIN, INPUT_PULLUP);
  pinMode(SW8_PIN, INPUT_PULLUP);
}

int readSwitch() {
  int value = 0;

  int sw1 = digitalRead(SW1_PIN);
  int sw2 = digitalRead(SW2_PIN);
  int sw4 = digitalRead(SW4_PIN);
  int sw8 = digitalRead(SW8_PIN);
  /*
  Serial.print(sw1);
  Serial.print(sw2);
  Serial.print(sw4);
  Serial.println(sw8);
  */

  value = sw1;
  value += sw2 << 1;
  value += sw4 << 2;
  value += sw8 << 3;
  return 15 - value;
}

void setupNumberOfLeds() {
  initSwitch();
  int switchValue = readSwitch();
  if (switchValue == 0) {
    switchValue = 1;
    DEBUGTLN("Switch value is set to 0, using 1 instead!");
  }

  numLeds = switchValue * 50;
  if (numLeds > MAX_LED_NUM) {
    numLeds = MAX_LED_NUM;
    DEBUGT("Switch is set to a greater value, than maximum number of leds supported!");
  }

  DEBUGV("Number of LED", numLeds);

}

/*
 * Setup
 */
void setup() {
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, MAX_LED_NUM);
  FastLED.setCorrection( TypicalLEDStrip );
  FastLED.setDither(0);
  FastLED.setBrightness( brightness );

#ifdef DEBUG_ENABLED
  Serial.begin(9600);
#endif
  IR_SERIAL.begin(9600);

  pinMode(PIR_PIN, INPUT);

  lastPIREvent = millis();
  lastIREvent = millis();
  autoPowerOffEnabled = false;

  setupNumberOfLeds();
  random16_add_entropy( random());
  initRedLed();
  DEBUGTLN("Setup finished");
  /* Serial.println(sizeof(CRGBPalette256));
   Serial.print("fire: "); Serial.println(sizeof(FIRE_EFFECT_DATA));
   Serial.print("blink: "); Serial.println(sizeof(BLINK_EFFECT_DATA));
   Serial.print("lumu: "); Serial.println(sizeof(LUMU_EFFECT_DATA));
   Serial.print("trail: "); Serial.println(sizeof(TRAIL_EFFECT_DATA));
   Serial.print("single candle: "); Serial.println(sizeof(CANDLE_DATA));
   Serial.print("line: "); Serial.println(sizeof(LINE_EFFECT_DATA));
  */

}

#define MAX_PIR_EVENT_TRESHOLD  15000

void loop()
{

  unsigned long startTime = millis();

  autoPowerOff();
  readIRReceiver();
  readPIRSensor();
  updateRedLed();


  if (appState == APP_START) {
    DEBUGTLN("App state: APP_START");
    effectIndex = 0;
    effectState = STATE_INIT;
    appState = APP_RUN;
  }
  else if (appState == APP_RUN) {
    // DEBUGV("On: ", on);

    if (!on) {
      appState = APP_STOPPING;
      effectState = STATE_STOPPING;
      autoPowerOffEnabled = false;
      standbySpeed = STANDBY_LEVEL_SPEED;
    }

  }
  else if (appState == APP_STOP) {
    // DEBUGT("App state: APP_STOP");
    if (on) {
      appState = APP_START;
      autoPowerOffEnabled = true;
      standbySpeed = -STANDBY_LEVEL_SPEED;
    }
  }

  if (appState != APP_STOP) {
    runEffect();
  }

  unsigned long finishTime = millis();

  unsigned long updateInterval = (finishTime - startTime);

  long delayTime = (1000 / FRAMES_PER_SECOND) - updateInterval;

  if (delayTime > 0) {
    long ellapsedAfterPIR = millis() - lastPIREvent;
    if (ellapsedAfterPIR < MAX_PIR_EVENT_TRESHOLD) {
      long _delayTime = ((delayTime * ellapsedAfterPIR << 8) / MAX_PIR_EVENT_TRESHOLD) >> 8;

      delayTime = _delayTime;
    }
    FastLED.delay(delayTime);
  }

}

void autoPowerOff() {
  if (autoPowerOffEnabled) {
    unsigned long currentTimeMS = millis();
    unsigned long ellapsedPIR = currentTimeMS - lastPIREvent;
    unsigned long ellapsedIR = currentTimeMS - lastIREvent;

    boolean pir = (ellapsedPIR > AUTO_POWER_OFF_MS);
    boolean ir = (ellapsedIR > AUTO_POWER_OFF_MS);

    /*
        Serial.print("ellapsedPIR");
        Serial.println(ellapsedPIR);
        Serial.print("ellapsedIR");
        Serial.println(ellapsedIR);
        Serial.print("pir");
        Serial.println(pir);
        Serial.print("ir");
        Serial.println(ir);
        Serial.print("AUTO_POWER_OFF_MS");
        Serial.println(AUTO_POWER_OFF_MS);
    */
    if (pir && ir) {
      on = false;
      DEBUGTLN("Auto Power Off");
    }
  }
}

int prevEffectState = -1;

void printEffectState(int effectState) {
  if (effectState == 101) {
    DEBUGT("STOPPING");
  }
  else if (effectState == 103) {
    DEBUGT("INIT");
  }
  else if (effectState == 104) {
    DEBUGT("RUN");
  }
  else {
    DEBUGT("Unknown!");
  }
}


void runEffect() {

  Effect currentEffect = effects[effectIndex];

  if (prevEffectState != effectState) {
    printEffectState(prevEffectState);
    DEBUGT(" -> ");
    printEffectState(effectState);

    DEBUGVO(effectIndex);
    prevEffectState = effectState;
  }

  if (effectState == STATE_INIT) {

    currentEffect.init();
    effectState = STATE_RUN;
  }


  if (effectState == STATE_RUN) {
    //DEBUGT("Effect state: STATE_RUN");
  }
  if (effectState == STATE_RUN && nextEffectIndex != -1) {
    effectState = STATE_STOPPING;
  }

  boolean stopFlag = false;
  if (effectState == STATE_STOPPING) {
    stopFlag = true;
    effectState = STATE_STOPPING;
  }

  boolean stopped = currentEffect.update(stopFlag);

  if (stopped && appState == APP_STOPPING) {
    appState = APP_STOP;
    return;
  }

  if (stopped && on && appState == APP_RUN) {
    if (nextEffectIndex != -1) {
      effectIndex = nextEffectIndex;
      nextEffectIndex = -1;
    }
    else {
      effectIndex = 0;
    }
    effectState = STATE_INIT;
  }
}


void readIRReceiver() {
  if (IR_SERIAL.available() > 0) {


    int buttonCode = IR_SERIAL.parseInt();
    // consume end of lines
    while (IR_SERIAL.read() != '\n');

    if (buttonCode != -1) {

      DEBUGV("IR Remote Button code", buttonCode);

      irCounter = IR_FRAMES;
      lastIREvent = millis();
    }

    if (buttonCode == BUTTON_ON_OFF && (millis() - onLastPushed) > ON_OFF_PUSH_TIME_TRESHOLD ) {
      on = !on;
      onLastPushed = millis();
      DEBUGV("IR ON: ", on);
    }

    if (buttonCode == BUTTON_CHANNEL_DOWN && effectState == STATE_RUN && (millis() - nextEffectLastPushed) > ON_OFF_PUSH_TIME_TRESHOLD ) {
      nextEffectIndex = effectIndex + 1;
      if (nextEffectIndex > NUM_EFFECTS - 1) {
        nextEffectIndex = 0;
      }
      nextEffectLastPushed = millis();
      DEBUGV("UP", nextEffectIndex);
    }
    else if (buttonCode == BUTTON_CHANNEL_UP && effectState == STATE_RUN && (millis() - prevEffectLastPushed) > ON_OFF_PUSH_TIME_TRESHOLD) {

      nextEffectIndex = effectIndex - 1;
      if (nextEffectIndex < 0) {
        nextEffectIndex = NUM_EFFECTS - 1;
      }
      prevEffectLastPushed = millis();
      DEBUGV("DOWN", nextEffectIndex);
    }



    if (buttonCode == BUTTON_VOLUME_UP) {
      if (brightness < MAX_BRIGHTNESS) {
        brightness += BRIGHTNESS_STEP;
        if (brightness > MAX_BRIGHTNESS) {
          brightness = MAX_BRIGHTNESS;
        }
        FastLED.setBrightness( brightness );
      }
    }
    else if (buttonCode == BUTTON_VOLUME_DOWN) {
      if (brightness > BRIGHTNESS_STEP) {
        brightness -= BRIGHTNESS_STEP;
        FastLED.setBrightness( brightness );

      }
      else {
        brightness = MIN_BRIGHTNESS;
        FastLED.setBrightness( brightness );
      }
    }

    if (buttonCode != lastButtonCode) {
      lastButtonCode = buttonCode;
    }
  }
}



unsigned long start ;

void readPIRSensor() {
#ifdef USE_PIR
  newPIR = digitalRead(PIR_PIN);
  if (newPIR != oldPIR) {


    if (newPIR) {
      start = millis();
      pirMotionOn = true;
      lastPIREvent = millis();
      DEBUGT("PIR ON");
    }
    else {
      pirMotionOn = false;
      DEBUGT("PIR OFF");
      //lastPIREvent = millis();
    }

    oldPIR = newPIR;
  }
#endif
}

/*
* Updates the Red led
*/

void initRedLed() {
  standbyLevel = STANDBY_LEVEL;
  pirLevel = 0;
  irLevel = 0;
  irCounter = 0;
  pinMode(RED_PIN, OUTPUT);
}

void setRedLed(byte value) {
  analogWrite(RED_PIN, value);
}

void updateRedLed() {

  standbyLevel += standbySpeed;
  //Serial.println(standbySpeed);
  if (standbyLevel > STANDBY_LEVEL) {
    standbyLevel = STANDBY_LEVEL;
    standbySpeed = 0;
  }
  if (standbyLevel < 0) {
    standbyLevel = 0;
    standbySpeed = 0;
  }
  if (pirMotionOn) {
    pirLevel = PIR_LEVEL;
  }
  else {
    pirLevel = 0;
  }

  if (irCounter > 0) {
    irLevel = IR_LEVEL;
    irCounter--;
  }
  else {
    irLevel = 0;
  }

  long level = standbyLevel;
  if (level < pirLevel) {
    level = pirLevel;
  }
  if (level < irLevel) {
    level = irLevel;
  }

  setRedLed((level >> 8) & 0xff);

}
