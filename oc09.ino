/**
 * oc09
 * clock // pendant
 * xx0x.cz
 * 
 * the code is a little bit messy, sorry...
 */
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <Adafruit_NeoPixel.h>
#include <DS3232RTC.h> // https://github.com/JChristensen/DS3232RTC
#include <TinyWireM.h>
#include <TimeLib.h>

// Pins
#define PIN_NEOPIXELS 1
#define PIN_ENABLE 3
#define PIN_BUTTON 4

// RGB pixels
#define NUMPIXELS 9
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN_NEOPIXELS, NEO_GRB + NEO_KHZ800);

// SLEEP TIMINGS n * 4096ms
#define TIMING_MODES 5
byte timingModes[TIMING_MODES] = {1, 2, 4, 6, 12};
byte currentTiming = 0;
byte wakeUpCount = 0;

// Time h1h2:m1m2
byte h1 = 0;
byte h2 = 0;
byte m1 = 0;
byte m2 = 0;

// Colors
byte baseColor[3] = {255, 0, 0};
byte pixelsAddresses[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
#define RED 0xFF0000
#define GREEN 0x00FF00
#define BLUE 0x0000FF
#define YELLOW 0x777700
#define WHITE 0x888888
#define GRAY 0x333333

// Animations definitions
#include "functions.h"
#include "Animation.h"
#include "animations/AnimationGalaxy.cpp"
#include "animations/AnimationSimple.cpp"
#include "animations/AnimationCurrent.cpp"

// Animations
Animation *animation;
bool animationEnded = true;
bool forceReload = true;
byte currentAnimation = 0;
#define ANIMATION_COUNT 3
unsigned long nextTimeAnimation = 0;

// Buttons
#define IS_BUTTON_PRESSED digitalRead(PIN_BUTTON) == LOW
#define IS_BUTTON_RELEASED digitalRead(PIN_BUTTON) != LOW
#define BUTTON_LONG_PRESS 1000
#define BUTTON_OFF_PRESS 2000
bool buttonPressed = false;
bool ignoreNextPress = false;
unsigned long buttonPressedTime = 0;
bool longPressHappening = false;
bool wakenUpByButton = false;

// Menu
#define MENU_ITEMS 4

#define MENU_BRIGHTNESS 0
#define MENU_TIMINGS 1
#define MENU_SET 2
#define MENU_EXIT 3
#define MENU_VALUE_OFFSET 4
byte currentMenuItem = 0;
byte menuDetailSetting = 0;

// Modes
#define MODE_SET 0
#define MODE_CLOCK 1
#define MODE_MENU 2
#define MODE_MENU_DETAIL 3
byte currentMode = MODE_CLOCK;

// Time setting
byte digitValues[4] = {0, 0, 0, 0};
byte currentSettingDigit = 0;

// Brighness setting
#define BRIGHTNESS_MODES 5
byte brightnessModes[BRIGHTNESS_MODES] = {4, 8, 32, 64, 128};
byte currentBrightness = 2;

void setup()
{
    pinMode(PIN_ENABLE, OUTPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    digitalWrite(PIN_ENABLE, HIGH);
    setClock(00, 00);
    drawIntroAnimation();
    delay(1000);
}

void loop()
{
    buttonsCheck();
    switch (currentMode)
    {
    case MODE_CLOCK:
        drawClock();
        break;
    case MODE_MENU:
        drawMenu();
        break;
    case MODE_MENU_DETAIL:
        drawMenuDetail();
        break;
    case MODE_SET:
        drawSet();
        break;
    }
}

void drawIntroAnimation()
{
    pixels.begin();
    pixels.setBrightness(brightnessModes[currentBrightness]);
    for (byte i = 0; i < NUMPIXELS; i++)
    {
        pixels.clear();
        createNiceRandomColor(baseColor);
        pixels.setPixelColor(i, baseColor[0], baseColor[1], baseColor[2]);
        pixels.show();
        delay(150);
    }
    pixels.clear();
    pixels.show();
}

void drawClock()
{
    if (animationEnded || forceReload)
    {
        readTime();
        reloadMode();
    }

    if (!animationEnded && nextTimeAnimation < millis())
    {
        int waitFor = animation->draw();
        if (waitFor != 0)
        {
            nextTimeAnimation = millis() + waitFor;
        }
        else
        {
            pixels.clear();
            pixels.show();
            animationEnded = true;
            if (!longPressHappening && animation->turnOffAfterFinish)
            {
                automaticSleep();
            }
        }
    }
}

void drawMenu()
{
    pixels.clear();
    pixels.setPixelColor(currentMenuItem, RED);
    pixels.show();
    delay(50);
    pixels.clear();
    pixels.show();
    delay(50);
}

void drawMenuDetail()
{
    pixels.clear();
    switch (currentMenuItem)
    {
    case MENU_BRIGHTNESS:
        pixels.setPixelColor(currentMenuItem, RED);
        for (byte i = 0; i <= currentBrightness; i++)
        {
            pixels.setPixelColor(MENU_VALUE_OFFSET + i, GREEN);
        }
        break;
    case MENU_TIMINGS:
        pixels.setPixelColor(currentMenuItem, RED);
        for (byte i = 0; i <= currentTiming; i++)
        {
            pixels.setPixelColor(MENU_VALUE_OFFSET + i, BLUE);
        }
        break;
    }
    pixels.show();
    delay(50);
}

void drawSet()
{
    pixels.clear();
    uint32_t color;
    switch (currentSettingDigit)
    {
    case 0:
        color = RED;
        break;
    case 1:
        color = GREEN;
        break;
    case 2:
        color = BLUE;
        break;
    case 3:
        color = YELLOW;
        break;
    }
    if (digitValues[currentSettingDigit] > 0)
    {
        pixels.setPixelColor(digitValues[currentSettingDigit] - 1, color);
    }
    else
    {
        for (byte i = 0; i < NUMPIXELS; i++)
        {
            pixels.setPixelColor(i, color);
        }
    }
    pixels.show();
    delay(50);
}

void buttonsCheck()
{
    if (longPressHappening && IS_BUTTON_PRESSED)
    {
        if (millis() - buttonPressedTime >= BUTTON_OFF_PRESS)
        {
            longPressHappening = false;
            manualSleep();
        }
        return;
    }
    longPressHappening = false;

    if (IS_BUTTON_PRESSED && !buttonPressed)
    {
        buttonPressed = true;
        buttonPressedTime = millis();
    }
    if (IS_BUTTON_RELEASED && buttonPressed)
    {
        buttonPressed = false;
        if (!ignoreNextPress)
        {
            shortPress();
        }
        else
        {
            ignoreNextPress = false;
        }
    }
    if (buttonPressed)
    {
        if (millis() - buttonPressedTime >= BUTTON_LONG_PRESS)
        {
            buttonPressed = false;
            longPressHappening = true;
            longPress();
            ignoreNextPress = false;
        }
    }
}

void shortPress()
{

    switch (currentMode)
    {
    case MODE_CLOCK:
        nextAnimation();
        break;
    case MODE_MENU:
        nextMenuItem();
        break;
    case MODE_MENU_DETAIL:
        switch (currentMenuItem)
        {
        case MENU_BRIGHTNESS:
            nextBrightness();
            break;
        case MENU_TIMINGS:
            nextTiming();
            break;
        }

        break;
    case MODE_SET:
        digitValues[currentSettingDigit]++;
        byte limit = 9;
        switch (currentSettingDigit)
        {
        case 0:
            limit = 2;
            break;
        case 1:
            if (digitValues[0] == 2)
            {
                limit = 3;
            }
            break;
        case 2:
            limit = 5;
            break;
        }
        if (digitValues[currentSettingDigit] > limit)
        {
            digitValues[currentSettingDigit] = 0;
        }
        break;
    }
}

void longPress()
{
    switch (currentMode)
    {
    case MODE_SET:
        if (currentSettingDigit < 3)
        {
            currentSettingDigit++;
            digitValues[currentSettingDigit] = 0;
        }
        else
        {
            finishSettingTime();
            forceReload = true;
            currentMode = MODE_CLOCK;
        }
        break;
    case MODE_CLOCK:
        currentMenuItem = 0;
        currentMode = MODE_MENU;
        break;
    case MODE_MENU:
        switch (currentMenuItem)
        {
        case MENU_EXIT:
            currentMode = MODE_CLOCK;
            forceReload = true;
            delay(500);
            break;
        case MENU_TIMINGS:
        case MENU_BRIGHTNESS:
            currentMode = MODE_MENU_DETAIL;
            break;
        case MENU_SET:
            digitValues[0] = 0;
            digitValues[1] = 0;
            digitValues[2] = 0;
            digitValues[3] = 0;
            currentMode = MODE_SET;
            break;
        }
        break;
    case MODE_MENU_DETAIL:
        currentMode = MODE_MENU;
        break;
    }
}

void nextAnimation()
{
    currentAnimation++;
    if (currentAnimation >= ANIMATION_COUNT)
    {
        currentAnimation = 0;
    }
    nextTimeAnimation = 0;
    animationEnded = false;
    forceReload = false;
    delete animation;
    animation = new AnimationCurrent(currentAnimation);
    animation->setup();
}

void nextMenuItem()
{
    currentMenuItem++;
    if (currentMenuItem >= MENU_ITEMS)
    {
        currentMenuItem = 0;
    }
}

void nextBrightness()
{
    currentBrightness++;
    if (currentBrightness >= BRIGHTNESS_MODES)
    {
        currentBrightness = 0;
    }
    pixels.setBrightness(brightnessModes[currentBrightness]);
}

void nextTiming()
{
    currentTiming++;
    if (currentTiming >= TIMING_MODES)
    {
        currentTiming = 0;
    }
}

void reloadMode()
{
    delete animation;
    switch (currentAnimation)
    {
    case 0:
        animation = new AnimationSimple();
        break;
    case 1:
        animation = new AnimationGalaxy();
        animation->holdTime = 400;
        animation->separateTime = 800;
        animation->pauseTime = 100;
        break;
    case 2:
        animation = new AnimationGalaxy();
        break;
    }
    createNiceRandomColor(baseColor);
    shuffleAddresses(pixelsAddresses);
    animation->setDigits(h1, h2, m1, m2);
    animation->setup();
    animationEnded = false;
    nextTimeAnimation = 0;
    forceReload = false;
}

void finishSettingTime()
{
    currentSettingDigit = 0;
    byte hr = digitValues[0] * 10 + digitValues[1];
    if (hr > 23)
    {
        hr = 0;
    }
    byte min = digitValues[2] * 10 + digitValues[3];
    if (min > 59)
    {
        min = 0;
    }
    setClock(hr, min);
}

/**
 * Reads time from RTC
 */
void readTime()
{
    tmElements_t t;
    RTC.read(t);
    h1 = t.Hour / 10;
    h2 = t.Hour % 10;
    m1 = t.Minute / 10;
    m2 = t.Minute % 10;
}

void setClock(byte hr, byte min)
{
    tmElements_t tm;
    tm.Hour = hr;
    tm.Minute = min;
    tm.Second = 0;
    tm.Day = 1;
    tm.Month = 1;
    tm.Year = 1;
    RTC.write(tm);
}

void manualSleep()
{
    wakenUpByButton = false;
    for (short i = NUMPIXELS; i >= 0; i--)
    {
        pixels.clear();
        for (byte j = 0; j < i; j++)
        {
            pixels.setPixelColor(j, YELLOW);
        }
        pixels.show();
        delay(100);
    }
    delay(1000);
    digitalWrite(PIN_ENABLE, LOW);
    doSleep();
    digitalWrite(PIN_ENABLE, HIGH);
    currentMode = MODE_CLOCK;
}

void automaticSleep()
{
    wakenUpByButton = false;
    wakeUpCount = 0;
    digitalWrite(PIN_ENABLE, LOW);
    //wdt_reset();
    cli();
    MCUSR = 0x00;
    WDTCR |= _BV(WDCE) | _BV(WDE);
    WDTCR = _BV(WDIE) | _BV(WDP3); // 4s
                                   // WDTCR = _BV(WDIE) | _BV(WDP3) | _BV(WDP0); // 8s
    sei();
    do
    {
        doSleep();
        wakeUpCount++;
    } while (wakeUpCount < timingModes[currentTiming] && !wakenUpByButton);

    //wdt_reset();
    cli();
    MCUSR = 0x00;
    WDTCR |= _BV(WDCE) | _BV(WDE);
    WDTCR = 0x00;
    sei();

    if (wakenUpByButton)
    {
        delay(500);
    }

    currentMode = MODE_CLOCK;
    digitalWrite(PIN_ENABLE, HIGH);
}

// btn interrupt
ISR(PCINT0_vect)
{
    ignoreNextPress = true;
    wakenUpByButton = true;
}

// wdt interrupt
ISR(WDT_vect) {}

/**
 * Falling to sleep and waking up when button pressed.
 */
void doSleep()
{
    GIMSK |= _BV(PCIE);                  // Enable Pin Change Interrupts
    PCMSK |= _BV(PCINT4);                // Use PB4 as interrupt pin
    ADCSRA &= ~_BV(ADEN);                // ADC off
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // replaces above statement
    sleep_enable();                      // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
    sei();                               // Enable interrupts
    sleep_cpu();                         // sleep

    // NOW WAKING UP
    cli();                 // Disable interrupts
    PCMSK &= ~_BV(PCINT4); // Turn off PB4 as interrupt pin
    sleep_disable();       // Clear SE bit
    ADCSRA |= _BV(ADEN);   // ADC on
}
