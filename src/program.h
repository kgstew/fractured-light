#ifndef PROGRAM_H
#define PROGRAM_H

#include <FastLED.h>

enum PatternType {
    PATTERN_BREATHING,
    PATTERN_FLAME,
    PATTERN_GROW,
    PATTERN_CHASE,
    PATTERN_POP,
    PATTERN_SPIN
};

enum InterruptState {
    INTERRUPT_NONE,
    INTERRUPT_TRANSITIONING_OUT,
    INTERRUPT_ACTIVE,
    INTERRUPT_TRANSITIONING_IN
};

struct PatternParams {
    union {
        struct {
            int speed;
            CRGB* palette;
            int paletteSize;
        } breathing;
        struct {
            int speed;
            int cooling;
            int sparking;
        } flame;
        struct {
            int speed;
            int n;
            int fadeDelay;
            int holdDelay;
            CRGB* palette;
            int paletteSize;
            int transitionSpeed;
            int offsetDelay;
        } grow;
        struct {
            int speed;
            CRGB* palette;
            int paletteSize;
            int transitionSpeed;
            int holdDelay;
            int offsetDelay;
        } chase;
        struct {
            int speed;
            int holdDelay;
            CRGB* palette;
            int paletteSize;
            bool random;
            int accelerationTime;
        } pop;
        struct {
            int speed;
            int separation;
            int span;
            CRGB* palette;
            int paletteSize;
            bool loop;
            bool continuous;
            bool blend;
        } spin;
    };
};

struct PatternInstance {
    PatternType patternType;
    int* pins;
    int numPins;
    PatternParams params;
    bool reverse;
    
    PatternInstance(PatternType type, int* pinArray, int pinCount, PatternParams parameters, bool reverseDirection = false);
    ~PatternInstance();
};

class Segment {
protected:
    PatternInstance** patterns;
    int numPatterns;
    unsigned long duration;
    unsigned long startTime;
    bool isActive;

public:
    Segment(PatternType type, int* pinArray, int pinCount, unsigned long durationSeconds, PatternParams parameters, bool reverseDirection = false);
    Segment(PatternInstance** patternArray, int patternCount, unsigned long durationSeconds);
    ~Segment();
    void start();
    void stop();
    virtual bool isFinished();
    void update();
    void addPattern(PatternInstance* pattern);
    
    friend class Program;
};

// Forward declaration
struct PatternState;

struct SavedSegmentState {
    int segmentIndex;
    unsigned long remainingDuration;
    unsigned long pauseStartTime;
    PatternState* patternState;
};

class InterruptSegment : public Segment {
private:
    unsigned long interruptDuration;
    unsigned long interruptStartTime;

public:
    InterruptSegment(PatternType type, int* pinArray, int pinCount, unsigned long durationMs, PatternParams parameters, bool reverseDirection = false);
    InterruptSegment(PatternInstance** patternArray, int patternCount, unsigned long durationMs);
    bool isFinished() override;
    void start() override;
};

class Program {
private:
    Segment** segments;
    int numSegments;
    int currentSegment;
    bool isRunning;
    
    InterruptState interruptState;
    InterruptSegment* interruptSegment;
    SavedSegmentState savedState;
    unsigned long transitionStartTime;
    unsigned long transitionDuration;
    uint8_t originalBrightness;
    uint8_t currentBrightness;

public:
    Program(int segmentCount);
    ~Program();
    void addSegment(int index, Segment* segment);
    void start();
    void stop();
    void update();
    bool getIsRunning();
    
    void triggerInterrupt(PatternType type, int* pins, int numPins, unsigned long durationMs, PatternParams params);
    void pauseCurrentSegment();
    void resumeCurrentSegment();
    void updateTransitions();
    void fadeOut();
    void fadeIn();
};

#endif