#include "program.h"
#include "patterns.h"
#include <Arduino.h>

PatternInstance::PatternInstance(
    PatternType type, int* pinArray, int pinCount, PatternParams parameters, bool reverseDirection)
{
    patternType = type;
    pins = new int[pinCount];
    for (int i = 0; i < pinCount; i++) {
        pins[i] = pinArray[i];
    }
    numPins = pinCount;
    params = parameters;
    reverse = reverseDirection;
}

PatternInstance::~PatternInstance() { delete[] pins; }

InterruptSegment::InterruptSegment(PatternType type, int* pinArray, int pinCount, unsigned long durationMs, PatternParams parameters, bool reverseDirection)
    : Segment(type, pinArray, pinCount, 0, parameters, reverseDirection)
{
    interruptDuration = durationMs;
    interruptStartTime = 0;
}

InterruptSegment::InterruptSegment(PatternInstance** patternArray, int patternCount, unsigned long durationMs)
    : Segment(patternArray, patternCount, 0)
{
    interruptDuration = durationMs;
    interruptStartTime = 0;
}

bool InterruptSegment::isFinished()
{
    if (!isActive)
        return false;
    return (millis() - interruptStartTime) >= interruptDuration;
}

void InterruptSegment::start()
{
    Segment::start();
    interruptStartTime = millis();
}

Segment::Segment(PatternType type, int* pinArray, int pinCount, unsigned long durationSeconds, PatternParams parameters,
    bool reverseDirection)
{
    patterns = new PatternInstance*[1];
    patterns[0] = new PatternInstance(type, pinArray, pinCount, parameters, reverseDirection);
    numPatterns = 1;
    duration = durationSeconds * 1000;
    startTime = 0;
    isActive = false;
}

Segment::Segment(PatternInstance** patternArray, int patternCount, unsigned long durationSeconds)
{
    patterns = new PatternInstance*[patternCount];
    for (int i = 0; i < patternCount; i++) {
        patterns[i] = patternArray[i];
    }
    numPatterns = patternCount;
    duration = durationSeconds * 1000;
    startTime = 0;
    isActive = false;
}

Segment::~Segment()
{
    for (int i = 0; i < numPatterns; i++) {
        delete patterns[i];
    }
    delete[] patterns;
}

void Segment::start()
{
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
        case PATTERN_POP:
            resetPopPattern();
            break;
        case PATTERN_SPIN:
            resetSpinPattern();
            break;
        case PATTERN_FLASHBULB:
            resetFlashbulbPattern();
            break;
        }
    }
}

