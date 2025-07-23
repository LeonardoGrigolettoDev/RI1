#include "mqtt_service.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
bool mqttConnected = false;

bool initMQTT() {
    if (!isWiFiConnected()) {
        Serial.println("Cannot initialize MQTT: WiFi not connected");
        return false;
    }
    
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    
    Serial.println("MQTT initialized");
    return true;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    Serial.printf("[MQTT] Received message on topic %s: %s\n", topic, message.c_str());
    
    // Handle different commands
    if (message == "capture") {
        Serial.println("[MQTT] Command: Capture image");
        publishStatus("capturing_image");
    } else if (message == "status") {
        Serial.println("[MQTT] Command: Get status");
        publishStatus("online");
    } else if (message.startsWith("rtsp_")) {
        if (message == "rtsp_enable") {
            Serial.println("[MQTT] Command: Enable RTSP");
            publishStatus("rtsp_enabled");
        } else if (message == "rtsp_disable") {
            Serial.println("[MQTT] Command: Disable RTSP");
            publishStatus("rtsp_disabled");
        }
    }
}

void reconnectMQTT() {
    while (!mqttClient.connected() && isWiFiConnected()) {
        Serial.print("Attempting MQTT connection...");
        if (mqttClient.connect(MQTT_CLIENT_ID)) {
            Serial.println(" connected!");
            mqttConnected = true;
            mqttClient.subscribe(MQTT_TOPIC_SUBSCRIBE);
            publishStatus("online");
        } else {
            Serial.print(" failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void publishStatus(const char* message) {
    if (mqttConnected) {
        mqttClient.publish(MQTT_TOPIC_PUBLISH, message);
        Serial.printf("[MQTT] Published status: %s\n", message);
    }
}

void mqttWorkerTask(void* parameter) {
    Serial.println("Worker 1 (MQTT) started");
    
    // Initialize WiFi and MQTT
    if (!initWiFi()) {
        Serial.println("MQTT Worker: WiFi initialization failed, stopping task");
        vTaskDelete(NULL);
        return;
    }
    
    if (!initMQTT()) {
        Serial.println("MQTT Worker: MQTT initialization failed, stopping task");
        vTaskDelete(NULL);
        return;
    }
    
    while (true) {
        // Check WiFi connection
        if (!isWiFiConnected()) {
            mqttConnected = false;
            Serial.println("[MQTT] WiFi disconnected, attempting reconnection...");
            checkWiFiConnection();
            if (isWiFiConnected()) {
                initMQTT();
            }
        }
        
        // Check MQTT connection
        if (isWiFiConnected() && !mqttClient.connected()) {
            mqttConnected = false;
            Serial.println("[MQTT] MQTT disconnected, attempting reconnection...");
            reconnectMQTT();
        }
        
        // Process MQTT messages
        if (mqttConnected) {
            mqttClient.loop();
        }
        
        // Send periodic heartbeat
        static unsigned long lastHeartbeat = 0;
        if (millis() - lastHeartbeat > 30000) { // Every 30 seconds
            publishStatus("heartbeat");
            lastHeartbeat = millis();
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); // Check every 100ms
    }
}
