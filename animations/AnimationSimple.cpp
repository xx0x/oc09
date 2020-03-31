#include "Arduino.h"
#include "../Animation.h"

#define SIMPLE_HOLD 300
#define SIMPLE_SEPARATE 500

class AnimationSimple : public Animation
{

public:
    void setup()
    {
        turnOffAfterFinish = true;
        currentDigit = 0;
    }

    int draw()
    {
        pixels.clear();
        if (nextWait > 0)
        {
            int rt = nextWait;
            nextWait = 0;
            pixels.show();
            return rt;
        }
        if (currentDigit >= 4)
        {
            pixels.show();
            return 0;
        }
        createNiceRandomColor(baseColor);

        byte val = digits[currentDigit];
        if (val > 0)
        {
            setPixelColorToBase(val - 1);
        }
        else
        {
            for (byte i = 0; i < NUMPIXELS; i++)
            {
                setPixelColorToBase(i);
            }
        }
        pixels.show();

        currentDigit++;
        nextWait = SIMPLE_SEPARATE;
        return SIMPLE_HOLD;
    }

private:
    int nextWait;
    byte currentDigit;
};
