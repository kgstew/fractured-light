#include "patterns.h"
#include <Arduino.h>

// External references to pattern state variables
// Breathing pattern state (from breathing.cpp)
extern unsigned long lastUpdate;
extern float brightness;
extern bool increasing;
extern unsigned long colorTransitionTime;
extern float colorProgress;

// Spin pattern state (from spin.cpp)  
extern unsigned long lastUpdateTime;
extern int currentPosition[8];

void savePatternStates(PatternState* state)
{
    // Save breathing pattern state
    state->breathing.lastUpdateTime = lastUpdate;
    state->breathing.currentBrightness = (int)brightness;
    state->breathing.increasing = increasing;
    
    // Save spin pattern state
    state->spin.lastUpdateTime = lastUpdateTime;
    for (int i = 0; i < 8; i++) {
        state->spin.currentPosition[i] = currentPosition[i];
    }
}

void restorePatternStates(PatternState* state)
{
    // Restore breathing pattern state
    lastUpdate = state->breathing.lastUpdateTime;
    brightness = (float)state->breathing.currentBrightness;
    increasing = state->breathing.increasing;
    
    // Restore spin pattern state
    lastUpdateTime = state->spin.lastUpdateTime;
    for (int i = 0; i < 8; i++) {
        currentPosition[i] = state->spin.currentPosition[i];
    }
}