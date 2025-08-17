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

    // Create a program with 7 segments
    mainProgram = new Program(7);

    // Segment 1: Spin pattern test on all pins for 15 seconds
    int allPins[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    static CRGB spinPalette[] = { CRGB::Red, CRGB::Blue, CRGB::Green, CRGB::Yellow };
    PatternParams spinParams;
    spinParams.spin.speed = 75; // Medium-fast speed
    spinParams.spin.separation = 20; // 20 LEDs of black space between colors
    spinParams.spin.span = 15; // Each color fills 15 LEDs
    spinParams.spin.palette = spinPalette;
    spinParams.spin.paletteSize = 4;
    spinParams.spin.loop = true; // Fill entire strip with repeating pattern
    spinParams.spin.continuous = true; // Use span/separation pattern instead of all LEDs
    spinParams.spin.blend = true; // Smooth color transitions using FastLED lerp8
    mainProgram->addSegment(0, new Segment(PATTERN_SPIN, allPins, 8, 15, spinParams));

    // Segment 2: Multi-color breathing on all pins for 10 seconds
    static CRGB breathingPalette[] = { CRGB::Purple, CRGB::Magenta, CRGB::Blue, CRGB::Cyan };
    PatternParams breathingParams;
    breathingParams.breathing.speed = 50;
    breathingParams.breathing.palette = breathingPalette;
    breathingParams.breathing.paletteSize = 4;
    mainProgram->addSegment(1, new Segment(PATTERN_BREATHING, allPins, 8, 10, breathingParams));

    // Segment 3: Flame pattern on all pins for 15 seconds
    PatternParams flameParams;
    flameParams.flame.speed = 80;
    flameParams.flame.cooling = 55;
    flameParams.flame.sparking = 120;
    mainProgram->addSegment(2, new Segment(PATTERN_FLAME, allPins, 8, 10, flameParams, 1));

    // Segment 4: Grow pattern on all pins for 20 seconds
    static CRGB growPalette[] = { CRGB::Cyan, CRGB::Blue, CRGB::Purple, CRGB::Magenta, CRGB::Red, CRGB::Orange };
    PatternParams growParams;
    growParams.grow.speed = 60;
    growParams.grow.n = 1;
    growParams.grow.fadeDelay = 100;
    growParams.grow.holdDelay = 2000;
    growParams.grow.palette = growPalette;
    growParams.grow.paletteSize = 6;
    growParams.grow.transitionSpeed = 40;
    growParams.grow.offsetDelay = 1000;
    mainProgram->addSegment(3, new Segment(PATTERN_GROW, allPins, 8, 10, growParams, 1));

    // Segment 5: Multi-pattern segment - different patterns on different pins
    // Create pattern instances for different pin groups
    PatternInstance* patterns[3];

    // Breathing on pins 0-2
    int breathingPins[] = { 0, 1, 2 };
    static CRGB multiBreathingPalette[] = { CRGB(0, 255, 128), CRGB::Green, CRGB::Teal };
    PatternParams multiBreathingParams;
    multiBreathingParams.breathing.speed = 60;
    multiBreathingParams.breathing.palette = multiBreathingPalette;
    multiBreathingParams.breathing.paletteSize = 3;
    patterns[0] = new PatternInstance(PATTERN_BREATHING, breathingPins, 3, multiBreathingParams);

    // Flame on pins 3-5
    int flamePins[] = { 3, 4, 5 };
    PatternParams multiFlameParams;
    multiFlameParams.flame.speed = 90;
    multiFlameParams.flame.cooling = 60;
    multiFlameParams.flame.sparking = 130;
    patterns[1] = new PatternInstance(PATTERN_FLAME, flamePins, 3, multiFlameParams);

    int growPins[] = { 6, 7 };
    PatternParams growParams2;
    growParams2.grow.speed = 60;
    growParams2.grow.n = 1;
    growParams2.grow.fadeDelay = 100;
    growParams2.grow.holdDelay = 2000;
    growParams2.grow.palette = growPalette;
    growParams2.grow.paletteSize = 6;
    growParams2.grow.transitionSpeed = 40;
    growParams2.grow.offsetDelay = 1000;
    patterns[2] = new PatternInstance(PATTERN_GROW, growPins, 2, growParams2);

    mainProgram->addSegment(4, new Segment(patterns, 3, 5));

    // Segment 6: Pop pattern with random pins and acceleration
    static CRGB popPalette[]
        = { CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green, CRGB::Blue, CRGB::Purple, CRGB::Pink, CRGB::White };
    PatternParams popParams;
    popParams.pop.speed = 10; // Maximum speed after acceleration
    popParams.pop.holdDelay = 300; // Hold each color for 300ms
    popParams.pop.palette = popPalette;
    popParams.pop.paletteSize = 8;
    popParams.pop.random = true; // Randomize pin order
    popParams.pop.accelerationTime = 8; // Accelerate over 8 seconds
    mainProgram->addSegment(5, new Segment(PATTERN_POP, allPins, 8, 20, popParams));

    // Segment 7: Complex multi-pattern symphony - showcase of all features
    PatternInstance* symphonyPatterns[4];

    // Pattern 1: Pulsing ocean colors on pins 0-1 with smooth breathing
    int oceanPins[] = { 0, 1 };
    static CRGB oceanPalette[] = { 
        CRGB(0, 100, 150),    // Deep blue
        CRGB(0, 150, 200),    // Ocean blue  
        CRGB(0, 200, 255),    // Bright cyan
        CRGB(100, 255, 200),  // Aqua green
        CRGB(0, 255, 255)     // Pure cyan
    };
    PatternParams oceanParams;
    oceanParams.breathing.speed = 25;
    oceanParams.breathing.palette = oceanPalette;
    oceanParams.breathing.paletteSize = 5;
    symphonyPatterns[0] = new PatternInstance(PATTERN_BREATHING, oceanPins, 2, oceanParams);

    // Pattern 2: Rapid spinning rainbow on pins 2-3 with blending
    int rainbowPins[] = { 2, 3 };
    static CRGB rainbowPalette[] = { 
        CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green, 
        CRGB::Blue, CRGB::Indigo, CRGB::Violet, CRGB::Magenta 
    };
    PatternParams rainbowParams;
    rainbowParams.spin.speed = 90;
    rainbowParams.spin.separation = 8;
    rainbowParams.spin.span = 12;
    rainbowParams.spin.palette = rainbowPalette;
    rainbowParams.spin.paletteSize = 8;
    rainbowParams.spin.loop = true;
    rainbowParams.spin.continuous = false;
    rainbowParams.spin.blend = true;
    symphonyPatterns[1] = new PatternInstance(PATTERN_SPIN, rainbowPins, 2, rainbowParams);

    // Pattern 3: Growing sunset on pins 4-5 with staggered timing
    int sunsetPins[] = { 4, 5 };
    static CRGB sunsetPalette[] = { 
        CRGB(255, 40, 0),     // Deep red
        CRGB(255, 100, 0),    // Orange-red
        CRGB(255, 150, 0),    // Orange
        CRGB(255, 200, 50),   // Yellow-orange
        CRGB(255, 255, 100)   // Warm yellow
    };
    PatternParams sunsetParams;
    sunsetParams.grow.speed = 45;
    sunsetParams.grow.n = 3;
    sunsetParams.grow.fadeDelay = 150;
    sunsetParams.grow.holdDelay = 3000;
    sunsetParams.grow.palette = sunsetPalette;
    sunsetParams.grow.paletteSize = 5;
    sunsetParams.grow.transitionSpeed = 30;
    sunsetParams.grow.offsetDelay = 2000;
    symphonyPatterns[2] = new PatternInstance(PATTERN_GROW, sunsetPins, 2, sunsetParams);

    // Pattern 4: Accelerating neon flash on pins 6-7
    int neonPins[] = { 6, 7 };
    static CRGB neonPalette[] = { 
        CRGB(255, 0, 255),    // Magenta
        CRGB(0, 255, 255),    // Cyan
        CRGB(255, 255, 0),    // Yellow
        CRGB(255, 0, 128),    // Hot pink
        CRGB(128, 255, 0),    // Lime green
        CRGB(255, 128, 0)     // Neon orange
    };
    PatternParams neonParams;
    neonParams.pop.speed = 80;
    neonParams.pop.holdDelay = 200;
    neonParams.pop.palette = neonPalette;
    neonParams.pop.paletteSize = 6;
    neonParams.pop.random = true;
    neonParams.pop.accelerationTime = 15;
    symphonyPatterns[3] = new PatternInstance(PATTERN_POP, neonPins, 2, neonParams);

    mainProgram->addSegment(6, new Segment(symphonyPatterns, 4, 25));

    mainProgram->start();
}

void loop() { mainProgram->update(); }