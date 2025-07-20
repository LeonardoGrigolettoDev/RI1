#include "mqtt_service.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
bool wifiConnected = false;
bool mqttConnected = false;

bool initWiFi() {
    Serial.printf("Connecting to WiFi network: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    // Tenta indefinidamente até conseguir conexão
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        attempts++;
        
        // A cada 20 tentativas (10 segundos), mostra progresso
        if (attempts % 20 == 0) {
            Serial.printf("\n[WiFi] Tentativa %d - Status: %d\n", attempts, WiFi.status());
            Serial.printf("[WiFi] Continuando tentativas de conexão...\n");
        }
        
        // A cada 60 tentativas (30 segundos), reinicia o WiFi
        if (attempts % 60 == 0) {
            Serial.println("\n[WiFi] Reiniciando WiFi após 30 segundos...");
            WiFi.disconnect();
            delay(1000);
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
    }
    
    wifiConnected = true;
    Serial.println();
    Serial.printf("WiFi conectado com sucesso!\n");
    Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Signal strength: %d dBm\n", WiFi.RSSI());
    return true;
}

bool initMQTT() {
    if (!wifiConnected) {
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
    while (!mqttClient.connected() && wifiConnected) {
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
        if (WiFi.status() != WL_CONNECTED) {
            wifiConnected = false;
            mqttConnected = false;
            Serial.println("[MQTT] WiFi disconnected, attempting reconnection...");
            if (initWiFi()) {
                initMQTT();
            }
        }
        
        // Check MQTT connection
        if (wifiConnected && !mqttClient.connected()) {
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
