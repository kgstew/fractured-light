#include "patterns.h"
#include <Arduino.h>
#include <FastLED.h>

static unsigned long lastUpdateTime = 0;
static int currentPin = 0;
static int currentColorIndex = 0;
static bool pinFilled = false;
static unsigned long fillStartTime = 0;

void resetPopPattern() {
    lastUpdateTime = 0;
    currentPin = 0;
    currentColorIndex = 0;
    pinFilled = false;
    fillStartTime = 0;
}

void popPattern(int pins[], int numPins, int speed, int holdDelay, CRGB palette[], int paletteSize, bool reverse) {
    if (numPins == 0 || paletteSize == 0) return;
    
    unsigned long currentTime = millis();
    
    // Calculate delay between updates based on speed
    unsigned long updateDelay = map(speed, 0, 100, 200, 10);
    
    if (currentTime - lastUpdateTime >= updateDelay) {
        lastUpdateTime = currentTime;
        
        // If we haven't filled the current pin yet, fill it
        if (!pinFilled) {
            int pin = reverse ? pins[numPins - 1 - currentPin] : pins[currentPin];
            int startIndex = pin * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            int totalLeds = NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            
            CRGB color = palette[currentColorIndex % paletteSize];
            
            // Fill all LEDs on this pin with the current color
            for (int i = 0; i < totalLeds; i++) {
                leds[startIndex + i] = color;
            }
            
            pinFilled = true;
            fillStartTime = currentTime;
        }
        // If we've filled the pin and enough time has passed, move to next pin
        else if (currentTime - fillStartTime >= holdDelay) {
            // Turn off current pin
            int pin = reverse ? pins[numPins - 1 - currentPin] : pins[currentPin];
            int startIndex = pin * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            int totalLeds = NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            
            for (int i = 0; i < totalLeds; i++) {
                leds[startIndex + i] = CRGB::Black;
            }
            
            // Move to next pin and color
            currentPin = (currentPin + 1) % numPins;
            currentColorIndex = (currentColorIndex + 1) % paletteSize;
            pinFilled = false;
        }
        
        FastLED.show();
    }
}