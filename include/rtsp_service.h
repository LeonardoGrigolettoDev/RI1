#ifndef RTSP_SERVICE_H
#define RTSP_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"

// RTSP Configuration
#define RTSP_PORT 554
#define RTSP_STREAM_PATH "/stream"
#define RTSP_MAX_CLIENTS 3

// RTSP Service functions
bool initRTSP();
void rtspWorkerTask(void* parameter);
void handleRTSPClient(WiFiClient& client);
void sendRTSPResponse(WiFiClient& client, const char* response);
void sendFrameToServer(WiFiClient& client, camera_fb_t* fb);
bool parseRTSPRequest(WiFiClient& client, String& method, String& path);
// Global RTSP server
extern WiFiServer rtspServer;
extern bool rtspEnabled;
extern int activeClients;
extern WiFiClient client;

#endif // RTSP_SERVICE_H
