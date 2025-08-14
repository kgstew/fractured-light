#include "patterns.h"
#include "program.h"
#include <Arduino.h>
#include <FastLED.h>

// Enable this to turn on WiFi and websocket interrupts
#define USE_INTERRUPTS

#ifdef USE_INTERRUPTS
#include <Arduino_JSON.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <WiFi.h>
#endif

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

#ifdef USE_INTERRUPTS
// WiFi and WebSocket configuration
const char* ssid = "StreetLighting";
const char* password = "lightitup";

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Sensor ID to LED pin mapping
#define MAX_SENSORS 8

struct SensorMapping {
    uint8_t sensor_id;
    uint8_t led_pins[4]; // Up to 4 pins per sensor
    uint8_t num_pins;
    bool active;
    unsigned long last_trigger_time;
};

SensorMapping sensor_mappings[MAX_SENSORS] = {
    { 1, { 0 }, 1, true, 0 }, // Sensor 1 -> Pin 0
    { 2, { 1 }, 1, true, 0 }, // Sensor 2 -> Pin 1
    { 3, { 2 }, 1, true, 0 }, // Sensor 3 -> Pin 2
    { 4, { 3, 4 }, 2, true, 0 }, // Sensor 4 -> Pins 3,4
    { 5, { 5, 6 }, 2, true, 0 }, // Sensor 5 -> Pins 5,6
    { 6, { 7 }, 1, true, 0 }, // Sensor 6 -> Pin 7
    { 7, { 0, 1, 2, 3 }, 4, false, 0 }, // Sensor 7 -> Pins 0-3 (disabled)
    { 8, { 4, 5, 6, 7 }, 4, false, 0 }, // Sensor 8 -> Pins 4-7 (disabled)
};

// Function declarations
void setupWiFiAndWebSocket();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
void handleSensorMessage(String message);
void testInterrupts();
#endif

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
    mainProgram = new Program(1);

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

    // // Segment 2: Multi-color breathing on all pins for 10 seconds
    // static CRGB breathingPalette[] = { CRGB::Purple, CRGB::Magenta, CRGB::Blue, CRGB::Cyan };
    // PatternParams breathingParams;
    // breathingParams.breathing.speed = 50;
    // breathingParams.breathing.palette = breathingPalette;
    // breathingParams.breathing.paletteSize = 4;
    // mainProgram->addSegment(1, new Segment(PATTERN_BREATHING, allPins, 8, 10, breathingParams));

    // // Segment 3: Flame pattern on all pins for 15 seconds
    // PatternParams flameParams;
    // flameParams.flame.speed = 80;
    // flameParams.flame.cooling = 55;
    // flameParams.flame.sparking = 120;
    // mainProgram->addSegment(2, new Segment(PATTERN_FLAME, allPins, 8, 10, flameParams, 1));

    // // Segment 4: Grow pattern on all pins for 20 seconds
    // static CRGB growPalette[] = { CRGB::Cyan, CRGB::Blue, CRGB::Purple, CRGB::Magenta, CRGB::Red, CRGB::Orange };
    // PatternParams growParams;
    // growParams.grow.speed = 60;
    // growParams.grow.n = 1;
    // growParams.grow.fadeDelay = 100;
    // growParams.grow.holdDelay = 2000;
    // growParams.grow.palette = growPalette;
    // growParams.grow.paletteSize = 6;
    // growParams.grow.transitionSpeed = 40;
    // growParams.grow.offsetDelay = 1000;
    // mainProgram->addSegment(3, new Segment(PATTERN_GROW, allPins, 8, 10, growParams, 1));

    // // Segment 5: Multi-pattern segment - different patterns on different pins
    // // Create pattern instances for different pin groups
    // PatternInstance* patterns[3];

    // // Breathing on pins 0-2
    // int breathingPins[] = { 0, 1, 2 };
    // static CRGB multiBreathingPalette[] = { CRGB(0, 255, 128), CRGB::Green, CRGB::Teal };
    // PatternParams multiBreathingParams;
    // multiBreathingParams.breathing.speed = 60;
    // multiBreathingParams.breathing.palette = multiBreathingPalette;
    // multiBreathingParams.breathing.paletteSize = 3;
    // patterns[0] = new PatternInstance(PATTERN_BREATHING, breathingPins, 3, multiBreathingParams);

    // // Flame on pins 3-5
    // int flamePins[] = { 3, 4, 5 };
    // PatternParams multiFlameParams;
    // multiFlameParams.flame.speed = 90;
    // multiFlameParams.flame.cooling = 60;
    // multiFlameParams.flame.sparking = 130;
    // patterns[1] = new PatternInstance(PATTERN_FLAME, flamePins, 3, multiFlameParams);

    // int growPins[] = { 6, 7 };
    // PatternParams growParams2;
    // growParams2.grow.speed = 60;
    // growParams2.grow.n = 1;
    // growParams2.grow.fadeDelay = 100;
    // growParams2.grow.holdDelay = 2000;
    // growParams2.grow.palette = growPalette;
    // growParams2.grow.paletteSize = 6;
    // growParams2.grow.transitionSpeed = 40;
    // growParams2.grow.offsetDelay = 1000;
    // patterns[2] = new PatternInstance(PATTERN_GROW, growPins, 2, growParams2);

    // mainProgram->addSegment(4, new Segment(patterns, 3, 5));

    // // Segment 6: Pop pattern with random pins and acceleration
    // static CRGB popPalette[]
    //     = { CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green, CRGB::Blue, CRGB::Purple, CRGB::Pink, CRGB::White };
    // PatternParams popParams;
    // popParams.pop.speed = 10; // Maximum speed after acceleration
    // popParams.pop.holdDelay = 300; // Hold each color for 300ms
    // popParams.pop.palette = popPalette;
    // popParams.pop.paletteSize = 8;
    // popParams.pop.random = true; // Randomize pin order
    // popParams.pop.accelerationTime = 8; // Accelerate over 8 seconds
    // mainProgram->addSegment(5, new Segment(PATTERN_POP, allPins, 8, 20, popParams));

    mainProgram->start();

