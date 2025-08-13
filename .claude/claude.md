# Fractured Light LED Art Installation

## Project Overview
ESP32-based LED art installation controlling 1,952 LEDs (8 pins × 2 strips × 122 LEDs) using FastLED library. Features sequential pattern segments with websocket-based pattern interrupts.

## Pattern Interrupt Implementation Steps

### 1. Add Websocket Dependencies ✅
Dependencies already added to `platformio.ini`:
- `esphome/AsyncTCP-esphome@2.1.1`
- `esphome/ESPAsyncWebServer-esphome@3.1.0` 
- `arduino-libraries/Arduino_JSON @ 0.2.0`
- `links2004/WebSockets@^2.6.1`

### 2. Extend Program Class with Interrupt State Management ✅
**File**: `src/program.h`
- Added `InterruptState` enum: `INTERRUPT_NONE`, `INTERRUPT_ACTIVE`, `INTERRUPT_TRANSITIONING_OUT`, `INTERRUPT_TRANSITIONING_IN`
- Added private members:
  - `InterruptState interruptState`
  - `InterruptSegment* interruptSegment` 
  - `SavedSegmentState savedState` struct
  - `unsigned long transitionStartTime`
  - `unsigned long transitionDuration` (500ms default)
  - Brightness control variables

**File**: `src/program.cpp`
- Modified `update()` method to handle interrupt states
- Added `triggerInterrupt()` method
- Added `pauseCurrentSegment()` and `resumeCurrentSegment()` methods
- Implemented `updateTransitions()` with fade logic

### 3. Create InterruptSegment Class ✅
**File**: `src/program.h` & `src/program.cpp`
- Created `InterruptSegment` class inheriting from `Segment`
- Override `isFinished()` to use interrupt-specific duration
- Override `start()` to track interrupt timing

### 4. Add State Preservation System ✅
**File**: `src/program.h`
- Created `SavedSegmentState` struct:
  - `int segmentIndex`
  - `unsigned long remainingDuration` 
  - `unsigned long pauseStartTime`
  - `PatternState* patternState`

**File**: `src/patterns.h` & `src/pattern_state.cpp`
- Added `PatternState` struct for breathing and spin patterns
- Added `savePatternStates()` and `restorePatternStates()` functions
- Modified pattern files to expose state variables

### 5. Implement Transition Effects ✅
**File**: `src/program.cpp`
- Added `fadeOut()` and `fadeIn()` methods using `FastLED.setBrightness()`
- Implemented smooth brightness transitions in `updateTransitions()`
- 500ms transition duration with linear fade

### 6. Add Websocket Integration ✅
**File**: `src/main.cpp`
- Added websocket libraries and WiFi Access Point setup
- Created websocket server on port 81
- Added message parsing for interrupt commands:
  ```json
  {
    "type": "interrupt",
    "pattern": "spin",
    "duration": 5000,
    "speed": 75,
    "palette": ["#FF0000", "#00FF00", "#0000FF"]
  }
  ```
- Added websocket event handlers and HTTP server with web interface

### 7. Pattern Command Structure
Supported interrupt patterns with parameters:
- **spin**: `speed`, `separation`, `span`, `palette`, `loop`, `continuous`, `blend`
- **breathing**: `speed`, `palette`
- **flame**: `speed`, `cooling`, `sparking`
- **grow**: `speed`, `n`, `fadeDelay`, `holdDelay`, `palette`, `transitionSpeed`
- **pop**: `speed`, `holdDelay`, `palette`, `random`, `accelerationTime`
- **flashbulb**: `flashDuration` (100ms), `fadeDuration` (5000ms), `transitionDuration` (2000ms)

### 8. Testing Commands
Build and upload:
```bash
platformio run --target upload
```

Monitor serial output:
```bash
platformio device monitor
```

## File Structure
```
src/
├── main.cpp           # Main program + websocket integration
├── program.h          # Program/Segment classes + interrupt state
├── program.cpp        # Program logic + interrupt handling  
├── patterns.h         # Pattern function declarations
├── breathing.cpp      # Breathing pattern implementation
├── flame.cpp          # Flame pattern implementation
├── grow.cpp           # Grow pattern implementation
├── pop.cpp            # Pop pattern implementation
└── spin.cpp           # Spin pattern implementation
```

## Architecture Notes
- Patterns use static state variables that need preservation during interrupts
- FastLED.show() called once per update cycle for performance
- Interrupt system is non-invasive to existing pattern logic
- State management ensures smooth resumption of original patterns