#include "patterns.h"
#include <Arduino.h>
#include <FastLED.h>

static unsigned long lastUpdateTime = 0;
static int currentPin = 0;
static int currentColorIndex = 0;
static bool pinFilled = false;
static unsigned long fillStartTime = 0;
static unsigned long patternStartTime = 0;
static int* pinSequence = nullptr;
static int sequenceLength = 0;

void resetPopPattern() {
    lastUpdateTime = 0;
    currentPin = 0;
    currentColorIndex = 0;
    pinFilled = false;
    fillStartTime = 0;
    patternStartTime = 0;
    if (pinSequence != nullptr) {
        delete[] pinSequence;
        pinSequence = nullptr;
    }
    sequenceLength = 0;
}

void popPattern(int pins[], int numPins, int speed, int holdDelay, CRGB palette[], int paletteSize, bool random, int accelerationTime, bool reverse) {
    if (numPins == 0 || paletteSize == 0) return;
    
    unsigned long currentTime = millis();
    
    // Initialize pattern start time and pin sequence on first call
    if (patternStartTime == 0) {
        patternStartTime = currentTime;
        
        // Create pin sequence based on random parameter
        if (pinSequence == nullptr) {
            sequenceLength = numPins;
            pinSequence = new int[sequenceLength];
            
            if (random) {
                // Create randomized pin sequence
                for (int i = 0; i < numPins; i++) {
                    pinSequence[i] = pins[i];
                }
                // Fisher-Yates shuffle algorithm
                for (int i = numPins - 1; i > 0; i--) {
                    int j = rand() % (i + 1);
                    int temp = pinSequence[i];
                    pinSequence[i] = pinSequence[j];
                    pinSequence[j] = temp;
                }
            } else {
                // Create sequential pin order
                for (int i = 0; i < numPins; i++) {
                    pinSequence[i] = reverse ? pins[numPins - 1 - i] : pins[i];
                }
            }
        }
    }
    
    // Calculate current speed based on acceleration
    int currentSpeed = speed;
    if (accelerationTime > 0) {
        unsigned long elapsed = currentTime - patternStartTime;
        unsigned long accelTimeMs = accelerationTime * 1000;
        
        if (elapsed < accelTimeMs) {
            // Linear acceleration from initial speed (lower value) to max speed (higher value)
            // Note: lower speed value = slower, higher speed value = faster
            int minSpeed = 10;  // Starting speed (slow)
            currentSpeed = minSpeed + ((speed - minSpeed) * elapsed) / accelTimeMs;
        }
    }
    
    // Calculate delay between updates based on current speed
    unsigned long updateDelay = map(currentSpeed, 0, 100, 200, 10);
    
    if (currentTime - lastUpdateTime >= updateDelay) {
        lastUpdateTime = currentTime;
        
        // If we haven't filled the current pin yet, fill it
        if (!pinFilled) {
            int pin = pinSequence[currentPin];
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
            int pin = pinSequence[currentPin];
            int startIndex = pin * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            int totalLeds = NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            
            for (int i = 0; i < totalLeds; i++) {
                leds[startIndex + i] = CRGB::Black;
            }
            
            // Move to next pin and color
            currentPin = (currentPin + 1) % sequenceLength;
            currentColorIndex = (currentColorIndex + 1) % paletteSize;
            pinFilled = false;
            
            // If random mode and we've completed a full cycle, reshuffle
            if (random && currentPin == 0) {
                // Fisher-Yates shuffle algorithm
                for (int i = sequenceLength - 1; i > 0; i--) {
                    int j = rand() % (i + 1);
                    int temp = pinSequence[i];
                    pinSequence[i] = pinSequence[j];
                    pinSequence[j] = temp;
                }
            }
        }
        
        FastLED.show();
    }
}