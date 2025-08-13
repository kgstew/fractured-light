#include "patterns.h"
#include <Arduino.h>
#include <FastLED.h>

enum FlashbulbState {
    FLASHBULB_FLASH,
    FLASHBULB_FADE_TO_BLACK,
    FLASHBULB_TRANSITION_BACK,
    FLASHBULB_COMPLETE
};

struct FlashbulbData {
    FlashbulbState state;
    unsigned long startTime;
    unsigned long flashDuration;
    unsigned long fadeDuration;
    unsigned long transitionDuration;
    CRGB* savedColors;
    int numSavedColors;
};

static FlashbulbData flashbulbData[8];
static bool flashbulbInitialized = false;

void resetFlashbulbPattern()
{
    for (int i = 0; i < 8; i++) {
        flashbulbData[i].state = FLASHBULB_COMPLETE;
        flashbulbData[i].startTime = 0;
        if (flashbulbData[i].savedColors != nullptr) {
            delete[] flashbulbData[i].savedColors;
            flashbulbData[i].savedColors = nullptr;
        }
        flashbulbData[i].numSavedColors = 0;
    }
    flashbulbInitialized = false;
}

void flashbulbPattern(int pins[], int numPins, int flashDuration, int fadeDuration, int transitionDuration, bool reverse)
{
    if (numPins == 0) return;
    
    unsigned long currentTime = millis();
    
    // Initialize on first call
    if (!flashbulbInitialized) {
        for (int p = 0; p < numPins; p++) {
            int pin = pins[p];
            if (pin >= 8) continue;
            
            FlashbulbData& data = flashbulbData[pin];
            data.state = FLASHBULB_FLASH;
            data.startTime = currentTime;
            data.flashDuration = flashDuration;
            data.fadeDuration = fadeDuration;
            data.transitionDuration = transitionDuration;
            
            // Save current colors
            int startIndex = pin * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            int totalLeds = NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            
            data.savedColors = new CRGB[totalLeds];
            data.numSavedColors = totalLeds;
            
            for (int i = 0; i < totalLeds; i++) {
                data.savedColors[i] = leds[startIndex + i];
            }
            
            // Set to white immediately
            for (int i = 0; i < totalLeds; i++) {
                leds[startIndex + i] = CRGB::White;
            }
        }
        flashbulbInitialized = true;
        FastLED.show();
        return;
    }
    
    // Update each pin's flashbulb state
    bool allComplete = true;
    
    for (int p = 0; p < numPins; p++) {
        int pin = pins[p];
        if (pin >= 8) continue;
        
        FlashbulbData& data = flashbulbData[pin];
        if (data.state == FLASHBULB_COMPLETE) continue;
        
        allComplete = false;
        unsigned long elapsed = currentTime - data.startTime;
        int startIndex = pin * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
        int totalLeds = NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
        
        switch (data.state) {
            case FLASHBULB_FLASH:
                if (elapsed >= data.flashDuration) {
                    // Start fade to black
                    data.state = FLASHBULB_FADE_TO_BLACK;
                    data.startTime = currentTime;
                } else {
                    // Keep white during flash
                    for (int i = 0; i < totalLeds; i++) {
                        int ledIndex = reverse ? (startIndex + totalLeds - 1 - i) : (startIndex + i);
                        leds[ledIndex] = CRGB::White;
                    }
                }
                break;
                
            case FLASHBULB_FADE_TO_BLACK:
                if (elapsed >= data.fadeDuration) {
                    // Start transition back
                    data.state = FLASHBULB_TRANSITION_BACK;
                    data.startTime = currentTime;
                    
                    // Set to black
                    for (int i = 0; i < totalLeds; i++) {
                        int ledIndex = reverse ? (startIndex + totalLeds - 1 - i) : (startIndex + i);
                        leds[ledIndex] = CRGB::Black;
                    }
                } else {
                    // Fade from white to black
                    float fadeProgress = (float)elapsed / data.fadeDuration;
                    uint8_t brightness = (uint8_t)(255 * (1.0 - fadeProgress));
                    
                    for (int i = 0; i < totalLeds; i++) {
                        int ledIndex = reverse ? (startIndex + totalLeds - 1 - i) : (startIndex + i);
                        leds[ledIndex] = CRGB(brightness, brightness, brightness);
                    }
                }
                break;
                
            case FLASHBULB_TRANSITION_BACK:
                if (elapsed >= data.transitionDuration) {
                    // Transition complete
                    data.state = FLASHBULB_COMPLETE;
                    
                    // Restore saved colors
                    for (int i = 0; i < totalLeds; i++) {
                        int ledIndex = reverse ? (startIndex + totalLeds - 1 - i) : (startIndex + i);
                        leds[ledIndex] = data.savedColors[i];
                    }
                } else {
                    // Blend from black to saved colors
                    float transitionProgress = (float)elapsed / data.transitionDuration;
                    
                    for (int i = 0; i < totalLeds; i++) {
                        int ledIndex = reverse ? (startIndex + totalLeds - 1 - i) : (startIndex + i);
                        CRGB black = CRGB::Black;
                        leds[ledIndex] = black.lerp8(data.savedColors[i], (uint8_t)(transitionProgress * 255));
                    }
                }
                break;
                
            default:
                break;
        }
    }
    
    // If all pins are complete, reset for next use
    if (allComplete) {
        resetFlashbulbPattern();
    }
    
    FastLED.show();
}