#include "patterns.h"
#include "program.h"
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
Program* mainProgram;

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

    // Create a program with 3 segments
    mainProgram = new Program(3);

    // Segment 1: Purple breathing on all pins for 10 seconds
    int allPins[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    PatternParams breathingParams;
    breathingParams.breathing.speed = 30;
    breathingParams.breathing.color = CRGB(128, 0, 128);
    mainProgram->addSegment(0, new Segment(PATTERN_BREATHING, allPins, 8, 10, breathingParams));

    // Segment 2: Flame pattern on all pins for 15 seconds
    PatternParams flameParams;
    flameParams.flame.speed = 80;
    flameParams.flame.cooling = 55;
    flameParams.flame.sparking = 120;
    mainProgram->addSegment(1, new Segment(PATTERN_FLAME, allPins, 8, 10, flameParams, 1));

    // Segment 3: Grow pattern on all pins for 12 seconds
    PatternParams growParams;
    growParams.grow.speed = 60;
    growParams.grow.n = 5;
    growParams.grow.fadeDelay = 100;
    growParams.grow.holdDelay = 2000;
    growParams.grow.color = CRGB(0, 255, 128);
    mainProgram->addSegment(2, new Segment(PATTERN_GROW, allPins, 8, 12, growParams, 1));

    mainProgram->start();
}

void loop() { mainProgram->update(); }

/**
 *
 * Pulsing different colors slowly in a breathing pattern.
 *
 * Purple and greens and teals
 * Fire effect
 *
 * Spirals around the column like a tornado
 * All 8 in sync and then breaking apart
 *
 */