#include "rtsp_service.h"

WiFiServer rtspServer(RTSP_PORT);
bool rtspEnabled = true;
int activeClients = 0;
WiFiClient client;

bool initRTSP() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Cannot initialize RTSP: WiFi not connected");
        return false;
    }
    
    rtspServer.begin();
    Serial.printf("RTSP server started on port %d\n", RTSP_PORT);
    Serial.printf("RTSP stream URL: rtsp://%s:%d%s\n", 
                  WiFi.localIP().toString().c_str(), RTSP_PORT, RTSP_STREAM_PATH);
    return true;
}

void rtspWorkerTask(void* parameter) {
    Serial.println("Worker 3 (RTSP) started");
    
    // Wait for WiFi to be connected (handled by MQTT worker)
    while (WiFi.status() != WL_CONNECTED) {
        Serial.println("[RTSP] Waiting for WiFi connection...");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    
    if (!initRTSP()) {
        Serial.println("RTSP Worker: RTSP initialization failed, stopping task");
        vTaskDelete(NULL);
        return;
    }
    
    while (true) {
        if (!rtspEnabled) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        
        // Check for new RTSP clients
        client = rtspServer.available();
        if (client) {
            if (activeClients < RTSP_MAX_CLIENTS) {
                Serial.printf("[RTSP] New client connected from %s\n", 
                             client.remoteIP().toString().c_str());
                activeClients++;
                handleRTSPClient(client);
                activeClients--;
                Serial.println("[RTSP] Client disconnected");
            } else {
                Serial.println("[RTSP] Maximum clients reached, rejecting connection");
                client.stop();
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void handleRTSPClient(WiFiClient& client) {
    String method, path;
    
    while (client.connected()) {
        if (client.available()) {
            if (parseRTSPRequest(client, method, path)) {
                if (method == "OPTIONS") {
                    sendRTSPResponse(client, 
                        "RTSP/1.0 200 OK\r\n"
                        "CSeq: 1\r\n"
                        "Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN\r\n"
                        "\r\n");
                } else if (method == "DESCRIBE") {
                    String sdp = "v=0\r\n"
                                "o=- 0 0 IN IP4 " + WiFi.localIP().toString() + "\r\n"
                                "s=ESP32CAM Stream\r\n"
                                "c=IN IP4 " + WiFi.localIP().toString() + "\r\n"
                                "t=0 0\r\n"
                                "m=video 0 RTP/AVP 26\r\n"
                                "a=control:track1\r\n";
                    
                    String response = "RTSP/1.0 200 OK\r\n"
                                     "CSeq: 2\r\n"
                                     "Content-Type: application/sdp\r\n"
                                     "Content-Length: " + String(sdp.length()) + "\r\n"
                                     "\r\n" + sdp;
                    sendRTSPResponse(client, response.c_str());
                } else if (method == "SETUP") {
                    sendRTSPResponse(client,
                        "RTSP/1.0 200 OK\r\n"
                        "CSeq: 3\r\n"
                        "Transport: RTP/AVP;unicast;client_port=8000-8001\r\n"
                        "Session: 12345678\r\n"
                        "\r\n");
                } else if (method == "PLAY") {
                    sendRTSPResponse(client,
                        "RTSP/1.0 200 OK\r\n"
                        "CSeq: 4\r\n"
                        "Session: 12345678\r\n"
                        "\r\n");
                    
                    // Start streaming
                    Serial.println("[RTSP] Starting video stream");
                    while (client.connected()) {
                        camera_fb_t* fb = esp_camera_fb_get();
                        if (fb) {
                            sendFrameToServer(client, fb);
                            esp_camera_fb_return(fb);
                        }
                        vTaskDelay(pdMS_TO_TICKS(33)); // ~30 FPS
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

bool parseRTSPRequest(WiFiClient& client, String& method, String& path) {
    String line = client.readStringUntil('\n');
    line.trim();
    
    if (line.length() == 0) return false;
    
    int firstSpace = line.indexOf(' ');
    int secondSpace = line.indexOf(' ', firstSpace + 1);
    
    if (firstSpace > 0 && secondSpace > firstSpace) {
        method = line.substring(0, firstSpace);
        path = line.substring(firstSpace + 1, secondSpace);
        
        // Read the rest of the headers
        while (client.available()) {
            String header = client.readStringUntil('\n');
            header.trim();
            if (header.length() == 0) break;
        }
        
        return true;
    }
    
    return false;
}

void sendRTSPResponse(WiFiClient& client, const char* response) {
    client.print(response);
}

void sendFrameToServer(WiFiClient& client, camera_fb_t* fb) {
    if (!fb) return;



    client.println("POST /upload HTTP/1.1");
    client.println("Host: meu-servidor.com");
    client.println("Content-Type: image/jpeg");
    client.print("Content-Length: ");
    client.println(fb->len);
    client.println();
    client.write(fb->buf, fb->len);
}