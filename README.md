# ESP32-CAM Multi-Worker System

This project implements a comprehensive ESP32-CAM solution with three dedicated worker tasks:

1. **MQTT Worker** - Handles MQTT communication and remote commands
2. **Camera Worker** - Manages camera operations and image capture  
3. **RTSP Worker** - Provides optional video streaming via RTSP protocol

## Features

- 🌐 **WiFi Connectivity** with automatic reconnection
- 📡 **MQTT Integration** for command and control
- 🎥 **RTSP Video Streaming** compatible with standard media players
- � **Continuous Image Capture** with system monitoring
- ⚡ **Multi-core Processing** using FreeRTOS tasks
- 💾 **PSRAM Utilization** for better performance
- 🔄 **Robust Error Handling** with automatic recovery

## Architecture

### Worker Distribution
- **Core 0**: MQTT Worker (Priority 3) + RTSP Worker (Priority 1)
- **Core 1**: Camera Worker (Priority 2)

### Communication Flow
```
MQTT Commands → Worker 1 → System Control
Camera Data → Worker 2 → RTSP Stream → Worker 3 → Clients
```

## Hardware Requirements

- ESP32-CAM AI Thinker board
- MicroSD card (optional)
- External antenna (recommended for better WiFi range)

## Configuration

### 1. WiFi Settings
Edit `include/config.h`:
```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

### 2. MQTT Settings
```cpp
#define MQTT_SERVER "YOUR_MQTT_BROKER"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "ESP32CAM_AI"
#define MQTT_USERNAME "YOUR_MQTT_USERNAME"  // Optional
#define MQTT_PASSWORD "YOUR_MQTT_PASSWORD"  // Optional
```

### 3. Camera Settings
```cpp
#define FRAME_SIZE FRAMESIZE_VGA  // Adjust based on your needs
#define JPEG_QUALITY 12           // Lower = higher quality
```

## MQTT Commands

Send JSON commands to topic `esp32cam/command`:

### Start/Stop Streaming
```json
{
  "command": "start_stream"
}
```
```json
{
  "command": "stop_stream"
}
```

### Face Detection Control
```json
{
  "command": "enable_face_detection"
}
```
```json
{
  "command": "disable_face_detection"
}
```

### Capture Single Image
```json
{
  "command": "capture_image"
}
```

### Get System Status
```json
{
  "command": "get_status"
}
```

### Get Face Detection Data
```json
{
  "command": "get_face_data"
}
```

### Camera Quality Control
```json
{
  "command": "set_camera_quality",
  "params": {
    "quality": 10
  }
}
```

### System Reboot
```json
{
  "command": "reboot"
}
```

## WebSocket API

Connect to `ws://[ESP32_IP]:8080` for real-time communication.

### WebSocket Commands
```json
{
  "command": "start_stream"
}
```
```json
{
  "command": "stop_stream"
}
```
```json
{
  "command": "capture_frame"
}
```
```json
{
  "command": "get_status"
}
```

## Status Topics

The device publishes status information to:
- `esp32cam/status` - General status and responses
- `esp32cam/faces` - Face detection results

## Task Architecture

### Core 0 (Network Tasks)
- **MQTT Task** (Priority 2) - Handles MQTT communication
- **WebSocket Task** (Priority 2) - Manages WebSocket connections and streaming

### Core 1 (Processing Tasks)
- **Face Detection Task** (Priority 1) - Performs face detection analysis

### Main Loop
- System health monitoring
- Watchdog functionality
- Resource usage reporting

## Resource Management

The system is designed to balance performance with limited ESP32 resources:

- **Memory Management**: Uses PSRAM when available
- **Task Priorities**: Network tasks on Core 0, processing on Core 1
- **Frame Rate Control**: Configurable streaming FPS
- **Face Detection Intervals**: Adjustable detection frequency
- **Mutex Protection**: Thread-safe camera access

## Performance Tuning

### For Better Streaming Performance
```cpp
#define STREAM_FPS 15              // Increase FPS
#define FRAME_SIZE FRAMESIZE_QVGA  // Reduce frame size
#define JPEG_QUALITY 15            // Reduce quality
```

### For Better Face Detection
```cpp
#define FACE_DETECTION_INTERVAL_MS 500  // More frequent detection
#define FRAME_SIZE FRAMESIZE_VGA        // Higher resolution
```

### For Memory Constrained Environments
```cpp
#define FRAME_SIZE FRAMESIZE_QVGA
#define JPEG_QUALITY 20
#define STREAM_FPS 5
```

## Installation

1. Clone this repository
2. Open in VS Code with PlatformIO
3. Update configuration in `include/config.h`
4. Build and upload to your ESP32-CAM

## Monitoring

The system provides comprehensive monitoring:
- Heap and PSRAM usage
- Task states
- Connection status
- Face detection statistics
- System uptime

## Extending Face Detection

The current implementation includes a basic face detection framework. To enhance it:

1. Replace the simple detection algorithm in `face_detection.cpp`
2. Integrate with ESP-WHO library for better accuracy
3. Add additional AI models for object detection
4. Implement edge AI processing

## Troubleshooting

### Common Issues

1. **Camera initialization fails**
   - Check connections
   - Verify PSRAM is available
   - Ensure adequate power supply

2. **WiFi connection issues**
   - Verify credentials
   - Check signal strength
   - Try external antenna

3. **Memory issues**
   - Reduce frame size
   - Increase JPEG quality number (lower quality)
   - Disable unnecessary features

4. **Performance issues**
   - Adjust task priorities
   - Modify frame rates
   - Balance detection intervals

## License

This project is open source and available under the MIT License.
