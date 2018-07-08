#include "Effect.h"


Effect effects[] = {
  //{&initBlur, &updateBlur},
  {&initCandle, &updateCandle},
  {&initFire, &updateFire},
  {&initTrail1, &updateTrail1},
  {&initSpark, &updateSpark},
  {&initBlink, &updateBlink},
  {&initLUMU, &updateLUMU},
  {&initWhite, &updateWhite},
  {&initSet, &updateSet},

  //  {&initLine, &updateLine}
};

const CRGB LUMU_COLORS[] = {
  0xFF0000,
  0xFF0000,

  0x800080,
  0x800080,

  0x0000FF,
  0x0000FF,

  0x008080,
  0x008080,

  0x00FF00,
  0x00FF00,

  0x808000,
  0x808000

  /*
  CRGB::DeepPink,
  CRGB::MediumVioletRed,
  CRGB::Crimson,
  CRGB::Red,
  CRGB::OrangeRed,
  CRGB::Brown,
  CRGB::Maroon,
  CRGB::DarkOliveGreen,
  CRGB::Olive,
  CRGB::OliveDrab,
  CRGB::SeaGreen,
  CRGB::ForestGreen,
  CRGB::Green,
  CRGB::DarkGreen,
  CRGB::Blue,
  CRGB::Navy,
  CRGB::DarkViolet,
  CRGB::DarkMagenta,
  CRGB::Indigo,
  CRGB::DarkSlateBlue,
  CRGB::Black*/
};

#define LUMU_COLORS_LENGTH (sizeof(LUMU_COLORS) / sizeof(CRGB))

CRGBPalette256  heatMap;
void printCRGB(CRGB color) {
  Serial.print("r: ");
  Serial.print(color.r);
  Serial.print(", g:");
  Serial.print(color.g);
  Serial.print(", b:");
  Serial.println(color.b);

}

void printBuffer() {
  for (int i = 0; i < BLUR_SIDE_LENGTH; i++) {
    Serial.print("i=");
    Serial.print(i);
    Serial.print(", color: ");
    printCRGB(u.blurEffectData.buffer[i]);
  }
}


CRGB updateBuffer(CRGB newValue) {
  //Serial.print("-------------------------newValue=");
  //printCRGB(newValue);
  CRGB result = u.blurEffectData.buffer[BLUR_SIDE_LENGTH - 1];
  for (int i = BLUR_SIDE_LENGTH - 1; i > 0; i--) {
    u.blurEffectData.buffer[i] = u.blurEffectData.buffer[i - 1];
  }
  u.blurEffectData.buffer[0] = newValue;
  //printBuffer();
  return result;
}




unsigned long gauss[] = {16, 62, 99, 62, 16};

void blur() {
  CRGB pixel = CRGB::Black;
  unsigned long r;
  unsigned long g;
  unsigned long b;
  unsigned long _r;
  unsigned long _g;
  unsigned long _b;
  byte index;

  for (int i = 0; i < numLeds + BLUR_SIDE_LENGTH; i++) {
    r = 0;
    g = 0;
    b = 0;

    index = 0;
    for (int j = i - BLUR_SIDE_LENGTH; j <= i + BLUR_SIDE_LENGTH; j++) {


      if (j > 0 && j < numLeds) {
        _r = leds[j].r * gauss[index];
        _g = leds[j].g * gauss[index];
        _b = leds[j].b * gauss[index];

        r += _r;
        g += _g;
        b += _b;
      }

      index++;
    }
    r = r / BLUR_WIDTH;
    g = g / BLUR_WIDTH;
    b = b / BLUR_WIDTH;

    r += 127;
    g += 127;
    b += 127;

    r = r >> 8;
    if (r > 0xff) {
      r = 0xff;
    };
    g = g >> 8;
    if (g > 0xff) {
      g = 0xff;
    };
    b = b >> 8;
    if (b > 0xff) {
      b = 0xff;
    };

    /*
        Serial.print("r: ");
        Serial.print(r);
        Serial.print(", g:");
        Serial.print(g);
        Serial.print(", b:");
        Serial.println(b);
    */
    pixel.r = (byte)r;
    pixel.g = (byte)g;
    pixel.b = (byte)b;

    CRGB value = updateBuffer(pixel);
    int writeBackIndex = i - BLUR_SIDE_LENGTH;
    if (writeBackIndex >= 0) {
      /*   Serial.print("i=");Serial.print(i);
         Serial.print(",wbi=");Serial.println(writeBackIndex);
         printCRGB(pixel);*/
      leds[writeBackIndex] = value;
    }
  }
}




