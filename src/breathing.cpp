#include "patterns.h"
#include <Arduino.h>

static unsigned long lastUpdate = 0;
static float brightness = 0.0;
static bool increasing = true;
static unsigned long colorTransitionTime = 0;
static float colorProgress = 0.0;

void breathingPattern(int pins[], int numPins, int speed, CRGB palette[], int paletteSize, bool reverse)
{
    if (speed == 0 || paletteSize == 0)
        return;

    unsigned long currentTime = millis();
    unsigned long interval = map(speed, 1, 100, 100, 5);

    if (currentTime - lastUpdate >= interval) {
        lastUpdate = currentTime;

        // Update breathing brightness
        if (increasing) {
            brightness += 1.0;
            if (brightness >= 255.0) {
                brightness = 255.0;
                increasing = false;
            }
        } else {
            brightness -= 1.0;
            if (brightness <= 0.0) {
                brightness = 0.0;
                increasing = true;
            }
        }

        // Update color transition for multi-color palettes
        CRGB currentColor;
        if (paletteSize == 1) {
            currentColor = palette[0];
        } else {
            // Smooth color transitions through palette
            unsigned long colorInterval = 50; // Color transition speed
            if (currentTime - colorTransitionTime >= colorInterval) {
                colorTransitionTime = currentTime;
                colorProgress += 0.01; // Increment color progress
                
                if (colorProgress >= paletteSize) {
                    colorProgress = 0.0;
                }
            }
            
            // Calculate current color by blending between palette colors
            int colorIndex1 = (int)colorProgress % paletteSize;
            int colorIndex2 = (colorIndex1 + 1) % paletteSize;
            float blendAmount = colorProgress - (int)colorProgress;
            
            // Use FastLED's lerp8 for smooth blending
            currentColor = palette[colorIndex1].lerp8(palette[colorIndex2], (uint8_t)(blendAmount * 255));
        }

        // Apply breathing brightness to the current color
        CRGB scaledColor = currentColor;
        scaledColor.nscale8((uint8_t)brightness);

        for (int p = 0; p < numPins; p++) {
            int pin = pins[p];
            int startIndex = pin * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            int endIndex = startIndex + (NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN);

            if (reverse) {
                for (int i = endIndex - 1; i >= startIndex; i--) {
                    leds[i] = scaledColor;
                }
            } else {
                for (int i = startIndex; i < endIndex; i++) {
                    leds[i] = scaledColor;
                }
            }
        }

        FastLED.show();
    }
}

void resetBreathingPattern()
{
    lastUpdate = 0;
    brightness = 0.0;
    increasing = true;
    colorTransitionTime = 0;
    colorProgress = 0.0;
}