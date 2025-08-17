# Fractured Light: Improvements Needed

## Current State Analysis

Fractured Light is a simpler LED art installation with:
- **2 Pattern Types**: Breathing and Flame only
- **Sequential Architecture**: Only one segment runs at a time
- **Dynamic Memory**: Uses `new`/`delete` for segments and pin arrays
- **Limited Features**: No transitions, queuing, or advanced effects

## Critical Issues Found

### 🚨 **1. Memory Safety Problems (HIGH PRIORITY)**

**Dynamic Memory Allocation**:
```cpp
// PROBLEM: Dynamic allocation on embedded system
Segment::Segment(...) {
    pins = new int[pinCount];  // Heap allocation
}

Program::Program(int segmentCount) {
    segments = new Segment*[segmentCount];  // More heap allocation
}
```

**Risks**:
- **Heap fragmentation** over time
- **Memory leaks** if destructors fail
- **Out of memory** errors during runtime
- **Unpredictable behavior** in embedded environment

### 🚨 **2. Stack Size Not Configured**
- **Current**: 8KB default stack (same issue as reflecting-the-present had)
- **Risk**: Stack overflow with complex patterns
- **Solution**: Need to add `board_build.arduino.stack_size = 16384`

### ⚠️ **3. Limited Pattern Library**
- **Current**: Only 2 pattern types (breathing, flame)
- **Missing**: Chase, rainbow, pinwheel, warp, solid patterns
- **Impact**: Very limited creative possibilities

### ⚠️ **4. No Concurrent Patterns**
- **Current**: Only one segment active at a time
- **Missing**: Cannot run different patterns on different pin groups simultaneously
- **Impact**: Less dynamic light shows

### ⚠️ **5. No Advanced Features**
- **Missing**: Pattern transitions (fade in/out)
- **Missing**: FlashBulb/interrupt effects
- **Missing**: Real-time parameter adjustment
- **Missing**: Strip-level control (all pins treated equally)

## Recommended Improvements

### **Phase 1: Critical Memory Safety (IMMEDIATE)**

#### 1.1 Fix Stack Size
```ini
# Add to platformio.ini
[env:esp32dev]
# ... existing config ...
board_build.arduino.stack_size = 16384
```

#### 1.2 Replace Dynamic Allocation
```cpp
// Replace dynamic arrays with static allocation
#define MAX_SEGMENTS 20
#define MAX_PINS_PER_SEGMENT 8

class Program {
private:
    Segment segments[MAX_SEGMENTS];  // Static allocation
    uint8_t segment_count;
    bool segment_active[MAX_SEGMENTS];
    
public:
    Program() : segment_count(0) {
        memset(segment_active, false, sizeof(segment_active));
    }
    // No need for destructor - no dynamic memory
};

class Segment {
private:
    uint8_t pins[MAX_PINS_PER_SEGMENT];  // Static allocation
    uint8_t pin_count;
    
public:
    Segment() : pin_count(0) {}
    // No need for destructor - no dynamic memory
};
```

### **Phase 2: Enhanced Pattern Library (HIGH PRIORITY)**

#### 2.1 Add Missing Pattern Types
```cpp
enum PatternType {
    PATTERN_BREATHING,
    PATTERN_FLAME,
    PATTERN_CHASE,      // Add from reflecting-the-present
    PATTERN_RAINBOW,    // Add from reflecting-the-present  
    PATTERN_PINWHEEL,   // Add from reflecting-the-present
    PATTERN_WARP,       // Add from reflecting-the-present
    PATTERN_SOLID       // Add from reflecting-the-present
};
```

#### 2.2 Expand PatternParams Union
```cpp
struct PatternParams {
    union {
        struct {
            int speed;
            CRGB color;
        } breathing;
        
        struct {
            int speed;
            int cooling;
            int sparking;
        } flame;
        
        // Add all the new pattern params from reflecting-the-present
        struct {
            uint8_t chase_width;
            float fade_rate;
            bool bounce_mode;
            bool color_shift;
        } chase;
        
        struct {
            float rotation_speed;
            float color_cycles;
            bool radial_fade;
            float center_brightness;
        } pinwheel;
        
        struct {
            uint16_t acceleration_delay;
            bool fade_previous;
        } warp;
        
        // ... etc for all patterns
    };
};
```

### **Phase 3: Concurrent Pattern Support (MEDIUM PRIORITY)**

#### 3.1 Allow Multiple Active Segments
```cpp
class Program {
public:
    void update() {
        // Run ALL active segments, not just current one
        for (uint8_t i = 0; i < segment_count; i++) {
            if (segment_active[i] && !segments[i].isFinished()) {
                segments[i].update();
            } else if (segments[i].isFinished()) {
                segment_active[i] = false;  // Segment finished
            }
        }
        
        // Check if any segments should start
        checkSegmentStartTimes();
    }
};
```

### **Phase 4: Advanced Features (LOW PRIORITY)**

#### 4.1 Add Pattern Transitions
```cpp
struct Segment {
    unsigned long transition_duration;
    bool is_transitioning;
    unsigned long transition_start;
    
    void startTransition() {
        is_transitioning = true;
        transition_start = millis();
    }
};
```

#### 4.2 Add Strip-Level Control
Convert from pin-based to strip-based addressing (like reflecting-the-present).

## Migration Strategy

### **Option A: Incremental Improvements**
1. Fix memory safety issues first
2. Add patterns one by one
3. Gradually add advanced features
4. **Timeline**: 2-3 weeks

### **Option B: Architectural Upgrade**
1. Adopt reflecting-the-present architecture
2. Simplify by removing features fractured-light doesn't need
3. Keep the segment-based program concept
4. **Timeline**: 1-2 weeks

## Recommended Approach: Option B

**Why**: Reflecting-the-present has solved all these problems already. Better to:
1. **Copy the proven architecture** (static allocation, robust pattern system)
2. **Adapt the Program/Segment concept** on top of it  
3. **Remove unused features** (WebSocket, FlashBulb, etc.)
4. **Keep fractured-light's simplicity** while gaining safety and features

## Priority Order

| Priority | Task | Impact | Effort |
|----------|------|--------|--------|
| **CRITICAL** | Fix dynamic memory allocation | Prevents crashes | Medium |
| **HIGH** | Increase stack size | Prevents stack overflow | Low |
| **HIGH** | Add missing patterns | 5x more creative options | Medium |
| **MEDIUM** | Concurrent segment support | Better light shows | Medium |
| **LOW** | Pattern transitions | Polish feature | High |

The fractured-light project needs significant memory safety improvements and would benefit greatly from adopting the lessons learned in reflecting-the-present!