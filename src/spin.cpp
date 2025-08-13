#include "patterns.h"
#include <Arduino.h>
#include <FastLED.h>

unsigned long lastUpdateTime = 0;
int currentPosition[8] = { 0 };

void resetSpinPattern() {
    lastUpdateTime = 0;
    for (int i = 0; i < 8; i++) {
        currentPosition[i] = 0;
    }
}

void spinPattern(int pins[], int numPins, int speed, int separation, int span, CRGB palette[], int paletteSize, bool loop, bool continuous, bool blend, bool reverse) {
    if (numPins == 0 || paletteSize == 0 || span <= 0 || separation < 0) return;
    
    unsigned long currentTime = millis();
    unsigned long updateDelay = map(speed, 1, 100, 200, 10);
    
    if (currentTime - lastUpdateTime >= updateDelay) {
        lastUpdateTime = currentTime;
        
        for (int p = 0; p < numPins; p++) {
            int pin = pins[p];
            
            // Skip pins that are currently being used by flashbulb
            if (isPinUsedByFlashbulb(pin)) {
                continue;
            }
            
            int startIndex = pin * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            int totalLeds = NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            
            if (continuous) {
                // Continuous mode: light all LEDs transitioning through palette colors
                for (int i = 0; i < totalLeds; i++) {
                    // Calculate position in the pattern cycle
                    float cyclePos = (float)((i + currentPosition[pin]) % totalLeds) / totalLeds;
                    
                    // Scale to palette range and get fractional part for blending
                    float palettePos = cyclePos * paletteSize;
                    int colorIndex1 = (int)palettePos % paletteSize;
                    int colorIndex2 = (colorIndex1 + 1) % paletteSize;
                    float blendAmount = palettePos - (int)palettePos;
                    
                    CRGB color;
                    if (blend) {
                        // Use FastLED's lerp8 for smooth blending
                        color = palette[colorIndex1].lerp8(palette[colorIndex2], (uint8_t)(blendAmount * 255));
                    } else {
                        // Use discrete colors without blending
                        color = palette[colorIndex1];
                    }
                    
                    int ledPos = reverse ? (totalLeds - 1 - i) : i;
                    leds[startIndex + ledPos] = color;
                }
            } else {
                // Clear all LEDs for this pin first
                for (int i = 0; i < totalLeds; i++) {
                    leds[startIndex + i] = CRGB::Black;
                }
                
                if (loop) {
                    // Loop mode: fill the entire strip with repeating pattern
                    int patternLength = (paletteSize * span) + (paletteSize * separation);
                    
                    for (int i = 0; i < totalLeds; i++) {
                        int patternPos = (i + currentPosition[pin]) % patternLength;
                        int colorIndex = patternPos / (span + separation);
                        int posInColor = patternPos % (span + separation);
                        
                        if (posInColor < span) {
                            CRGB color;
                            if (blend && span > 1) {
                                // Blend within each span
                                float spanProgress = (float)posInColor / (span - 1);
                                int nextColorIndex = (colorIndex + 1) % paletteSize;
                                color = palette[colorIndex % paletteSize].lerp8(palette[nextColorIndex], (uint8_t)(spanProgress * 255));
                            } else {
                                color = palette[colorIndex % paletteSize];
                            }
                            
                            int ledPos = reverse ? (totalLeds - 1 - i) : i;
                            leds[startIndex + ledPos] = color;
                        }
                        // Separation areas remain black (already cleared above)
                    }
                } else {
                    // Single cycle mode: show each color once per cycle
                    for (int colorIndex = 0; colorIndex < paletteSize; colorIndex++) {
                        
                        // Calculate the starting position for this color
                        int colorStartPos = colorIndex * (span + separation);
                        
                        // Draw the span for this color
                        for (int spanIndex = 0; spanIndex < span; spanIndex++) {
                            int ledPos = (currentPosition[pin] + colorStartPos + spanIndex) % totalLeds;
                            
                            if (reverse) {
                                ledPos = totalLeds - 1 - ledPos;
                            }
                            
                            CRGB color;
                            if (blend && span > 1) {
                                // Blend within each color span for smooth transitions
                                float spanProgress = (float)spanIndex / (span - 1);
                                int nextColorIndex = (colorIndex + 1) % paletteSize;
                                color = palette[colorIndex].lerp8(palette[nextColorIndex], (uint8_t)(spanProgress * 255));
                            } else {
                                color = palette[colorIndex];
                            }
                            
                            leds[startIndex + ledPos] = color;
                        }
                    }
                }
            }
            
            // Move to next position (always update to keep patterns in sync)
            currentPosition[pin] = (currentPosition[pin] + 1) % totalLeds;
        }
        
        FastLED.show();
    }
}