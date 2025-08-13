#include "program.h"
#include "patterns.h"
#include <Arduino.h>

Segment::Segment(PatternType type, int* pinArray, int pinCount, unsigned long durationSeconds, PatternParams parameters, bool reverseDirection) {
    patternType = type;
    pins = new int[pinCount];
    for (int i = 0; i < pinCount; i++) {
        pins[i] = pinArray[i];
    }
    numPins = pinCount;
    duration = durationSeconds * 1000;
    params = parameters;
    startTime = 0;
    isActive = false;
    reverse = reverseDirection;
}

Segment::~Segment() {
    delete[] pins;
}

void Segment::start() {
    startTime = millis();
    isActive = true;
    
    // Reset pattern state when starting
    switch (patternType) {
        case PATTERN_BREATHING:
            resetBreathingPattern();
            break;
        case PATTERN_FLAME:
            resetFlamePattern();
            break;
        case PATTERN_GROW:
            resetGrowPattern();
            break;
        case PATTERN_CHASE:
            resetChasePattern();
            break;
    }
}

void Segment::stop() {
    isActive = false;
    
    // Clear all LEDs for this segment's pins when stopping
    for (int p = 0; p < numPins; p++) {
        int pin = pins[p];
        int startIndex = pin * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
        int totalLeds = NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
        
        for (int i = 0; i < totalLeds; i++) {
            leds[startIndex + i] = CRGB::Black;
        }
    }
    
    FastLED.show();
}

bool Segment::isFinished() {
    if (!isActive) return false;
    return (millis() - startTime) >= duration;
}

void Segment::update() {
    if (!isActive) return;

    switch (patternType) {
        case PATTERN_BREATHING:
            breathingPattern(pins, numPins, params.breathing.speed, params.breathing.color, reverse);
            break;
        case PATTERN_FLAME:
            flamepattern(pins, numPins, params.flame.speed, params.flame.cooling, params.flame.sparking, reverse);
            break;
        case PATTERN_GROW:
            growPattern(pins, numPins, params.grow.speed, params.grow.n, params.grow.fadeDelay, params.grow.holdDelay, params.grow.palette, params.grow.paletteSize, params.grow.transitionSpeed, params.grow.offsetDelay, reverse);
            break;
        case PATTERN_CHASE:
            chasePattern(pins, numPins, params.chase.speed, params.chase.palette, params.chase.paletteSize, params.chase.transitionSpeed, params.chase.holdDelay, params.chase.offsetDelay, reverse);
            break;
    }
}

Program::Program(int segmentCount) {
    segments = new Segment*[segmentCount];
    numSegments = segmentCount;
    currentSegment = 0;
    isRunning = false;
    
    for (int i = 0; i < numSegments; i++) {
        segments[i] = nullptr;
    }
}

Program::~Program() {
    for (int i = 0; i < numSegments; i++) {
        delete segments[i];
    }
    delete[] segments;
}

void Program::addSegment(int index, Segment* segment) {
    if (index >= 0 && index < numSegments) {
        segments[index] = segment;
    }
}

void Program::start() {
    if (numSegments > 0 && segments[0] != nullptr) {
        currentSegment = 0;
        segments[currentSegment]->start();
        isRunning = true;
    }
}

void Program::stop() {
    if (isRunning && currentSegment < numSegments && segments[currentSegment] != nullptr) {
        segments[currentSegment]->stop();
    }
    isRunning = false;
}

void Program::update() {
    if (!isRunning || currentSegment >= numSegments || segments[currentSegment] == nullptr) {
        return;
    }

    segments[currentSegment]->update();

    if (segments[currentSegment]->isFinished()) {
        segments[currentSegment]->stop();
        currentSegment++;

        if (currentSegment >= numSegments) {
            currentSegment = 0;
            segments[currentSegment]->start();
        } else {
            segments[currentSegment]->start();
        }
    }
}

bool Program::getIsRunning() {
    return isRunning;
}