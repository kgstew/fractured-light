#include "patterns.h"
#include <Arduino.h>

void flamepattern(int pins[], int numPins, int speed, int cooling, int sparking, bool reverse)
{
    if (speed == 0)
        return;

    static unsigned long lastUpdate[8] = { 0 };
    static uint8_t heat[8][NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN];

    unsigned long currentTime = millis();

    for (int p = 0; p < numPins; p++) {
        int pin = pins[p];

        // Add random time offset per pin (0-30ms)
        unsigned long interval = map(speed, 1, 100, 100, 10) + random8(0, 31);

        if (currentTime - lastUpdate[pin] >= interval) {
            lastUpdate[pin] = currentTime;

            int startIndex = pin * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            int ledsPerPin = NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;

            // Step 1: Cool down every cell with slight random variation
            uint8_t pinCooling = cooling + random8(0, 11) - 5; // ±5 variation
            for (int i = 0; i < ledsPerPin; i++) {
                heat[pin][i] = qsub8(heat[pin][i], random8(0, ((pinCooling * 10) / ledsPerPin) + 2));
            }

            // Step 2: Heat from each cell drifts 'up' and diffuses a little
            for (int k = ledsPerPin - 1; k >= 2; k--) {
                heat[pin][k] = (heat[pin][k - 1] + heat[pin][k - 2] + heat[pin][k - 2]) / 3;
            }

            // Step 3: Randomly ignite new 'sparks' with slight random variation
            uint8_t pinSparking = sparking + random8(0, 21) - 10; // ±10 variation
            if (random8() < pinSparking) {
                int y = random8(7);
                heat[pin][y] = qadd8(heat[pin][y], random8(160, 255));
            }

            // Add extra heat to bottom eighth for more yellow
            int bottomEighth = ledsPerPin / 12;
            for (int i = 0; i < bottomEighth; i++) {
                if (random8() < 80) { // 80/255 chance
                    heat[pin][i] = qadd8(heat[pin][i], random8(30, 80));
                }
            }

            // Step 4: Map from heat cells to LED colors using HeatColor palette
            for (int j = 0; j < ledsPerPin; j++) {
                // Scale heat value to palette index
                uint8_t colorindex = scale8(heat[pin][j], 240);
                CRGB color = HeatColor(colorindex);

                if (reverse) {
                    leds[startIndex + (ledsPerPin - 1 - j)] = color;
                } else {
                    leds[startIndex + j] = color;
                }
            }
        }
    }

    FastLED.show();
}