boolean updateBlur(boolean stop) {
  CHSV hsv = CHSV(random(256), 255, 255);
  CRGB rgb;
  hsv2rgb_spectrum(hsv, rgb);


  int index = random(numLeds);
  int length = random(numLeds - index);
  for (int i = 0; i < length; i++) {
    leds[index] += rgb;
  }

  //  blur();

  // if (random(10) == 0) {
  // darker(1);
  //}
  darker(1);
  FastLED.show(); // display this frame

  return false;
}


void initBlur() {
  for (int i = 0; i < numLeds; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
}

//-----------------------
// WHITE test effect
//------------------------

boolean updateWhite(boolean stop) {
  if (!stop) {
    if (u.setEffectData.phase < 255) {
      u.setEffectData.phase += 5;
    }

    for (int i = 0; i < numLeds; i++) {
      leds[i] = CRGB(u.setEffectData.phase, u.setEffectData.phase, u.setEffectData.phase);

    }

    FastLED.show(); // display this frame
    return false;
  }
  else {

    darker(5);
    FastLED.show(); // display this frame

    if (sumLeds() == 0) {
      return true;
    }
    return false;
  }
}

void initWhite() {
  u.setEffectData.phase = 0;
}

//-----------------------
// SET test effect
//------------------------

boolean updateSet(boolean stop) {
  if (!stop) {
    if (u.setEffectData.phase < 255) {
      u.setEffectData.phase += 5;
    }
    for (int i = 0; i < numLeds; i++) {
      leds[i] = CRGB(u.setEffectData.phase / 25, u.setEffectData.phase / 25, u.setEffectData.phase / 25);
      if ((i + 1) % 50 == 0) {
        leds[i] = CRGB(u.setEffectData.phase, u.setEffectData.phase, u.setEffectData.phase);
      }
    }
    FastLED.show(); // display this frame
    return false;
  }
  else {

    darker(5);
    FastLED.show(); // display this frame

    if (sumLeds() == 0) {
      return true;
    }
    return false;
  }
}

void initSet() {
  u.setEffectData.phase = 0;
}

//--------------------------------------------------------------------------------------------------
// LINE
//--------------------------------------------------------------------------------------------------
void initOneLine(int lineIndex);

void renderLine(int index) {
  // Serial.print("renderLine:");
  // Serial.println(index);

  LINE_DATA line = u.lineEffectData.lines[index];
  int startIndex = line.position >> 16;

  // Serial.print("startIndex:");
  // Serial.println(startIndex);

  int length = line.length >> 16;

  // Serial.print("length:");
  // Serial.println(length);

  long speed = line.speed;

  // Serial.print("speed:");
  // Serial.println(speed);

  int endIndex;

  if (speed > 0) {
    endIndex = startIndex - length;

  }
  else {
    endIndex = startIndex + length;


  }

  // Serial.print("endIndex:");
  // Serial.println(endIndex);

  if (startIndex > endIndex) {
    long tmp = startIndex;
    startIndex = endIndex;
    endIndex = tmp;
  }

  bool bothLessThanZero =  (endIndex < 0 && startIndex < 0);
  bool bothGreaterThanNumLedsMinusOne = (endIndex > numLeds - 1 && startIndex > numLeds - 1);

  bool draw = !bothLessThanZero && !bothGreaterThanNumLedsMinusOne;
  //Serial.print("draw = ");
  //Serial.println(draw);
  if (draw) {
    if (startIndex < 0) {
      startIndex = 0;
    }
    if (endIndex > numLeds - 1) {
      endIndex = numLeds - 1;
    }
    /*
      Serial.print("_startIndex:");
      Serial.println(startIndex);
       Serial.print("_endIndex:");
      Serial.println(endIndex);
      */
    for (int p = startIndex; p <= endIndex; p++) {
      // Serial.print("p = ");
      // Serial.print(p);
      leds[p] += line.color;
    }
  }

  /*
    Serial.println("-----After change-----------");
    Serial.print("startIndex:");
    Serial.println(startIndex);
    Serial.print("endIndex:");
    Serial.println(endIndex);
    Serial.println("----------------");

    if (startIndex < 0) {
      startIndex = 0;
    }
    if (endIndex > numLeds - 1) {
      endIndex = numLeds - 1;
    }

    Serial.println("-----After clip-----------");
    Serial.print("startIndex:");
    Serial.println(startIndex);
    Serial.print("endIndex:");
    Serial.println(endIndex);
    Serial.println("----------------");

  */



}

void renderLines() {
  for (int i = 0; i < numLeds; i++) {
    leds[i] = CRGB::Black;
  }
  for (int i = 0; i < NUM_LINES; i++) {
    renderLine(i);
  }
}

void moveLines() {
  for (int i = 0; i < NUM_LINES; i++) {
    u.lineEffectData.lines[i].position += u.lineEffectData.lines[i].speed;

    //Serial.print("position: ");
    //Serial.print(u.lineEffectData.lines[i].position);
    // Serial.print(",e: ");
    // Serial.println(endIndex);

    int tailPos;
    if (u.lineEffectData.lines[i].speed > 0) {
      tailPos = (u.lineEffectData.lines[i].position - u.lineEffectData.lines[i].length) >> 16;
      if (!u.lineEffectData.stopFlag && tailPos > numLeds - 1) {
        initOneLine(i);
      }
    }
    else  {
      tailPos = (u.lineEffectData.lines[i].position + u.lineEffectData.lines[i].length) >> 16;
      if (!u.lineEffectData.stopFlag && tailPos < 0 ) {
        initOneLine(i);
      }
    }
  }
}


boolean updateLine(boolean stop) {

  //Serial.println("updateLine");
  //Serial.println(millis());

  renderLines();

  FastLED.show(); // display this frame

  // Serial.println("updateLine");
  moveLines();

  if (!stop) {
  }
  else {
    u.lineEffectData.stopFlag = true;
    unsigned long sum = sumLeds();

    if (sum <= numLeds * 3) {
      return true;
    }
  }


  /*
  /////////////////////////////////////////////
  // STOP MODE
  /////////////////////////////////////////////
  else {

  Serial.println("stop mode");

    ///////////////////////////////////////
    // First time stopped
    ///////////////////////////////////////
    if (!u.lineEffectData.stopFlag) {
      //u.lineEffectData.stopFlag = 1;

    }
    else {


      unsigned long sum = sumLeds();

      if (sum <= 600) {
        return false;
      }
    }*/
  return false;
  // }
}

void initOneLine(int lineIndex) {
  //  Serial.print("line index: ");
  //  Serial.println(lineIndex);

  int i = lineIndex;
  CHSV hsv = CHSV(random(256), 255, 255);
  CRGB rgb;
  hsv2rgb_rainbow(hsv, rgb);
  rgb = rgb /= (NUM_LINES / 3);
  u.lineEffectData.lines[i].color = rgb;

  int up = random(0, 2);

  //Serial.print("up: ");
  //Serial.println(up);



  long length = random(MAX_LINE_LENGTH);
  length = length << 16;
  length += random16();

  long position;
  long speed = MAX_LINE_LENGTH - length;
  speed = speed << 16;
  speed /= MAX_LINE_LENGTH;
  speed /= FRAMES_PER_SECOND;
  speed += random16() / 2;

  if (up) {
    position = -1L;
    position = position << 16;
    position += random16();
  }
  else {
    position = numLeds;
    position = position << 16;
    position += random16();
    speed = -speed;
  }
  /*
    Serial.print("position: ");
    Serial.println(position);
    Serial.print("speed: ");
    Serial.println(speed);
    Serial.print("length: ");
    Serial.println(length);
  */
  u.lineEffectData.lines[i].position = position;

  u.lineEffectData.lines[i].speed = speed;
  u.lineEffectData.lines[i].length = length;
}

void initLine() {
  u.lineEffectData.stopFlag = false;
  for (int i = 0; i < NUM_LINES; i++) {

    // Serial.println("Init Line!");

    initOneLine(i);
  }
}

//--------------------------------------------------------------------------------------------------
// BLINK
//--------------------------------------------------------------------------------------------------
boolean updateBlink(boolean stop) {
  CRGB color;

  color.r = lerp16by16(u.blinkEffectData.red, u.blinkEffectData.nextRed, u.blinkEffectData.phase << 8);
  color.g = lerp16by16(u.blinkEffectData.green, u.blinkEffectData.nextGreen, u.blinkEffectData.phase << 8);
  color.b = lerp16by16(u.blinkEffectData.blue, u.blinkEffectData.nextBlue, u.blinkEffectData.phase << 8);

  for (int i = 0; i < numLeds; i++) {
    leds[i] = color;
  }

  FastLED.show(); // display this frame

  if (!stop) {

    u.blinkEffectData.phase++;

    if (u.blinkEffectData.phase >= 255) {
      u.blinkEffectData.phase = 0;

      u.blinkEffectData.red = color.r;
      u.blinkEffectData.green = color.g;
      u.blinkEffectData.blue = color.b;

      CHSV hsv = CHSV(random(256), 255, 255);
      CRGB rgb;
      hsv2rgb_rainbow(hsv, rgb);

      u.blinkEffectData.nextRed = rgb.r;
      u.blinkEffectData.nextGreen = rgb.g;
      u.blinkEffectData.nextBlue = rgb.b;

    }
    return false;
  }
  /////////////////////////////////////////////
  // STOP MODE
  /////////////////////////////////////////////
  else {
    ///////////////////////////////////////
    // First time stopped
    ///////////////////////////////////////
    if (!u.blinkEffectData.stopFlag) {
      u.blinkEffectData.stopFlag = true;

      u.blinkEffectData.nextRed = 0;
      u.blinkEffectData.nextGreen = 0;
      u.blinkEffectData.nextBlue = 0;

      u.blinkEffectData.red = color.r;
      u.blinkEffectData.green = color.g;
      u.blinkEffectData.blue = color.b;

      u.blinkEffectData.phase = 0;

    }
    else {
      u.blinkEffectData.phase += 20;
      if (u.blinkEffectData.phase >= 255) {
        u.blinkEffectData.phase = 255;
      }

      unsigned long sum = sumLeds();

      if (sum <= 600) {
        return true;
      }
    }
    return false;
  }

}


void initBlink() {
  u.blinkEffectData.phase = 0;

  u.blinkEffectData.red = 0x00;
  u.blinkEffectData.green = 0x00;
  u.blinkEffectData.blue = 0x00;

  CHSV hsv = CHSV(random(256), 255, 255);
  CRGB rgb;
  hsv2rgb_rainbow(hsv, rgb);

  u.blinkEffectData.nextRed = rgb.r;
  u.blinkEffectData.nextGreen = rgb.g;
  u.blinkEffectData.nextBlue = rgb.b;

  u.blinkEffectData.stopFlag = false;
}

//--------------------------------------------------------------------------------------------------
// LUMU/MÜPA color effect
//--------------------------------------------------------------------------------------------------



boolean updateLUMU(boolean stop) {
  CRGB color;

  unsigned int frac = u.lumuEffectData.phase << LUMU_MAX_PHASE_SHIFT;
  //Serial.println(u.lumuEffectData.phase);

  color.r = lerp16by16(u.lumuEffectData.red, u.lumuEffectData.nextRed, frac);
  color.g = lerp16by16(u.lumuEffectData.green, u.lumuEffectData.nextGreen, frac);
  color.b = lerp16by16(u.lumuEffectData.blue, u.lumuEffectData.nextBlue, frac);
  /* Serial.print(color.r);
   Serial.print("/");
   Serial.print(color.g);
   Serial.print("/");
   Serial.println(color.b);
  */
  for (int i = 0; i < numLeds; i++) {
    leds[i] = color;
  }

  if (!stop) {
    for (int j = 0; j < NUM_SPARK; j++) {
      int rnd = random16(numLeds);
      leds[rnd] = CRGB::White;
    }
  }
  else {
    CRGB white = CRGB::White;
    white = white.fadeToBlackBy(u.lumuEffectData.phase >> (LUMU_MAX_PHASE_BITS - 8));
    for (int j = 0; j < NUM_SPARK; j++) {
      int rnd = random16(numLeds);
      leds[rnd] += white;
    }

  }

  FastLED.show(); // display this frame

  if (!stop) {


    u.lumuEffectData.phase += 1;

    if (u.lumuEffectData.phase >= LUMU_MAX_PHASE) {
      u.lumuEffectData.phase = 0;

      u.lumuEffectData.red = color.r;
      u.lumuEffectData.green = color.g;
      u.lumuEffectData.blue = color.b;

      //CHSV hsv = CHSV(random(256), 255, 127);
      //CRGB rgb;
      //hsv2rgb_rainbow(hsv, rgb);
      u.lumuEffectData.paletteIndex++;
      if (u.lumuEffectData.paletteIndex >= LUMU_COLORS_LENGTH) {
        u.lumuEffectData.paletteIndex = 0;
      }


      u.lumuEffectData.nextRed = LUMU_COLORS[u.lumuEffectData.paletteIndex].r;
      u.lumuEffectData.nextGreen = LUMU_COLORS[u.lumuEffectData.paletteIndex].g >> 2;
      u.lumuEffectData.nextBlue = LUMU_COLORS[u.lumuEffectData.paletteIndex].b >> 2;

    }
    return false;
  }
  /////////////////////////////////////////////
  // STOP MODE
  /////////////////////////////////////////////
  else {
    ///////////////////////////////////////
    // First time stopped
    ///////////////////////////////////////
    if (!u.lumuEffectData.stopFlag) {
      //Serial.println("First time stopped");
      u.lumuEffectData.stopFlag = true;

      u.lumuEffectData.nextRed = 0;
      u.lumuEffectData.nextGreen = 0;
      u.lumuEffectData.nextBlue = 0;

      u.lumuEffectData.red = color.r;
      u.lumuEffectData.green = color.g;
      u.lumuEffectData.blue = color.b;

      u.lumuEffectData.phase = 0;

    }
    else {
      u.lumuEffectData.phase += (unsigned long)20;

      if (u.lumuEffectData.phase >= LUMU_MAX_PHASE) {
        u.lumuEffectData.phase = LUMU_MAX_PHASE;
      }
      //Serial.println(u.lumuEffectData.phase);
      unsigned long sum = sumLeds();

      if (sum <=  (numLeds * 3)) {
        return true;
      }
    }
    return false;
  }

}


void initLUMU() {

  u.lumuEffectData.paletteIndex = 0;
  u.lumuEffectData.phase = 0;

  u.lumuEffectData.red = 0x00;
  u.lumuEffectData.green = 0x00;
  u.lumuEffectData.blue = 0x00;

  u.lumuEffectData.nextRed = LUMU_COLORS[u.lumuEffectData.paletteIndex].r;
  u.lumuEffectData.nextGreen = LUMU_COLORS[u.lumuEffectData.paletteIndex].g;
  u.lumuEffectData.nextBlue = LUMU_COLORS[u.lumuEffectData.paletteIndex].b;


  u.lumuEffectData.stopFlag = false;
}

//--------------------------------------------------------------------------------------------------
// SPARK
//--------------------------------------------------------------------------------------------------

#define NUM_SPARK 2
#define DARKEN_VALUE 1
#define SPARK_CHANCE 50

boolean updateSpark(boolean stop) {

  if (!stop) {
    for (int i = 0; i < NUM_SPARK; i++) {
      if (random(0xff) < SPARK_CHANCE) {
        CHSV spark = CHSV(random(256), 255, 255);
        leds[random(numLeds)] = spark;
      }
    }
    darker(DARKEN_VALUE);

    FastLED.show(); // display this frame
    return false;
  }
  else {

    darker(5);
    FastLED.show(); // display this frame

    if (sumLeds() == 0) {
      return true;
    }
    return false;
  }
}

void initSpark() {

}


//--------------------------------------------------------------------------
// Fire Effect
//--------------------------------------------------------------------------

// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
////
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation,
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on numLeds; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking.
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
//#define COOLING  55
#define COOLING  18

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
//#define SPARKING 120
#define SPARKING 20

// inits fire effect
void initFire() {
  heatMap = CRGBPalette256( CRGB::Black, CRGB::Red, CRGB::Yellow);
  u.fireEffectData.phase = 0;
  for (int i = 0; i < numLeds + 3; i++) {
    u.fireEffectData.heat[i] = 0;
  }
}

// updates fire effect
boolean updateFire(boolean stop)
{

  if (!stop) {


    // Step 1.  Cool down every cell a little
    for ( int i = 0; i < numLeds + 3; i++) {
      u.fireEffectData.heat[i] = qsub8( u.fireEffectData.heat[i],  random8(0, ((COOLING * 10) / (numLeds + 3)) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for ( int k = numLeds - 1 + 3; k >= 2; k--) {
      u.fireEffectData.heat[k] = (u.fireEffectData.heat[k - 1] + u.fireEffectData.heat[k - 2] + u.fireEffectData.heat[k - 2] ) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if ( random8() < SPARKING ) {
      int y = random8(7);
      u.fireEffectData.heat[y] = qadd8( u.fireEffectData.heat[y], random8(160, 255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for ( int j = 0; j < numLeds; j++) {
      //  byte colorindex = scale8( u.fireEffectData.heat[j + 3], 240);
      //  Serial.println(colorindex);
      CRGB color = ColorFromPalette( heatMap, u.fireEffectData.heat[j + 3]);
      leds[j] = color;
      //leds[j] = HeatColor( u.fireEffectData.heat[j + 3]);
    }

    FastLED.show(); // display this frame

    return false;
  }
  else {

    darker(5);
    FastLED.show(); // display this frame

    if (sumLeds() == 0) {
      return true;
    }
    return false;
  }

}

//--------------------------------------------------------------------------
// Trail Effect
//--------------------------------------------------------------------------

boolean updateTrail1(boolean stop) {
  if (!stop) {

    shiftDown();

    CHSV color = CHSV(u.trailEffectData.phase & 0xff, 255, 255);
    u.trailEffectData.phase++;

    leds[numLeds - 1] = color;
    FastLED.show(); // display this frame

    // do normal things
    return false;
  }
  else {
    // make leds darker until total black
    darker(5);
    FastLED.show(); // display this frame

    unsigned long sum = sumLeds();

    if (sum == 0) {
      return true;
    }
    return false;
  }
}

void initTrail1() {
  u.trailEffectData.phase = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Candle Effect
/////////////////////////////////////////////////////////////////////////////////////////

#define MIN_DELAY_LEVEL 0
#define MAX_DELAY_LEVEL 100

#define MIN_SUSTAIN_LEVEL (MAX_DELAY_LEVEL)
#define MAX_SUSTAIN_LEVEL 255

#define MIN_ATTACK_PHASE_NUM 10
#define MAX_ATTACK_PHASE_NUM 50
#define MIN_SUSTAIN_PHASE_NUM 10
#define MAX_SUSTAIN_PHASE_NUM 100
#define MIN_RELEASE_PHASE_NUM 10
#define MAX_RELEASE_PHASE_NUM 50
#define MIN_DELAY_PHASE_NUM 10
#define MAX_DELAY_PHASE_NUM 50

//#define MIN_PHASE  16
//#define MAX_PHASE_BITS  5
//#define MAX_PHASE  32

void printCRGB(CRGB* color) {
  /* Serial.print(color->r);
   Serial.print("/");
   Serial.print(color->g);
   Serial.print("/");
   Serial.println(color->b);
  */



}

// update single candle, set the result color in led
void updateSingleCandle(struct CANDLE_DATA* c) {


  // randomize candle if needed
  if (c->phase > c->delayEndPhase) {
    randomizeCandle(c);
  }
  else
    // reset phase counter on phase changes
    if (
      (c->phase == c->attackEndPhase + 1)
      ||
      (c->phase == c->sustainEndPhase + 1)
      ||
      (c->phase == c->releaseEndPhase + 1)
    )
    {
      c->phaseCounter = 0;
    }


  // attack phase
  if (c->phase <= c->attackEndPhase) {
    if (c->phaseCounter == 0) {
      // set initial level
      c->level = c->attackLevel << 8;
    }
    else {
      // increase level
      c->level += c->attackInc;
    }
  }
  else if (c->phase <= c->sustainEndPhase) {
    if (c->phaseCounter == 0) {
      c->level = c->sustainLevel << 8;
    }
  }
  else if (c->phase <= c->releaseEndPhase) {
    if (c->phaseCounter > 0) {
      c->level -= c->releaseDec;
    }
  }
  c->phase++;
  c->phaseCounter++;


}

void updateLedsForCandles() {
  for (int i = 0; i < numLeds; i++) {

    byte candleIndex = u.candleEffectData.candleMap[i];
    int index  = u.candleEffectData.candle[candleIndex].level >> 8;



    byte colorindex = scale8( index, 240);
    /* Serial.print(index);
     Serial.print("->");
     Serial.println(colorindex);*/
    CRGB color = ColorFromPalette( heatMap, colorindex);
    leds[i] = color;
    // printCRGB(&color);
  }

}

boolean updateCandle(boolean stop) {
  if (!stop) {

    for (int i = 0; i < NUM_CANDLES; i++) {
      updateSingleCandle(&u.candleEffectData.candle[i]);
    }
    updateLedsForCandles();
    FastLED.show(); // display this frame

    // do normal things
    return false;
  }
  else {
    // make leds darker until total black
    darker(5);
    FastLED.show(); // display this frame

    unsigned long sum = sumLeds();

    if (sum == 0) {
      return true;
    }
    return false;
  }
}

// randomize candle
void randomizeCandle(struct CANDLE_DATA* c) {

  if (c->phase = 0) {
    // start level
    c->attackLevel = 0;
    c->level = 0;
  }
  else {
    c->attackLevel = c->delayLevel;
    c->level = c->attackLevel << 8;
  }
  // high level
  c->sustainLevel = random(MIN_SUSTAIN_LEVEL, MAX_SUSTAIN_LEVEL);
  // end level
  c->delayLevel = random(MIN_DELAY_LEVEL, MAX_DELAY_LEVEL);
  // number of phases for attack
  c->attackEndPhase = random(MIN_ATTACK_PHASE_NUM, MAX_ATTACK_PHASE_NUM);
  c->sustainEndPhase = random(c->attackEndPhase + MIN_SUSTAIN_PHASE_NUM, c->attackEndPhase + MAX_SUSTAIN_PHASE_NUM);
  c->releaseEndPhase = random(c->sustainEndPhase + MIN_RELEASE_PHASE_NUM, c->sustainEndPhase + MAX_RELEASE_PHASE_NUM);

  c->delayEndPhase = random(c->releaseEndPhase + MIN_DELAY_PHASE_NUM, c->releaseEndPhase + MAX_DELAY_PHASE_NUM);
  // current phase
  c->phase = 0;
  c->phaseCounter = 0;
  c->attackInc = ((c->sustainLevel - c->attackLevel) << 8) / c->attackEndPhase;

  unsigned int releaseLength = c->releaseEndPhase - c->sustainEndPhase;
  c->releaseDec = ((c->sustainLevel - c->delayLevel) << 8) / releaseLength;
}

void initCandle() {
  // init heat map

  heatMap = CRGBPalette256( CRGB::Black, CRGB::Red, CRGB::Yellow);
  /*
  for (int i = 0; i < 0xff; i++) {
    CHSV colorHSV = CHSV(i >> 2, 255, 0x80 + i >> 1);
    CRGB colorRGB;
    hsv2rgb_rainbow(colorHSV, colorRGB);
    u.candleEffectData.heatMap[i * 3] = colorRGB.r;
    u.candleEffectData.heatMap[i * 3 + 1] = colorRGB.g;
    u.candleEffectData.heatMap[i * 3 + 2] = colorRGB.b;
  }
  */

  // init candles
  for (int i = 0; i < NUM_CANDLES; i++) {
    // this indicates the initial state
    u.candleEffectData.candle[i].phase = 0;
    randomizeCandle(&u.candleEffectData.candle[i]);
  }
  for (int i = 0; i < numLeds; i++) {
    u.candleEffectData.candleMap[i] = random(0, NUM_CANDLES - 1);
  }
}

unsigned long sumLeds() {
  unsigned long result = 0;
  for (int i = 0; i < numLeds; i++) {
    /*   Serial.print(i);
       Serial.print(" : ");
       Serial.print(leds[i].r);
       Serial.print(",");

       Serial.print(leds[i].g);
       Serial.print(",");
       Serial.print(leds[i].b);
       Serial.println();
      */
    result += leds[i].r;
    result += leds[i].g;
    result += leds[i].b;
  }
  return result;
}

/*
void print(char* text, long value) {
  Serial.print(text);
  Serial.println(value);
}
*/

// copy the color from sourceIndex to destIndex
void copy(int sourceIndex, int destIndex) {
  if (destIndex >= 0 && destIndex < numLeds) {
    CRGB sourceColor = CRGB(0, 0, 0);
    if (sourceIndex > 0 && sourceIndex < numLeds) {
      sourceColor = leds[sourceIndex];
    }
    leds[destIndex] = leds[sourceIndex];
  }
}

CRGB get(int index) {
  if (index >= 0 && index <= numLeds) {
    return leds[index];
  }
  return CRGB(0, 0, 0);
}

// Shifts from top (NUM_LED-1) to down (0)
void shiftDown() {
  for (int i = 1; i <= numLeds; i++) {
    leds[i - 1] = leds[i];
  }
}

void darker(byte value) {

  for (int i = 0; i < numLeds; i++) {
    leds[i].r = qsub8(leds[i].r, value);
    leds[i].g = qsub8(leds[i].g, value);
    leds[i].b = qsub8(leds[i].b, value);
  }
}


