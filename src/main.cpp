#include "patterns.h"
#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS_PER_STRIP 122
#define NUM_STRIPS_PER_PIN 2
#define NUM_PINS 8
#define TOTAL_LEDS (NUM_PINS * NUM_STRIPS_PER_PIN * NUM_LEDS_PER_STRIP)
#define COLOR_ORDER GRB

#define PIN1 13
#define PIN2 12
#define PIN3 14
#define PIN4 27
#define PIN5 26
#define PIN6 25
#define PIN7 33
#define PIN8 32

CRGB leds[TOTAL_LEDS];

void setup()
{
    Serial.begin(115200);

    // Configure FastLED for 8 pins, each controlling 2 strips of 122 LEDs
    FastLED.addLeds<WS2812B, PIN1, COLOR_ORDER>(
        leds, 0 * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN);
    FastLED.addLeds<WS2812B, PIN2, COLOR_ORDER>(
        leds, 1 * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN);
    FastLED.addLeds<WS2812B, PIN3, COLOR_ORDER>(
        leds, 2 * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN);
    FastLED.addLeds<WS2812B, PIN4, COLOR_ORDER>(
        leds, 3 * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN);
    FastLED.addLeds<WS2812B, PIN5, COLOR_ORDER>(
        leds, 4 * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN);
    FastLED.addLeds<WS2812B, PIN6, COLOR_ORDER>(
        leds, 5 * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN);
    FastLED.addLeds<WS2812B, PIN7, COLOR_ORDER>(
        leds, 6 * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN);
    FastLED.addLeds<WS2812B, PIN8, COLOR_ORDER>(
        leds, 7 * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN);

    FastLED.setBrightness(255);
    FastLED.clear();
    FastLED.show();
}

void loop()
{
    int pins[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    flamepattern(pins, 8, 50, 55, 120);
}