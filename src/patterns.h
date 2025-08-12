#ifndef PATTERNS_H
#define PATTERNS_H

#include <FastLED.h>

#define NUM_LEDS_PER_STRIP 122
#define NUM_STRIPS_PER_PIN 2

extern CRGB leds[];

void breathingPattern(int pins[], int numPins, int speed, CRGB color);
void flamepattern(int pins[], int numPins, int speed, int cooling, int sparking);

#endif