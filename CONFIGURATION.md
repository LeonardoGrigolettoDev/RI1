# ESP32-CAM Multi-Worker Configuration Guide

## Overview
This ESP32-CAM project implements three workers:
1. **Worker 1 (MQTT)**: Listens to MQTT topics for commands and publishes status
2. **Worker 2 (Camera)**: Handles camera operations and image capture
3. **Worker 3 (RTSP)**: Provides optional video streaming via RTSP

## Configuration Required

### 1. WiFi Configuration
Edit `include/mqtt_service.h` and update:
```cpp
#define WIFI_SSID "your_actual_wifi_name"
#define WIFI_PASSWORD "your_actual_wifi_password"
```

### 2. MQTT Configuration
Edit `include/mqtt_service.h` and update:
```cpp
#define MQTT_SERVER "your_mqtt_broker_ip_or_hostname"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "ESP32CAM_RI1"
#define MQTT_TOPIC_SUBSCRIBE "esp32cam/commands"
#define MQTT_TOPIC_PUBLISH "esp32cam/status"
```

## MQTT Commands
Send these commands to the subscribe topic (`esp32cam/commands`):

- `capture` - Trigger manual image capture
- `status` - Get current system status
- `rtsp_enable` - Enable RTSP streaming
- `rtsp_disable` - Disable RTSP streaming

## RTSP Streaming
Once the system is running and connected to WiFi, the RTSP stream will be available at:
```
rtsp://[ESP32_IP_ADDRESS]:554/stream
```

You can view the stream using:
- VLC Media Player
- OBS Studio
- Any RTSP-compatible viewer

## System Architecture

### Core Assignment
- **Core 0**: MQTT Worker + RTSP Worker (communication tasks)
- **Core 1**: Camera Worker (intensive image processing)

### Task Priorities
- MQTT Worker: Priority 3 (highest - maintains connectivity)
- Camera Worker: Priority 2 (medium - core functionality)
- RTSP Worker: Priority 1 (lowest - optional service)

### Memory Requirements
- MQTT Worker: 8KB stack (network operations)
- Camera Worker: 4KB stack (image processing)
- RTSP Worker: 8KB stack (client handling)

## Troubleshooting

### Common Issues
1. **Camera initialization fails**: Check camera module connections
2. **WiFi won't connect**: Verify SSID and password
3. **MQTT connection fails**: Check broker IP and port
4. **RTSP stream not working**: Ensure firewall allows port 554

### Serial Monitor Output
The system provides detailed logging:
- `[MQTT]` - MQTT-related messages
- `[CAMERA]` - Camera operation messages  
- `[RTSP]` - RTSP streaming messages
- System status reports every 10 seconds

## Hardware Requirements
- ESP32-CAM module (AI Thinker recommended)
- MicroSD card (optional, for future features)
- Stable 5V power supply
- WiFi network access

## Dependencies
- espressif/esp32-camera@^2.0.4
- knolleary/PubSubClient@^2.8
- WiFi (built-in)
- WiFiClientSecure (built-in)
