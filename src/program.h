#ifndef PROGRAM_H
#define PROGRAM_H

#include <FastLED.h>

enum PatternType {
    PATTERN_BREATHING,
    PATTERN_FLAME,
    PATTERN_GROW,
    PATTERN_CHASE
};

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
private:
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
    bool isFinished();
    void update();
    void addPattern(PatternInstance* pattern);
};

class Program {
private:
    Segment** segments;
    int numSegments;
    int currentSegment;
    bool isRunning;

public:
    Program(int segmentCount);
    ~Program();
    void addSegment(int index, Segment* segment);
    void start();
    void stop();
    void update();
    bool getIsRunning();
};

#endif