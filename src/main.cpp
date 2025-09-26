#include "patterns.h"
#include "program.h"
#include <Arduino.h>
#include <FastLED.h>

#define NUM_PINS 8
#define COLOR_ORDER RGB

#define PIN1 23
#define PIN2 18
#define PIN3 19
#define PIN4 5
#define PIN5 13
#define PIN6 14
#define PIN7 15
#define PIN8 12

CRGB leds1[MAX_LEDS_PER_PIN];
CRGB leds2[MAX_LEDS_PER_PIN];
CRGB leds3[MAX_LEDS_PER_PIN];
CRGB leds4[MAX_LEDS_PER_PIN];
CRGB leds5[MAX_LEDS_PER_PIN];
CRGB leds6[MAX_LEDS_PER_PIN];
CRGB leds7[MAX_LEDS_PER_PIN];
CRGB leds8[MAX_LEDS_PER_PIN];
Program* mainProgram;

// Helper function to get LED array for a specific pin (0-indexed)
CRGB* getLedArrayForPin(int pin)
{
    switch (pin) {
    case 0:
        return leds1;
    case 1:
        return leds2;
    case 2:
        return leds3;
    case 3:
        return leds4;
    case 4:
        return leds5;
    case 5:
        return leds6;
    case 6:
        return leds7;
    case 7:
        return leds8;
    default:
        return nullptr;
    }
}

void setup()
{
    Serial.begin(115200);

    // Configure FastLED for each pin independently (up to 122 LEDs per pin)
    FastLED.addLeds<WS2812B, PIN1, COLOR_ORDER>(leds1, MAX_LEDS_PER_PIN);
    FastLED.addLeds<WS2812B, PIN2, COLOR_ORDER>(leds2, MAX_LEDS_PER_PIN);
    FastLED.addLeds<WS2812B, PIN3, COLOR_ORDER>(leds3, MAX_LEDS_PER_PIN);
    FastLED.addLeds<WS2812B, PIN4, COLOR_ORDER>(leds4, MAX_LEDS_PER_PIN);
    FastLED.addLeds<WS2812B, PIN5, COLOR_ORDER>(leds5, MAX_LEDS_PER_PIN);
    FastLED.addLeds<WS2812B, PIN6, COLOR_ORDER>(leds6, MAX_LEDS_PER_PIN);
    FastLED.addLeds<WS2812B, PIN7, COLOR_ORDER>(leds7, MAX_LEDS_PER_PIN);
    FastLED.addLeds<WS2812B, PIN8, COLOR_ORDER>(leds8, MAX_LEDS_PER_PIN);

    FastLED.setBrightness(255);
    FastLED.clear();
    FastLED.show();

    Serial.println(
        "FastLED initialized for " + String(NUM_PINS) + " pins, up to " + String(MAX_LEDS_PER_PIN) + " LEDs each");

    // Create a program with multiple segments
    mainProgram = new Program(1);

    // All pins array
    int allPins[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    // Segment 1: Multi-color breathing on all pins for 10 seconds
    static CRGB breathingPalette[] = { CRGB::Purple, CRGB::Magenta, CRGB::Blue, CRGB::Cyan };
    PatternParams breathingParams;
    breathingParams.breathing.speed = 50;
    breathingParams.breathing.palette = breathingPalette;
    breathingParams.breathing.paletteSize = 4;
    mainProgram->addSegment(0, new Segment(PATTERN_BREATHING, allPins, 8, 10, breathingParams));

    mainProgram->start();
}

void loop()
{
    mainProgram->update();
}