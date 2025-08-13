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
void spinPattern(int pins[], int numPins, int speed, int separation, int span, CRGB palette[], int paletteSize, bool loop, bool continuous, bool blend, bool reverse = false);
void flashbulbPattern(int pins[], int numPins, int flashDuration, int fadeDuration, int transitionDuration, bool reverse = false);

void resetBreathingPattern();
void resetFlamePattern();
void resetGrowPattern();
void resetPopPattern();
void resetSpinPattern();
void resetFlashbulbPattern();

// Pattern state save/restore functions
struct PatternState {
    // Breathing state
    struct {
        unsigned long lastUpdateTime;
        int currentBrightness;
        bool increasing;
    } breathing;
    
    // Spin state 
    struct {
        unsigned long lastUpdateTime;
        int currentPosition[8];
    } spin;
    
    // Add other pattern states as needed
};

void savePatternStates(PatternState* state);
void restorePatternStates(PatternState* state);

#endif