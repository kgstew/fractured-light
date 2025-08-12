#include "patterns.h"
#include <Arduino.h>

void breathingPattern(int pins[], int numPins, int speed, CRGB color)
{
    if (speed == 0)
        return;

    static unsigned long lastUpdate = 0;
    static float brightness = 0.0;
    static bool increasing = true;

    unsigned long currentTime = millis();
    unsigned long interval = map(speed, 1, 100, 100, 5);

    if (currentTime - lastUpdate >= interval) {
        lastUpdate = currentTime;

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

        CRGB scaledColor = color;
        scaledColor.nscale8((uint8_t)brightness);

        for (int p = 0; p < numPins; p++) {
            int pin = pins[p];
            int startIndex = pin * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            int endIndex = startIndex + (NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN);

            for (int i = startIndex; i < endIndex; i++) {
                leds[i] = scaledColor;
            }
        }

        FastLED.show();
    }
}