#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>

// WiFi Configuration
#define WIFI_SSID "ESP32"
#define WIFI_PASSWORD "12345678"

// WiFi connection retry settings
#define WIFI_RETRY_INTERVAL_MS 500
#define WIFI_STATUS_CHECK_INTERVAL 20
#define WIFI_RESTART_INTERVAL 60

// WiFi service functions
bool initWiFi();
bool isWiFiConnected();
void disconnectWiFi();
void reconnectWiFi();
String getWiFiIP();
int getWiFiRSSI();
String getWiFiSSID();
void printWiFiStatus();
void checkWiFiConnection();

// Global WiFi status
extern bool wifiConnected;

#endif // WIFI_SERVICE_H