void Segment::stop()
{
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

bool Segment::isFinished()
{
    if (!isActive)
        return false;
    return (millis() - startTime) >= duration;
}

void Segment::update()
{
    if (!isActive)
        return;

    // Update all patterns in this segment
    for (int i = 0; i < numPatterns; i++) {
        PatternInstance* pattern = patterns[i];
        switch (pattern->patternType) {
        case PATTERN_BREATHING:
            breathingPattern(pattern->pins, pattern->numPins, pattern->params.breathing.speed,
                pattern->params.breathing.palette, pattern->params.breathing.paletteSize, pattern->reverse);
            break;
        case PATTERN_FLAME:
            flamepattern(pattern->pins, pattern->numPins, pattern->params.flame.speed, pattern->params.flame.cooling,
                pattern->params.flame.sparking, pattern->reverse);
            break;
        case PATTERN_GROW:
            growPattern(pattern->pins, pattern->numPins, pattern->params.grow.speed, pattern->params.grow.n,
                pattern->params.grow.fadeDelay, pattern->params.grow.holdDelay, pattern->params.grow.palette,
                pattern->params.grow.paletteSize, pattern->params.grow.transitionSpeed,
                pattern->params.grow.offsetDelay, pattern->reverse);
            break;
        case PATTERN_POP:
            popPattern(pattern->pins, pattern->numPins, pattern->params.pop.speed, pattern->params.pop.holdDelay,
                pattern->params.pop.palette, pattern->params.pop.paletteSize, pattern->params.pop.random, 
                pattern->params.pop.accelerationTime, pattern->reverse);
            break;
        case PATTERN_SPIN:
            spinPattern(pattern->pins, pattern->numPins, pattern->params.spin.speed, pattern->params.spin.separation,
                pattern->params.spin.span, pattern->params.spin.palette, pattern->params.spin.paletteSize, 
                pattern->params.spin.loop, pattern->params.spin.continuous, pattern->params.spin.blend, pattern->reverse);
            break;
        case PATTERN_FLASHBULB:
            flashbulbPattern(pattern->pins, pattern->numPins, pattern->params.flashbulb.flashDuration,
                pattern->params.flashbulb.fadeDuration, pattern->params.flashbulb.transitionDuration, pattern->reverse);
            break;
        }
    }
}

void Segment::addPattern(PatternInstance* pattern)
{
    // Note: This is a simple implementation that doesn't resize the array
    // In practice, you might want to implement dynamic resizing or use a vector-like approach
}

Program::Program(int segmentCount)
{
    segments = new Segment*[segmentCount];
    numSegments = segmentCount;
    currentSegment = 0;
    isRunning = false;
    
    interruptState = INTERRUPT_NONE;
    interruptSegment = nullptr;
    transitionDuration = 500;
    originalBrightness = 255;
    currentBrightness = 255;

    for (int i = 0; i < numSegments; i++) {
        segments[i] = nullptr;
    }
}

Program::~Program()
{
    for (int i = 0; i < numSegments; i++) {
        delete segments[i];
    }
    delete[] segments;
    
    if (interruptSegment != nullptr) {
        delete interruptSegment;
    }
}

void Program::addSegment(int index, Segment* segment)
{
    if (index >= 0 && index < numSegments) {
        segments[index] = segment;
    }
}

void Program::start()
{
    if (numSegments > 0 && segments[0] != nullptr) {
        currentSegment = 0;
        segments[currentSegment]->start();
        isRunning = true;
    }
}

void Program::stop()
{
    if (isRunning && currentSegment < numSegments && segments[currentSegment] != nullptr) {
        segments[currentSegment]->stop();
    }
    isRunning = false;
}

void Program::update()
{
    if (!isRunning) {
        return;
    }

    updateTransitions();

    if (interruptState == INTERRUPT_ACTIVE && interruptSegment != nullptr) {
        // For flashbulb patterns, also continue running the main pattern on unaffected pins
        bool isFlashbulb = false;
        if (interruptSegment->patterns != nullptr && interruptSegment->numPatterns > 0) {
            isFlashbulb = (interruptSegment->patterns[0]->patternType == PATTERN_FLASHBULB);
        }
        
        if (isFlashbulb && currentSegment < numSegments && segments[currentSegment] != nullptr) {
            // For flashbulb: run main pattern first, then flashbulb will override its pins
            segments[currentSegment]->update();
        }
        
        // Update the interrupt pattern (this will override target pins for flashbulb)
        interruptSegment->update();
        
        bool shouldFinish = false;
        if (isFlashbulb) {
            // For flashbulb, check its internal completion state
            shouldFinish = isFlashbulbComplete();
        } else {
            // For other patterns, use the segment's duration
            shouldFinish = interruptSegment->isFinished();
        }
        
        if (shouldFinish) {
            if (isFlashbulb) {
                // Flashbulb completed, restore immediately
                // Don't call stop() on flashbulb - it handles its own cleanup and 
                // stop() would clear LEDs to black causing the blink issue
                if (interruptSegment != nullptr) {
                    delete interruptSegment;
                    interruptSegment = nullptr;
                }
                resumeCurrentSegment();
                interruptState = INTERRUPT_NONE;
                
                // Immediately update the main segment to populate LEDs with pattern colors
                if (currentSegment < numSegments && segments[currentSegment] != nullptr) {
                    segments[currentSegment]->update();
                }
            } else {
                interruptState = INTERRUPT_TRANSITIONING_IN;
                transitionStartTime = millis();
            }
        }
    } else if (interruptState == INTERRUPT_NONE && currentSegment < numSegments && segments[currentSegment] != nullptr) {
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
}

bool Program::getIsRunning() { return isRunning; }

void Program::triggerInterrupt(PatternType type, int* pins, int numPins, unsigned long durationMs, PatternParams params)
{
    if (interruptState != INTERRUPT_NONE) {
        return;
    }

    pauseCurrentSegment();
    
    if (interruptSegment != nullptr) {
        delete interruptSegment;
    }
    
    interruptSegment = new InterruptSegment(type, pins, numPins, durationMs, params);
    
    // Flashbulb handles its own transitions, skip the fade system
    if (type == PATTERN_FLASHBULB) {
        interruptState = INTERRUPT_ACTIVE;
        if (interruptSegment != nullptr) {
            interruptSegment->start();
        }
    } else {
        interruptState = INTERRUPT_TRANSITIONING_OUT;
        transitionStartTime = millis();
    }
}

void Program::pauseCurrentSegment()
{
    if (currentSegment < numSegments && segments[currentSegment] != nullptr) {
        savedState.segmentIndex = currentSegment;
        savedState.pauseStartTime = millis();
        
        unsigned long elapsedTime = millis() - segments[currentSegment]->startTime;
        savedState.remainingDuration = segments[currentSegment]->duration - elapsedTime;
    }
}

void Program::resumeCurrentSegment()
{
    if (savedState.segmentIndex < numSegments && segments[savedState.segmentIndex] != nullptr) {
        currentSegment = savedState.segmentIndex;
        
        unsigned long pauseDuration = millis() - savedState.pauseStartTime;
        segments[currentSegment]->startTime = millis() - (segments[currentSegment]->duration - savedState.remainingDuration);
        
        segments[currentSegment]->isActive = true;
    }
}

void Program::updateTransitions()
{
    unsigned long currentTime = millis();
    
    switch (interruptState) {
        case INTERRUPT_TRANSITIONING_OUT:
            if (currentTime - transitionStartTime >= transitionDuration) {
                if (interruptSegment != nullptr) {
                    interruptSegment->start();
                }
                interruptState = INTERRUPT_ACTIVE;
                currentBrightness = originalBrightness;
                FastLED.setBrightness(currentBrightness);
            } else {
                // During transition out, continue running the current pattern but fade brightness
                float progress = (float)(currentTime - transitionStartTime) / transitionDuration;
                currentBrightness = originalBrightness * (1.0 - progress);
                FastLED.setBrightness(currentBrightness);
            }
            break;
            
        case INTERRUPT_TRANSITIONING_IN:
            if (currentTime - transitionStartTime >= transitionDuration) {
                if (interruptSegment != nullptr) {
                    interruptSegment->stop();
                    delete interruptSegment;
                    interruptSegment = nullptr;
                }
                resumeCurrentSegment();
                interruptState = INTERRUPT_NONE;
                currentBrightness = originalBrightness;
                FastLED.setBrightness(currentBrightness);
            } else {
                // During transition in, the interrupt pattern handles the transition itself
                // We just restore brightness gradually
                float progress = (float)(currentTime - transitionStartTime) / transitionDuration;
                currentBrightness = originalBrightness * progress;
                FastLED.setBrightness(currentBrightness);
            }
            break;
            
        default:
            break;
    }
}

void Program::fadeOut()
{
    FastLED.setBrightness(0);
    FastLED.show();
}

void Program::fadeIn()
{
    FastLED.setBrightness(originalBrightness);
    FastLED.show();
}