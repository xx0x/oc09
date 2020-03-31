#ifndef Animation_h
#define Animation_h

#include "Arduino.h"

#define DEFAULT_HOLD 100
#define DEFAULT_PAUSE 30
#define DEFAULT_SEPARATE 500

class Animation
{
public:
  virtual void setup();
  virtual int draw();

  bool turnOffAfterFinish = false;
  int holdTime = DEFAULT_HOLD;
  int separateTime = DEFAULT_SEPARATE;
  int pauseTime = DEFAULT_PAUSE;

  void setDigits(byte a, byte b, byte c, byte d)
  {
    digits[0] = a;
    digits[1] = b;
    digits[2] = c;
    digits[3] = d;
  }

  void setPixelColorToBase(byte pixel)
  {
    pixels.setPixelColor(pixel, baseColor[0], baseColor[1], baseColor[2]);
  }

protected:
  byte digits[4] = {0, 0, 0, 0};
};

#endif
