#include "patterns.h"
#include "program.h"
#include <Arduino.h>
#include <Arduino_JSON.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <WebSocketsServer.h>
#include <WiFi.h>

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

// WiFi and WebSocket configuration
const char* ssid = "FracturedLight";
const char* password = "lightshow2024";

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Function declarations
void setupWiFiAndWebSocket();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
void handleInterruptMessage(String message);
CRGB parseColor(String colorStr);
void testInterrupts();

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

    // Create a program with 6 segments
    mainProgram = new Program(6);

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

    mainProgram->start();
    
    // Setup WiFi and WebSocket
    setupWiFiAndWebSocket();
}

// WebSocket event handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length)
{
    switch (type) {
    case WStype_DISCONNECTED:
        Serial.printf("[%u] Disconnected!\n", num);
        break;

    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
    } break;

    case WStype_TEXT:
        Serial.printf("[%u] Received: %s\n", num, payload);
        handleInterruptMessage((char*)payload);
        break;

    default:
        break;
    }
}

// Handle incoming interrupt messages
void handleInterruptMessage(String message)
{
    JSONVar json = JSON.parse(message);

    if (JSON.typeof(json) == "undefined") {
        Serial.println("Failed to parse JSON");
        return;
    }

    if (json.hasOwnProperty("type") && String((const char*)json["type"]) == "interrupt") {
        String patternName = String((const char*)json["pattern"]);
        unsigned long duration = (unsigned long)json["duration"];
        
        PatternType patternType;
        PatternParams params;
        
        // Parse pattern type
        if (patternName == "spin") {
            patternType = PATTERN_SPIN;
            params.spin.speed = json.hasOwnProperty("speed") ? (int)json["speed"] : 75;
            params.spin.separation = json.hasOwnProperty("separation") ? (int)json["separation"] : 20;
            params.spin.span = json.hasOwnProperty("span") ? (int)json["span"] : 15;
            params.spin.loop = json.hasOwnProperty("loop") ? (bool)json["loop"] : true;
            params.spin.continuous = json.hasOwnProperty("continuous") ? (bool)json["continuous"] : true;
            params.spin.blend = json.hasOwnProperty("blend") ? (bool)json["blend"] : true;
            
            // Parse palette
            static CRGB interruptPalette[8];
            int paletteSize = 4;
            if (json.hasOwnProperty("palette")) {
                JSONVar palette = json["palette"];
                paletteSize = 0;
                for (int i = 0; i < 8 && palette.hasOwnProperty(String(i)); i++) {
                    interruptPalette[i] = parseColor(String((const char*)palette[String(i)]));
                    paletteSize++;
                }
            } else {
                // Default palette
                interruptPalette[0] = CRGB::Red;
                interruptPalette[1] = CRGB::Blue; 
                interruptPalette[2] = CRGB::Green;
                interruptPalette[3] = CRGB::Yellow;
            }
            params.spin.palette = interruptPalette;
            params.spin.paletteSize = paletteSize;
            
        } else if (patternName == "breathing") {
            patternType = PATTERN_BREATHING;
            params.breathing.speed = json.hasOwnProperty("speed") ? (int)json["speed"] : 50;
            
            // Parse palette
            static CRGB interruptPalette[8];
            int paletteSize = 3;
            if (json.hasOwnProperty("palette")) {
                JSONVar palette = json["palette"];
                paletteSize = 0;
                for (int i = 0; i < 8 && palette.hasOwnProperty(String(i)); i++) {
                    interruptPalette[i] = parseColor(String((const char*)palette[String(i)]));
                    paletteSize++;
                }
            } else {
                // Default palette
                interruptPalette[0] = CRGB::Purple;
                interruptPalette[1] = CRGB::Magenta;
                interruptPalette[2] = CRGB::Blue;
            }
            params.breathing.palette = interruptPalette;
            params.breathing.paletteSize = paletteSize;
            
        } else if (patternName == "flame") {
            patternType = PATTERN_FLAME;
            params.flame.speed = json.hasOwnProperty("speed") ? (int)json["speed"] : 80;
            params.flame.cooling = json.hasOwnProperty("cooling") ? (int)json["cooling"] : 55;
            params.flame.sparking = json.hasOwnProperty("sparking") ? (int)json["sparking"] : 120;
            
        } else if (patternName == "pop") {
            patternType = PATTERN_POP;
            params.pop.speed = json.hasOwnProperty("speed") ? (int)json["speed"] : 10;
            params.pop.holdDelay = json.hasOwnProperty("holdDelay") ? (int)json["holdDelay"] : 300;
            params.pop.random = json.hasOwnProperty("random") ? (bool)json["random"] : true;
            params.pop.accelerationTime = json.hasOwnProperty("accelerationTime") ? (int)json["accelerationTime"] : 8;
            
            // Parse palette
            static CRGB interruptPalette[8];
            int paletteSize = 8;
            if (json.hasOwnProperty("palette")) {
                JSONVar palette = json["palette"];
                paletteSize = 0;
                for (int i = 0; i < 8 && palette.hasOwnProperty(String(i)); i++) {
                    interruptPalette[i] = parseColor(String((const char*)palette[String(i)]));
                    paletteSize++;
                }
            } else {
                // Default pop palette
                interruptPalette[0] = CRGB::Red;
                interruptPalette[1] = CRGB::Orange;
                interruptPalette[2] = CRGB::Yellow;
                interruptPalette[3] = CRGB::Green;
                interruptPalette[4] = CRGB::Blue;
                interruptPalette[5] = CRGB::Purple;
                interruptPalette[6] = CRGB::Pink;
                interruptPalette[7] = CRGB::White;
            }
            params.pop.palette = interruptPalette;
            params.pop.paletteSize = paletteSize;
            
        } else {
            Serial.println("Unknown pattern type: " + patternName);
            return;
        }
        
        // Use all pins for interrupt
        int allPins[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
        
        Serial.printf("Triggering %s interrupt for %lums\n", patternName.c_str(), duration);
        mainProgram->triggerInterrupt(patternType, allPins, 8, duration, params);
    }
}

// Parse color string (hex format like "#FF0000")
CRGB parseColor(String colorStr)
{
    if (colorStr.startsWith("#") && colorStr.length() == 7) {
        long color = strtol(colorStr.c_str() + 1, NULL, 16);
        return CRGB((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    }
    return CRGB::White; // Default fallback
}

// Setup WiFi Access Point and WebSocket server
void setupWiFiAndWebSocket()
{
    // Set up WiFi Access Point
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Setup WebSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket server started on port 81");

    // Setup basic web server
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        String html = "<!DOCTYPE html><html><head><title>Fractured Light</title></head><body>";
        html += "<h1>Fractured Light - LED Pattern Interrupts</h1>";
        html += "<p>WebSocket server running on port 81</p>";
        html += "<h2>Send JSON messages with format:</h2>";
        html += "<pre>{\"type\":\"interrupt\",\"pattern\":\"spin\",\"duration\":5000,\"speed\":75}</pre>";
        html += "<h2>Supported patterns:</h2>";
        html += "<ul><li>spin (speed, separation, span, loop, continuous, blend, palette)</li>";
        html += "<li>breathing (speed, palette)</li>";
        html += "<li>flame (speed, cooling, sparking)</li>";
        html += "<li>pop (speed, holdDelay, random, accelerationTime, palette)</li></ul>";
        html += "<p><strong>Note:</strong> testInterrupts() runs automatically every 5 seconds with random pop patterns</p>";
        html += "</body></html>";
        request->send(200, "text/html", html);
    });

    server.begin();
    Serial.println("HTTP server started on port 80");
    Serial.println("=== WiFi Setup Complete ===");
    Serial.println("Connect to WiFi: FracturedLight");
    Serial.println("Password: lightshow2024");
    Serial.println("WebSocket: ws://192.168.4.1:81");
    Serial.println("Web interface: http://192.168.4.1");
}

// Test function that triggers random pop pattern interrupts every 5 seconds
void testInterrupts()
{
    static unsigned long lastTestTime = 0;
    unsigned long currentTime = millis();
    
    // Trigger interrupt every 5 seconds
    if (currentTime - lastTestTime >= 5000) {
        lastTestTime = currentTime;
        
        // Generate random number of pins (1-4)
        int numRandomPins = random(1, 5);
        int randomPins[4];
        
        // Select random pins without duplicates
        bool usedPins[8] = {false};
        for (int i = 0; i < numRandomPins; i++) {
            int pin;
            do {
                pin = random(0, 8);
            } while (usedPins[pin]);
            usedPins[pin] = true;
            randomPins[i] = pin;
        }
        
        // Setup pop pattern parameters
        PatternParams popParams;
        popParams.pop.speed = random(5, 20); // Random speed 5-20
        popParams.pop.holdDelay = random(200, 500); // Random hold delay 200-500ms
        popParams.pop.random = true;
        popParams.pop.accelerationTime = random(3, 8); // Random acceleration 3-8 seconds
        
        // Random palette colors
        static CRGB testPalette[8];
        testPalette[0] = CRGB::Red;
        testPalette[1] = CRGB::Orange;
        testPalette[2] = CRGB::Yellow;
        testPalette[3] = CRGB::Green;
        testPalette[4] = CRGB::Blue;
        testPalette[5] = CRGB::Purple;
        testPalette[6] = CRGB::Pink;
        testPalette[7] = CRGB::White;
        
        popParams.pop.palette = testPalette;
        popParams.pop.paletteSize = 8;
        
        // Random duration between 2-6 seconds
        unsigned long duration = random(2000, 6000);
        
        Serial.print("Test interrupt: Pop pattern on ");
        Serial.print(numRandomPins);
        Serial.print(" pins (");
        for (int i = 0; i < numRandomPins; i++) {
            Serial.print(randomPins[i]);
            if (i < numRandomPins - 1) Serial.print(", ");
        }
        Serial.print(") for ");
        Serial.print(duration);
        Serial.println("ms");
        
        // Trigger the interrupt
        mainProgram->triggerInterrupt(PATTERN_POP, randomPins, numRandomPins, duration, popParams);
    }
}

void loop() 
{
    // Handle WebSocket events
    webSocket.loop();
    
    // Test interrupts (comment out when not needed)
    testInterrupts();
    
    // Update main program (patterns and interrupts)
    mainProgram->update();
}