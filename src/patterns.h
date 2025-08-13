#ifndef PATTERNS_H
#define PATTERNS_H

#include <FastLED.h>

#define NUM_LEDS_PER_STRIP 122
#define NUM_STRIPS_PER_PIN 2

extern CRGB leds[];

void breathingPattern(int pins[], int numPins, int speed, CRGB palette[], int paletteSize, bool reverse = false);
void flamepattern(int pins[], int numPins, int speed, int cooling, int sparking, bool reverse = false);
void growPattern(int pins[], int numPins, int speed, int n, int fadeDelay, int holdDelay, CRGB palette[],
    int paletteSize, int transitionSpeed, int offsetDelay, bool reverse = false);
void popPattern(int pins[], int numPins, int speed, int holdDelay, CRGB palette[], int paletteSize, bool random, int accelerationTime, bool reverse = false);
void spinPattern(int pins[], int numPins, int speed, int separation, int span, CRGB palette[], int paletteSize, bool continuous, bool blend, bool reverse = false);

void resetBreathingPattern();
void resetFlamePattern();
void resetGrowPattern();
void resetPopPattern();
void resetSpinPattern();

#endif