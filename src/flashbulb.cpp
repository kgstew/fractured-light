#include "patterns.h"
#include <Arduino.h>
#include <FastLED.h>

enum FlashbulbState { FLASHBULB_FLASH, FLASHBULB_FADE_TO_BLACK, FLASHBULB_TRANSITION_BACK, FLASHBULB_COMPLETE };

struct FlashbulbData {
    FlashbulbState state;
    unsigned long startTime;
    unsigned long phaseStartTime;
    unsigned long flashDuration;
    unsigned long fadeDuration;
    unsigned long transitionDuration;
    CRGB* savedColors;
    int numSavedColors;
};

static FlashbulbData flashbulbData[8];
static bool flashbulbInitialized = false;
static bool flashbulbCompleted = false;

void resetFlashbulbPattern()
{
    for (int i = 0; i < 8; i++) {
        flashbulbData[i].state = FLASHBULB_COMPLETE;
        flashbulbData[i].startTime = 0;
        flashbulbData[i].phaseStartTime = 0;
        if (flashbulbData[i].savedColors != nullptr) {
            delete[] flashbulbData[i].savedColors;
            flashbulbData[i].savedColors = nullptr;
        }
        flashbulbData[i].numSavedColors = 0;
    }
    flashbulbInitialized = false;
    flashbulbCompleted = false;
}

void flashbulbPattern(
    int pins[], int numPins, int flashDuration, int fadeDuration, int transitionDuration, bool reverse)
{
    if (numPins == 0)
        return;

    unsigned long currentTime = millis();

    // Initialize on first call
    if (!flashbulbInitialized) {
        Serial.println("Flashbulb: Initializing");
        for (int p = 0; p < numPins; p++) {
            int pin = pins[p];
            if (pin >= 8)
                continue;

            FlashbulbData& data = flashbulbData[pin];
            data.state = FLASHBULB_FLASH;
            data.startTime = currentTime;
            data.phaseStartTime = currentTime;
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

            // Set to white immediately (only affected pins)
            for (int i = 0; i < totalLeds; i++) {
                int ledIndex = reverse ? (startIndex + totalLeds - 1 - i) : (startIndex + i);
                leds[ledIndex] = CRGB::White;
            }

            Serial.printf("Flashbulb: Pin %d initialized - Flash:%dms Fade:%dms Transition:%dms\n", pin, flashDuration,
                fadeDuration, transitionDuration);
        }
        flashbulbInitialized = true;
        // Don't call FastLED.show() here - let the main program handle it
        return;
    }

    // Update each pin's flashbulb state
    bool allComplete = true;

    for (int p = 0; p < numPins; p++) {
        int pin = pins[p];
        if (pin >= 8)
            continue;

        FlashbulbData& data = flashbulbData[pin];
        if (data.state == FLASHBULB_COMPLETE)
            continue;

        allComplete = false;
        unsigned long elapsed = currentTime - data.phaseStartTime;
        int startIndex = pin * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
        int totalLeds = NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;

        switch (data.state) {
        case FLASHBULB_FLASH:
            if (elapsed >= data.flashDuration) {
                // Start fade to black
                data.state = FLASHBULB_FADE_TO_BLACK;
                data.phaseStartTime = currentTime;
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
                data.phaseStartTime = currentTime;

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
                // Transition complete - don't restore colors, let main pattern take over
                data.state = FLASHBULB_COMPLETE;
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

    // Return whether all pins have completed their flashbulb sequence
    // This will be used by the interrupt system to know when to resume
    if (allComplete) {
        flashbulbCompleted = true;
    }

    // Show the LED changes
    FastLED.show();
}

bool isFlashbulbComplete() { return flashbulbCompleted; }

bool isPinUsedByFlashbulb(int pin)
{
    if (!flashbulbInitialized || pin >= 8) {
        return false;
    }
    return flashbulbData[pin].state != FLASHBULB_COMPLETE;
}