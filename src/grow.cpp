#include "patterns.h"
#include <Arduino.h>

static unsigned long lastUpdate[8] = { 0 };
static int currentPhase[8] = { 0 }; // 0: growing, 1: holding, 2: shrinking
static int activeLeds[8] = { 0 };
static unsigned long phaseStartTime[8] = { 0 };
static unsigned long nextLedTime[8] = { 0 };
static float brightness[8][NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN] = { 0 };
static unsigned long colorTransitionTime[8] = { 0 };
static int currentColorIndex[8] = { 0 };
static float colorTransitionProgress[8] = { 0.0 };
static unsigned long patternStartTime = 0;
static bool patternInitialized = false;

void growPattern(int pins[], int numPins, int speed, int n, int fadeDelay, int holdDelay, CRGB palette[], int paletteSize, int transitionSpeed, int offsetDelay, bool reverse)
{
    if (n == 0 || speed == 0 || paletteSize == 0) return;
    
    unsigned long currentTime = millis();
    unsigned long colorInterval = map(transitionSpeed, 1, 100, 100, 10);

    // Initialize pattern start time on first call
    if (!patternInitialized) {
        patternStartTime = currentTime;
        patternInitialized = true;
    }

    for (int p = 0; p < numPins; p++) {
        int pin = pins[p];
        int startIndex = pin * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
        int totalLeds = NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;

        // Calculate offset delay for this pin
        unsigned long pinOffsetDelay = (unsigned long)offsetDelay * p;
        
        // Check if this pin should start yet
        if (currentTime - patternStartTime < pinOffsetDelay) {
            // Pin hasn't started yet, keep LEDs off
            for (int i = 0; i < totalLeds; i++) {
                leds[startIndex + i] = CRGB::Black;
            }
            continue;
        }

        // Update color transition
        if (currentTime - colorTransitionTime[pin] >= colorInterval) {
            colorTransitionTime[pin] = currentTime;
            colorTransitionProgress[pin] += 0.02; // Increment transition progress
            
            if (colorTransitionProgress[pin] >= 1.0) {
                colorTransitionProgress[pin] = 0.0;
                currentColorIndex[pin] = (currentColorIndex[pin] + 1) % paletteSize;
            }
        }

        // Calculate current color by blending between palette colors
        CRGB currentColor;
        if (paletteSize == 1) {
            currentColor = palette[0];
        } else {
            int fromIndex = currentColorIndex[pin];
            int toIndex = (currentColorIndex[pin] + 1) % paletteSize;
            currentColor = palette[fromIndex].lerp8(palette[toIndex], (uint8_t)(colorTransitionProgress[pin] * 255));
        }

        if (currentTime >= nextLedTime[pin]) {
            switch (currentPhase[pin]) {
                case 0: // Growing phase
                    if (activeLeds[pin] < totalLeds) {
                        // Add n LEDs (or remaining LEDs if less than n)
                        int ledsToAdd = min(n, totalLeds - activeLeds[pin]);
                        
                        // Fade in the new LEDs
                        for (int i = 0; i < ledsToAdd; i++) {
                            int ledIndex;
                            if (reverse) {
                                ledIndex = startIndex + (totalLeds - 1 - activeLeds[pin] - i);
                            } else {
                                ledIndex = startIndex + activeLeds[pin] + i;
                            }
                            
                            // Start with black and fade to full color
                            static float brightness[8][NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN] = { 0 };
                            brightness[pin][reverse ? (totalLeds - 1 - activeLeds[pin] - i) : (activeLeds[pin] + i)] = 0;
                        }
                        
                        activeLeds[pin] += ledsToAdd;
                        nextLedTime[pin] = currentTime + fadeDelay;
                        
                        if (activeLeds[pin] >= totalLeds) {
                            currentPhase[pin] = 1; // Switch to holding phase
                            phaseStartTime[pin] = currentTime;
                        }
                    }
                    break;
                    
                case 1: // Holding phase
                    if (currentTime - phaseStartTime[pin] >= (unsigned long)holdDelay) {
                        currentPhase[pin] = 2; // Switch to shrinking phase
                        nextLedTime[pin] = currentTime;
                    }
                    break;
                    
                case 2: // Shrinking phase
                    if (activeLeds[pin] > 0) {
                        // Remove n LEDs (or remaining LEDs if less than n)
                        int ledsToRemove = min(n, activeLeds[pin]);
                        activeLeds[pin] -= ledsToRemove;
                        nextLedTime[pin] = currentTime + fadeDelay;
                        
                        if (activeLeds[pin] <= 0) {
                            currentPhase[pin] = 0; // Reset to growing phase
                            activeLeds[pin] = 0;
                        }
                    }
                    break;
            }
        }

        // Update LED display with fade effect
        
        // Calculate fade update interval based on speed (0-100 -> 50-5ms)
        unsigned long fadeInterval = map(speed, 1, 100, 50, 5);
        
        if (currentTime - lastUpdate[pin] >= fadeInterval) {
            lastUpdate[pin] = currentTime;
            
            for (int i = 0; i < totalLeds; i++) {
                int ledIndex;
                if (reverse) {
                    ledIndex = startIndex + (totalLeds - 1 - i);
                } else {
                    ledIndex = startIndex + i;
                }
                
                bool shouldBeOn = false;
                if (currentPhase[pin] == 0) { // Growing
                    shouldBeOn = (i < activeLeds[pin]);
                } else if (currentPhase[pin] == 1) { // Holding
                    shouldBeOn = true;
                } else { // Shrinking
                    shouldBeOn = (i < activeLeds[pin]);
                }
                
                // Calculate fade step based on speed (faster speed = bigger steps)
                float fadeStep = map(speed, 1, 100, 2, 15);
                
                if (shouldBeOn && brightness[pin][i] < 255.0) {
                    brightness[pin][i] += fadeStep; // Fade in
                    if (brightness[pin][i] > 255.0) brightness[pin][i] = 255.0;
                } else if (!shouldBeOn && brightness[pin][i] > 0.0) {
                    brightness[pin][i] -= fadeStep; // Fade out
                    if (brightness[pin][i] < 0.0) brightness[pin][i] = 0.0;
                }
                
                CRGB scaledColor = currentColor;
                scaledColor.nscale8((uint8_t)brightness[pin][i]);
                leds[ledIndex] = scaledColor;
            }
        }
    }

    FastLED.show();
}

void resetGrowPattern()
{
    for (int i = 0; i < 8; i++) {
        lastUpdate[i] = 0;
        currentPhase[i] = 0;
        activeLeds[i] = 0;
        phaseStartTime[i] = 0;
        nextLedTime[i] = 0;
        colorTransitionTime[i] = 0;
        currentColorIndex[i] = 0;
        colorTransitionProgress[i] = 0.0;
        for (int j = 0; j < NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN; j++) {
            brightness[i][j] = 0.0;
        }
    }
    patternStartTime = 0;
    patternInitialized = false;
}