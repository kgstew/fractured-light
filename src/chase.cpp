#include "patterns.h"
#include <Arduino.h>

static unsigned long lastUpdate[8] = { 0 };
static int currentPhase[8] = { 0 }; // 0: chasing forward, 1: holding, 2: chasing backward
static int currentLed[8] = { 0 };
static unsigned long phaseStartTime[8] = { 0 };
static unsigned long colorTransitionTime[8] = { 0 };
static int currentColorIndex[8] = { 0 };
static float colorTransitionProgress[8] = { 0.0 };

void chasePattern(int pins[], int numPins, int speed, CRGB palette[], int paletteSize, int transitionSpeed, int holdDelay, bool reverse)
{
    if (speed == 0 || paletteSize == 0) return;

    unsigned long currentTime = millis();
    unsigned long chaseInterval = map(speed, 1, 100, 200, 20);
    unsigned long colorInterval = map(transitionSpeed, 1, 100, 100, 10);

    for (int p = 0; p < numPins; p++) {
        int pin = pins[p];
        int startIndex = pin * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
        int totalLeds = NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;

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

        if (currentTime - lastUpdate[pin] >= chaseInterval) {
            lastUpdate[pin] = currentTime;

            switch (currentPhase[pin]) {
                case 0: // Chasing forward - LEDs accumulate
                    {
                        // Light all LEDs up to current position
                        for (int i = 0; i <= currentLed[pin]; i++) {
                            int ledIndex;
                            if (reverse) {
                                ledIndex = startIndex + (totalLeds - 1 - i);
                            } else {
                                ledIndex = startIndex + i;
                            }
                            leds[ledIndex] = currentColor;
                        }
                        
                        currentLed[pin]++;
                        
                        if (currentLed[pin] >= totalLeds) {
                            currentPhase[pin] = 1; // Switch to holding phase
                            phaseStartTime[pin] = currentTime;
                            currentLed[pin] = totalLeds - 1;
                        }
                    }
                    break;
                    
                case 1: // Holding phase
                    {
                        // Keep all LEDs on during hold with current color
                        for (int i = 0; i < totalLeds; i++) {
                            int ledIndex;
                            if (reverse) {
                                ledIndex = startIndex + (totalLeds - 1 - i);
                            } else {
                                ledIndex = startIndex + i;
                            }
                            leds[ledIndex] = currentColor;
                        }
                        
                        if (currentTime - phaseStartTime[pin] >= (unsigned long)holdDelay) {
                            currentPhase[pin] = 2; // Switch to chasing backward
                            currentLed[pin] = totalLeds - 1;
                        }
                    }
                    break;
                    
                case 2: // Chasing backward - LEDs turn off one by one
                    {
                        // Clear all LEDs first
                        for (int i = 0; i < totalLeds; i++) {
                            leds[startIndex + i] = CRGB::Black;
                        }
                        
                        // Light LEDs up to current position
                        for (int i = 0; i <= currentLed[pin]; i++) {
                            int ledIndex;
                            if (reverse) {
                                ledIndex = startIndex + (totalLeds - 1 - i);
                            } else {
                                ledIndex = startIndex + i;
                            }
                            leds[ledIndex] = currentColor;
                        }
                        
                        currentLed[pin]--;
                        
                        if (currentLed[pin] < 0) {
                            currentPhase[pin] = 0; // Reset to chasing forward
                            currentLed[pin] = 0;
                        }
                    }
                    break;
            }
        } else {
            // Update colors even when not advancing chase position
            switch (currentPhase[pin]) {
                case 0: // Update colors for accumulated LEDs
                    for (int i = 0; i <= currentLed[pin]; i++) {
                        int ledIndex;
                        if (reverse) {
                            ledIndex = startIndex + (totalLeds - 1 - i);
                        } else {
                            ledIndex = startIndex + i;
                        }
                        leds[ledIndex] = currentColor;
                    }
                    break;
                    
                case 1: // Update colors for all LEDs during hold
                    for (int i = 0; i < totalLeds; i++) {
                        int ledIndex;
                        if (reverse) {
                            ledIndex = startIndex + (totalLeds - 1 - i);
                        } else {
                            ledIndex = startIndex + i;
                        }
                        leds[ledIndex] = currentColor;
                    }
                    break;
                    
                case 2: // Update colors for remaining LEDs
                    for (int i = 0; i < totalLeds; i++) {
                        leds[startIndex + i] = CRGB::Black;
                    }
                    for (int i = 0; i <= currentLed[pin]; i++) {
                        int ledIndex;
                        if (reverse) {
                            ledIndex = startIndex + (totalLeds - 1 - i);
                        } else {
                            ledIndex = startIndex + i;
                        }
                        leds[ledIndex] = currentColor;
                    }
                    break;
            }
        }
    }

    FastLED.show();
}

void resetChasePattern()
{
    for (int i = 0; i < 8; i++) {
        lastUpdate[i] = 0;
        currentPhase[i] = 0;
        currentLed[i] = 0;
        phaseStartTime[i] = 0;
        colorTransitionTime[i] = 0;
        currentColorIndex[i] = 0;
        colorTransitionProgress[i] = 0.0;
    }
}