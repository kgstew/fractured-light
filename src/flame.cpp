#include "patterns.h"
#include <Arduino.h>

void flamepattern(int pins[], int numPins, int speed, int cooling, int sparking)
{
    if (speed == 0)
        return;

    static unsigned long lastUpdate = 0;
    static uint8_t heat[NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN];

    unsigned long currentTime = millis();
    unsigned long interval = map(speed, 1, 100, 100, 10);

    if (currentTime - lastUpdate >= interval) {
        lastUpdate = currentTime;

        for (int p = 0; p < numPins; p++) {
            int pin = pins[p];
            int startIndex = pin * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            int ledsPerPin = NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;

            // Step 1: Cool down every cell a little
            for (int i = 0; i < ledsPerPin; i++) {
                heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / ledsPerPin) + 2));
            }

            // Step 2: Heat from each cell drifts 'up' and diffuses a little
            for (int k = ledsPerPin - 1; k >= 2; k--) {
                heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
            }

            // Step 3: Randomly ignite new 'sparks' of heat near the bottom
            if (random8() < sparking) {
                int y = random8(7);
                heat[y] = qadd8(heat[y], random8(160, 255));
            }

            // Step 4: Map from heat cells to LED colors using HeatColor palette
            for (int j = 0; j < ledsPerPin; j++) {
                // Scale heat value to palette index
                uint8_t colorindex = scale8(heat[j], 240);
                CRGB color = HeatColor(colorindex);

                leds[startIndex + j] = color;
            }
        }

        FastLED.show();
    }
}