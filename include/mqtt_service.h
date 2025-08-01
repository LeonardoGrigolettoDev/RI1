#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "wifi_service.h"

// MQTT Configuration
#define MQTT_SERVER "192.168.4.2"
#define MQTT_PORT 1883

#define MQTT_CLIENT_ID "ESP32CAM_RI1"
#define MQTT_TOPIC_SUBSCRIBE "esp32cam/commands"
#define MQTT_TOPIC_PUBLISH "esp32cam/status"

// MQTT Service functions
bool initMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void mqttWorkerTask(void* parameter);
void publishStatus(const char* message);
void reconnectMQTT();

// Global MQTT client
extern WiFiClient wifiClient;
extern PubSubClient mqttClient;
extern bool mqttConnected;

#endif // MQTT_SERVICE_H
