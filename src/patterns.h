#ifndef PATTERNS_H
#define PATTERNS_H

#include <FastLED.h>

#define MAX_LEDS_PER_PIN 122

extern CRGB leds1[];
extern CRGB leds2[];
extern CRGB leds3[];
extern CRGB leds4[];
extern CRGB leds5[];
extern CRGB leds6[];
extern CRGB leds7[];
extern CRGB leds8[];

// Helper function to get LED array for a specific pin (0-indexed)
CRGB* getLedArrayForPin(int pin);

void breathingPattern(int pins[], int numPins, int speed, CRGB palette[], int paletteSize, bool reverse = false);
void flamepattern(int pins[], int numPins, int speed, int cooling, int sparking, bool reverse = false);
void growPattern(int pins[], int numPins, int speed, int n, int fadeDelay, int holdDelay, CRGB palette[],
    int paletteSize, int transitionSpeed, int offsetDelay, bool reverse = false);
void popPattern(int pins[], int numPins, int speed, int holdDelay, CRGB palette[], int paletteSize, bool random, int accelerationTime, bool reverse = false);
void spinPattern(int pins[], int numPins, int speed, int separation, int span, CRGB palette[], int paletteSize, bool loop, bool continuous, bool blend, bool reverse = false);

void resetBreathingPattern();
void resetFlamePattern();
void resetGrowPattern();
void resetPopPattern();
void resetSpinPattern();

#endif