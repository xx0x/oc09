#include "Arduino.h"
#include "../Animation.h"

class AnimationGalaxy : public Animation
{

public:
    void setup()
    {
        turnOffAfterFinish = true;
        nextWait = 0;
        currentDigit = 0;
        currentValue = 0;
    }

    int draw()
    {
        pixels.clear();
        if(currentDigit > 3) return 0;
        if (nextWait > 0)
        {
            pixels.show();
            int rt = nextWait;
            nextWait = 0;
            return rt;
        }

        byte val = digits[currentDigit];
        if (val == 0)
        {
            for (byte i = 0; i < NUMPIXELS; i++)
            {
                setPixelColorToBase(i);
            }
            pixels.show();
            currentDigit++;
            createNiceRandomColor(baseColor);
            if (currentDigit <= 3)
            {
                nextWait = separateTime;
            }
            return 100;
        }
        else
        {
            if (currentValue < val)
            {
                if(!keepLightsOn){
                    setPixelColorToBase(pixelsAddresses[currentValue]);
                }else{
                    for(byte i = 0; i <= currentValue; i++){
                        setPixelColorToBase(pixelsAddresses[i]);
                    }
                }
                currentValue++;
                pixels.show();
                nextWait = pauseTime;

                if(keepLightsOn && currentValue == val){
                    return holdTime * keepLastExtension;// last, make it last!
                } 

                return holdTime;
            }
            else if (currentDigit < 3)
            {
                currentValue = 0;
                currentDigit++;
                pixels.show();
                createNiceRandomColor(baseColor);
                shuffleAddresses(pixelsAddresses);
                return separateTime;
            }
        }
        return 0;
    }

private:
    int nextWait;
    byte currentValue;
    byte currentDigit;
};