#ifdef USE_INTERRUPTS
    // Setup WiFi and WebSocket
    setupWiFiAndWebSocket();
#endif
}

#ifdef USE_INTERRUPTS
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
        handleSensorMessage((char*)payload);
        break;

    default:
        break;
    }
}

// Handle incoming sensor messages
void handleSensorMessage(String message)
{
    JSONVar json = JSON.parse(message);

    if (JSON.typeof(json) == "undefined") {
        Serial.println("Failed to parse JSON");
        return;
    }

    if (json.hasOwnProperty("sensorId") && json.hasOwnProperty("timestamp")) {
        int sensor_id = (int)json["sensorId"];
        unsigned long timestamp = (unsigned long)json["timestamp"];

        Serial.print("Sensor message received - ID: ");
        Serial.print(sensor_id);
        Serial.print(", Timestamp: ");
        Serial.println(timestamp);

        // Find the sensor mapping
        for (uint8_t i = 0; i < MAX_SENSORS; i++) {
            if (sensor_mappings[i].active && sensor_mappings[i].sensor_id == sensor_id) {
                // Check if enough time has passed since last trigger (10 seconds = 10000ms)
                unsigned long current_time = millis();
                unsigned long time_since_last = current_time - sensor_mappings[i].last_trigger_time;

                if (sensor_mappings[i].last_trigger_time == 0 || time_since_last >= 10000) {
                    // Update last trigger time
                    sensor_mappings[i].last_trigger_time = current_time;

                    // Setup flashbulb pattern parameters
                    PatternParams flashbulbParams;
                    flashbulbParams.flashbulb.flashDuration = 100; // 100ms flash
                    flashbulbParams.flashbulb.fadeDuration = 5000; // 5 second fade
                    flashbulbParams.flashbulb.transitionDuration = 2000; // 2 second transition back

                    // Calculate total duration
                    unsigned long duration = flashbulbParams.flashbulb.flashDuration
                        + flashbulbParams.flashbulb.fadeDuration + flashbulbParams.flashbulb.transitionDuration;

                    // Convert sensor pins to int array
                    int sensorPins[4];
                    for (uint8_t j = 0; j < sensor_mappings[i].num_pins; j++) {
                        sensorPins[j] = (int)sensor_mappings[i].led_pins[j];
                    }

                    Serial.print("Flashbulb triggered on ");
                    Serial.print(sensor_mappings[i].num_pins);
                    Serial.print(" pins: ");
                    for (uint8_t j = 0; j < sensor_mappings[i].num_pins; j++) {
                        Serial.print(sensor_mappings[i].led_pins[j]);
                        if (j < sensor_mappings[i].num_pins - 1)
                            Serial.print(", ");
                    }
                    Serial.println();

                    // Trigger the flashbulb interrupt
                    mainProgram->triggerInterrupt(
                        PATTERN_FLASHBULB, sensorPins, sensor_mappings[i].num_pins, duration, flashbulbParams);
                } else {
                    // Too soon since last trigger
                    unsigned long wait_time = 10000 - time_since_last;
                    Serial.print("Sensor ");
                    Serial.print(sensor_id);
                    Serial.print(" trigger ignored - wait ");
                    Serial.print(wait_time / 1000);
                    Serial.println(" more seconds");
                }

                break;
            }
        }
    }
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
        html += "<h1>Fractured Light - Sensor Triggered Flashbulbs</h1>";
        html += "<p>WebSocket server running on port 81</p>";
        html += "<h2>Send JSON messages with format:</h2>";
        html += "<pre>{\"sensorId\": 1, \"timestamp\": 1234567890}</pre>";
        html += "<h2>Sensor Mappings:</h2><ul>";

        for (uint8_t i = 0; i < MAX_SENSORS; i++) {
            if (sensor_mappings[i].active) {
                html += "<li>Sensor " + String(sensor_mappings[i].sensor_id) + " -> Pins: ";
                for (uint8_t j = 0; j < sensor_mappings[i].num_pins; j++) {
                    html += String(sensor_mappings[i].led_pins[j]);
                    if (j < sensor_mappings[i].num_pins - 1)
                        html += ", ";
                }
                html += " (Flashbulb)</li>";
            }
        }

        html += "</ul>";
        html += "<p><strong>Note:</strong> Each sensor has a 10-second cooldown period</p>";
        html += "<p><strong>Test:</strong> Random flashbulb patterns trigger every 10 seconds</p>";
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
#endif

#ifdef USE_INTERRUPTS
// Test function that triggers random flashbulb pattern interrupts every 10 seconds
void testInterrupts()
{
    static unsigned long lastTestTime = 0;
    unsigned long currentTime = millis();

    // Trigger interrupt every 10 seconds
    if (currentTime - lastTestTime >= 10000) {
        lastTestTime = currentTime;

        // Generate random number of pins (1-4)
        int numRandomPins = random(1, 5);
        int randomPins[4];

        // Select random pins without duplicates
        bool usedPins[8] = { false };
        for (int i = 0; i < numRandomPins; i++) {
            int pin;
            do {
                pin = random(0, 8);
            } while (usedPins[pin]);
            usedPins[pin] = true;
            randomPins[i] = pin;
        }

        // Setup flashbulb pattern parameters
        PatternParams flashbulbParams;
        flashbulbParams.flashbulb.flashDuration = random(50, 200); // Random flash duration 50-200ms
        flashbulbParams.flashbulb.fadeDuration = random(3000, 7000); // Random fade duration 3-7 seconds
        flashbulbParams.flashbulb.transitionDuration = random(1000, 3000); // Random transition 1-3 seconds

        // Calculate total duration for the interrupt
        unsigned long duration = flashbulbParams.flashbulb.flashDuration + flashbulbParams.flashbulb.fadeDuration
            + flashbulbParams.flashbulb.transitionDuration;

        Serial.print("Test interrupt: Flashbulb pattern on ");
        Serial.print(numRandomPins);
        Serial.print(" pins (");
        for (int i = 0; i < numRandomPins; i++) {
            Serial.print(randomPins[i]);
            if (i < numRandomPins - 1)
                Serial.print(", ");
        }
        Serial.print(") - Flash: ");
        Serial.print(flashbulbParams.flashbulb.flashDuration);
        Serial.print("ms, Fade: ");
        Serial.print(flashbulbParams.flashbulb.fadeDuration);
        Serial.print("ms, Transition: ");
        Serial.print(flashbulbParams.flashbulb.transitionDuration);
        Serial.print("ms, Total: ");
        Serial.print(duration);
        Serial.println("ms");

        // Trigger the interrupt
        mainProgram->triggerInterrupt(PATTERN_FLASHBULB, randomPins, numRandomPins, duration, flashbulbParams);
    }
}
#endif

void loop()
{
#ifdef USE_INTERRUPTS
    // Handle WebSocket events
    webSocket.loop();

    // Test interrupts (comment out when not needed)
    testInterrupts();
#endif

    // Update main program (patterns and interrupts)
    mainProgram->update();
}