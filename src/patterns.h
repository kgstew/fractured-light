#ifndef PATTERNS_H
#define PATTERNS_H

#include <FastLED.h>

#define NUM_LEDS_PER_STRIP 122
#define NUM_STRIPS_PER_PIN 2

extern CRGB leds[];

void breathingPattern(int pins[], int numPins, int speed, CRGB color, bool reverse = false);
void flamepattern(int pins[], int numPins, int speed, int cooling, int sparking, bool reverse = false);
void growPattern(int pins[], int numPins, int speed, int n, int fadeDelay, int holdDelay, CRGB color, bool reverse = false);
void chasePattern(int pins[], int numPins, int speed, CRGB palette[], int paletteSize, int transitionSpeed, int holdDelay, int offsetDelay, bool reverse = false);

void resetBreathingPattern();
void resetFlamePattern();
void resetGrowPattern();
void resetChasePattern();

#endif