#include <Arduino.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "camera_service.h"

// Task handles
TaskHandle_t worker1Handle = NULL;
TaskHandle_t worker2Handle = NULL;
TaskHandle_t worker3Handle = NULL;


volatile int syncCounter = 0;



// Worker 2 Task
void worker2Task(void* parameter) {
    Serial.println("Worker 2 started");
    while (true) {
        syncCounter++;
        Serial.printf("[ASYNC %d] Worker 2: Processing task at %lu ms\n", syncCounter, millis());
        vTaskDelay(pdMS_TO_TICKS(150));
        Serial.printf("[ASYNC %d] Worker 2: Task completed\n", syncCounter);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Worker 3 Task
void worker3Task(void* parameter) {
    Serial.println("Worker 3 started");
    while (true) {
        syncCounter++;
        Serial.printf("[ASYNC %d] Worker 3: Processing task at %lu ms\n", syncCounter, millis());
        vTaskDelay(pdMS_TO_TICKS(200));
        Serial.printf("[ASYNC %d] Worker 3: Task completed\n", syncCounter);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== RI1 ESP32-CAM initializing ===");
    uint64_t chipid = ESP.getEfuseMac();
    Serial.printf("Chip ID (Efuse MAC): %04X\n", (uint32_t)(chipid >> 32));
    Serial.printf("MAC Address: %012llX\n", chipid);
    // Initialize camera
    Serial.println("Initializing camera...");
    if (!initCamera()) {
        Serial.println("ERROR: Camera initialization failed!");
        while(1);
    }
    Serial.println("Starting async worker tasks...");
    // Create Worker 1 (Camera)
    xTaskCreatePinnedToCore(
        cameraWorkerTask,      // Task function
        "Worker1_Camera",     // Task name
        4096,                 // Stack size
        NULL,                 // No sync semaphore needed
        2,                    // Priority
        &worker1Handle,       // Task handle
        0                     // Core (0 or 1)
    );
    // Create Worker 2
    xTaskCreatePinnedToCore(
        worker2Task,           // Task function
        "Worker2",             // Task name
        4096,                  // Stack size
        NULL,                  // Parameters
        2,                     // Priority
        &worker2Handle,        // Task handle
        1                      // Core (0 or 1)
    );
    // Create Worker 3
    xTaskCreatePinnedToCore(
        worker3Task,           // Task function
        "Worker3",             // Task name
        4096,                  // Stack size
        NULL,                  // Parameters
        2,                     // Priority
        &worker3Handle,        // Task handle
        0                      // Core (0 or 1)
    );
    Serial.println("All workers created successfully!");
    Serial.println("Workers will print async messages...");
}

void loop() {
    // Main loop - monitor the system and camera
    static unsigned long lastPrint = 0;
    
    if (millis() - lastPrint > 5000) {
        Serial.printf("\n=== System Status at %lu ms ===\n", millis());
        Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
        Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
        Serial.printf("Sync counter: %d\n", syncCounter);
        Serial.printf("Images captured: %d\n", imageCounter);
        Serial.printf("Worker 1 (Camera): %s\n", worker1Handle ? "Running" : "Stopped");
        Serial.printf("Worker 2 status: %s\n", worker2Handle ? "Running" : "Stopped");
        Serial.printf("Worker 3 status: %s\n", worker3Handle ? "Running" : "Stopped");
        Serial.println("==============================\n");
        
        lastPrint = millis();
    }
    
    delay(100);
}
