#include "program.h"
#include "patterns.h"
#include <Arduino.h>

PatternInstance::PatternInstance(PatternType type, int* pinArray, int pinCount, PatternParams parameters, bool reverseDirection) {
    patternType = type;
    pins = new int[pinCount];
    for (int i = 0; i < pinCount; i++) {
        pins[i] = pinArray[i];
    }
    numPins = pinCount;
    params = parameters;
    reverse = reverseDirection;
}

PatternInstance::~PatternInstance() {
    delete[] pins;
}

Segment::Segment(PatternType type, int* pinArray, int pinCount, unsigned long durationSeconds, PatternParams parameters, bool reverseDirection) {
    patterns = new PatternInstance*[1];
    patterns[0] = new PatternInstance(type, pinArray, pinCount, parameters, reverseDirection);
    numPatterns = 1;
    duration = durationSeconds * 1000;
    startTime = 0;
    isActive = false;
}

Segment::Segment(PatternInstance** patternArray, int patternCount, unsigned long durationSeconds) {
    patterns = new PatternInstance*[patternCount];
    for (int i = 0; i < patternCount; i++) {
        patterns[i] = patternArray[i];
    }
    numPatterns = patternCount;
    duration = durationSeconds * 1000;
    startTime = 0;
    isActive = false;
}

Segment::~Segment() {
    for (int i = 0; i < numPatterns; i++) {
        delete patterns[i];
    }
    delete[] patterns;
}

void Segment::start() {
    startTime = millis();
    isActive = true;
    
    // Reset pattern state for all patterns when starting
    for (int i = 0; i < numPatterns; i++) {
        switch (patterns[i]->patternType) {
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
}

void Segment::stop() {
    isActive = false;
    
    // Clear all LEDs for all patterns' pins when stopping
    for (int patternIdx = 0; patternIdx < numPatterns; patternIdx++) {
        PatternInstance* pattern = patterns[patternIdx];
        for (int p = 0; p < pattern->numPins; p++) {
            int pin = pattern->pins[p];
            int startIndex = pin * NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            int totalLeds = NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_PIN;
            
            for (int i = 0; i < totalLeds; i++) {
                leds[startIndex + i] = CRGB::Black;
            }
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

    // Update all patterns in this segment
    for (int i = 0; i < numPatterns; i++) {
        PatternInstance* pattern = patterns[i];
        switch (pattern->patternType) {
            case PATTERN_BREATHING:
                breathingPattern(pattern->pins, pattern->numPins, pattern->params.breathing.speed, pattern->params.breathing.color, pattern->reverse);
                break;
            case PATTERN_FLAME:
                flamepattern(pattern->pins, pattern->numPins, pattern->params.flame.speed, pattern->params.flame.cooling, pattern->params.flame.sparking, pattern->reverse);
                break;
            case PATTERN_GROW:
                growPattern(pattern->pins, pattern->numPins, pattern->params.grow.speed, pattern->params.grow.n, pattern->params.grow.fadeDelay, pattern->params.grow.holdDelay, pattern->params.grow.palette, pattern->params.grow.paletteSize, pattern->params.grow.transitionSpeed, pattern->params.grow.offsetDelay, pattern->reverse);
                break;
            case PATTERN_CHASE:
                chasePattern(pattern->pins, pattern->numPins, pattern->params.chase.speed, pattern->params.chase.palette, pattern->params.chase.paletteSize, pattern->params.chase.transitionSpeed, pattern->params.chase.holdDelay, pattern->params.chase.offsetDelay, pattern->reverse);
                break;
        }
    }
}

void Segment::addPattern(PatternInstance* pattern) {
    // Note: This is a simple implementation that doesn't resize the array
    // In practice, you might want to implement dynamic resizing or use a vector-like approach
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