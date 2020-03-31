#include "Arduino.h"
#include "../Animation.h"

#define CURRENT_DELAY 150

class AnimationCurrent : public Animation
{

public:
    void setup()
    {
        turnOffAfterFinish = false;
        currentStep = 0;
    }

    AnimationCurrent(byte ca)
    {
        currentAnimation = ca;
    }

    int draw()
    {
        pixels.clear();
        if (currentStep % 2 == 0 && currentStep < 6)
        {
            for (byte i = 0; i <= currentAnimation; i++)
            {
                pixels.setPixelColor(i, WHITE);
            }
        }
        currentStep++;
        pixels.show();
        return currentStep < 12 ? CURRENT_DELAY : 0;
    }

private:
    byte currentAnimation;
    byte currentStep;
};
