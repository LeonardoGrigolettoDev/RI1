#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "camera_service.h"
#include "mqtt_service.h"
#include "wifi_service.h"

// Task handles
TaskHandle_t mqttWorkerHandle = NULL;
TaskHandle_t cameraWorkerHandle = NULL;
TaskHandle_t rtspWorkerHandle = NULL;

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32-CAM Multi-Worker System ===");
    
    // Display chip information
    uint64_t chipid = ESP.getEfuseMac();
    Serial.printf("Chip ID (Efuse MAC): %04X\n", (uint32_t)(chipid >> 32));
    Serial.printf("MAC Address: %012llX\n", chipid);
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    
    // Initialize camera first
    Serial.println("\nInitializing camera...");
    if (!initCamera()) {
        Serial.println("ERROR: Camera initialization failed!");
        Serial.println("System cannot continue without camera. Restarting in 5 seconds...");
        delay(5000);
        ESP.restart();
    }
    Serial.println("Camera initialized successfully!");
    
    Serial.println("\nStarting worker tasks...");
    
    // Create Worker 1: MQTT Listener
    Serial.println("Creating MQTT Worker (Worker 1)...");
    xTaskCreatePinnedToCore(
        mqttWorkerTask,           // Task function
        "Worker1_MQTT",           // Task name
        8192,                     // Stack size (increased for WiFi/MQTT)
        NULL,                     // Parameters
        3,                        // Priority (highest - for connectivity)
        &mqttWorkerHandle,        // Task handle
        0                         // Core 0
    );
    
    // Create Worker 2: Camera Handler
    Serial.println("Creating Camera Worker (Worker 2)...");
    xTaskCreatePinnedToCore(
        cameraWorkerTask,         // Task function
        "Worker2_Camera",         // Task name
        4096,                     // Stack size
        NULL,                     // Parameters
        2,                        // Priority (medium)
        &cameraWorkerHandle,      // Task handle
        1                         // Core 1
    );
    
    // Create Worker 3: RTSP Stream Handler (Optional)
    // Serial.println("Creating RTSP Worker (Worker 3)...");
    // xTaskCreatePinnedToCore(
    //     rtspWorkerTask,           // Task function
    //     "Worker3_RTSP",           // Task name
    //     8192,                     // Stack size (increased for network handling)
    //     NULL,                     // Parameters
    //     1,                        // Priority (lowest - optional service)
    //     &rtspWorkerHandle,        // Task handle
    //     0                         // Core 0
    // );
    
    Serial.println("All workers created successfully!");
    Serial.println("\nSystem Overview:");
    Serial.println("- Worker 1 (MQTT): Handles MQTT communication and commands");
    Serial.println("- Worker 2 (Camera): Captures and processes images");
    Serial.println("- Worker 3 (RTSP): Provides video streaming (optional)");
    Serial.println("\nConfiguration needed:");
    Serial.println("- Update WiFi credentials in mqtt_service.h");
    Serial.println("- Update MQTT server details in mqtt_service.h");
    Serial.println("=====================================\n");
}

void loop() {
    // Main loop - system monitoring and status reporting
    static unsigned long lastSystemReport = 0;
    
    if (millis() - lastSystemReport > 10000) { // Every 10 seconds
        Serial.printf("\n=== System Status Report at %lu ms ===\n", millis());
        
        // Memory status
        Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
        Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
        Serial.printf("Images captured: %d\n", imageCounter);
        
        // Worker status
        Serial.printf("Worker 1 (MQTT): %s\n", 
                     mqttWorkerHandle ? "Running" : "Stopped");
        Serial.printf("Worker 2 (Camera): %s\n", 
                     cameraWorkerHandle ? "Running" : "Stopped");
        Serial.printf("Worker 3 (RTSP): %s\n", 
                     rtspWorkerHandle ? "Running" : "Stopped");
        
        // Connectivity status
        Serial.printf("WiFi Status: %s\n", 
                     isWiFiConnected() ? "Connected" : "Disconnected");
        Serial.printf("MQTT Status: %s\n", 
                     mqttConnected ? "Connected" : "Disconnected");
        
        Serial.println("=========================================\n");
        lastSystemReport = millis();
    }
    
    // Check for system health and restart if needed
    if (ESP.getFreeHeap() < 10000) {
        Serial.println("WARNING: Low memory detected!");
    }
    
    delay(1000); // Main loop runs every second
}
