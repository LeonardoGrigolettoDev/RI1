#include "wifi_service.h"

// Global WiFi status variable
bool wifiConnected = false;

/**
 * Initialize WiFi connection with automatic retry mechanism
 * @return true if connection successful, false otherwise
 */
bool initWiFi() {
    Serial.printf("[WiFi] Connecting to network: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    // Try indefinitely until connection is established
    while (WiFi.status() != WL_CONNECTED) {
        delay(WIFI_RETRY_INTERVAL_MS);
        Serial.print(".");
        attempts++;
        
        // Show progress every 20 attempts (10 seconds)
        if (attempts % WIFI_STATUS_CHECK_INTERVAL == 0) {
            Serial.printf("\n[WiFi] Attempt %d - Status: %d\n", attempts, WiFi.status());
            Serial.printf("[WiFi] Continuing connection attempts...\n");
        }
        
        // Restart WiFi every 60 attempts (30 seconds)
        if (attempts % WIFI_RESTART_INTERVAL == 0) {
            Serial.println("\n[WiFi] Restarting WiFi after 30 seconds...");
            WiFi.disconnect();
            delay(1000);
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
    }
    
    wifiConnected = true;
    Serial.println();
    Serial.printf("[WiFi] Connected successfully!\n");
    Serial.printf("[WiFi] IP address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("[WiFi] Signal strength: %d dBm\n", WiFi.RSSI());
    Serial.printf("[WiFi] SSID: %s\n", WiFi.SSID().c_str());
    
    return true;
}

/**
 * Check if WiFi is currently connected
 * @return true if connected, false otherwise
 */
bool isWiFiConnected() {
    bool connected = (WiFi.status() == WL_CONNECTED);
    wifiConnected = connected;
    return connected;
}

/**
 * Disconnect from WiFi network
 */
void disconnectWiFi() {
    WiFi.disconnect();
    wifiConnected = false;
    Serial.println("[WiFi] Disconnected from network");
}

/**
 * Reconnect to WiFi network
 * @return true if reconnection successful, false otherwise
 */
void reconnectWiFi() {
    Serial.println("[WiFi] Attempting to reconnect...");
    disconnectWiFi();
    delay(1000);
    initWiFi();
}

/**
 * Get current WiFi IP address
 * @return IP address as string
 */
String getWiFiIP() {
    if (isWiFiConnected()) {
        return WiFi.localIP().toString();
    }
    return "Not connected";
}

/**
 * Get current WiFi signal strength
 * @return RSSI value in dBm
 */
int getWiFiRSSI() {
    if (isWiFiConnected()) {
        return WiFi.RSSI();
    }
    return 0;
}

/**
 * Get current connected WiFi SSID
 * @return SSID as string
 */
String getWiFiSSID() {
    if (isWiFiConnected()) {
        return WiFi.SSID();
    }
    return "Not connected";
}

/**
 * Print detailed WiFi status information
 */
void printWiFiStatus() {
    Serial.println("\n=== WiFi Status ===");
    Serial.printf("Status: %s\n", isWiFiConnected() ? "Connected" : "Disconnected");
    
    if (isWiFiConnected()) {
        Serial.printf("SSID: %s\n", getWiFiSSID().c_str());
        Serial.printf("IP Address: %s\n", getWiFiIP().c_str());
        Serial.printf("Signal Strength: %d dBm\n", getWiFiRSSI());
        Serial.printf("MAC Address: %s\n", WiFi.macAddress().c_str());
        Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
        Serial.printf("Subnet Mask: %s\n", WiFi.subnetMask().toString().c_str());
        Serial.printf("DNS 1: %s\n", WiFi.dnsIP(0).toString().c_str());
        Serial.printf("DNS 2: %s\n", WiFi.dnsIP(1).toString().c_str());
    }
    Serial.println("==================\n");
}

/**
 * Check WiFi connection and attempt reconnection if needed
 * This function should be called periodically to maintain connection
 */
void checkWiFiConnection() {
    if (!isWiFiConnected()) {
        Serial.println("[WiFi] Connection lost, attempting reconnection...");
        reconnectWiFi();
    }
}