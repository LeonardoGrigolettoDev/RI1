#include "camera_service.h"
#include "rtsp_service.h"

volatile int imageCounter = 0;
bool STREAM_ON = true;

bool initCamera()
{
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound())
    {
        config.frame_size = FRAMESIZE_VGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    }
    else
    {
        config.frame_size = FRAMESIZE_QVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }
    Serial.println("Camera initialized successfully!");
    return true;
}

void cameraWorkerTask(void *parameter)
{
    Serial.println("Worker 2 (Camera) started");

    vTaskDelay(pdMS_TO_TICKS(2000));

    WiFiClient streamClient;
    bool streamConnected = false;

    while (true)
    {
        if (STREAM_ON)
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                Serial.println("[CAMERA] WiFi not connected, skipping frame upload");
                if (streamConnected) {
                    streamClient.stop();
                    streamConnected = false;
                }
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }

            // Establish connection if not connected
            if (!streamConnected || !streamClient.connected())
            {
                if (streamConnected) {
                    streamClient.stop();
                }
                
                Serial.println("[STREAM] Establishing connection to server...");
                if (streamClient.connect("192.168.4.6", 8080))
                {
                    Serial.println("[STREAM] Connected to server - Starting continuous stream");
                    imageCounter = 0; // Reset frame counter for new stream

                    // Send HTTP headers for continuous multipart stream
                    streamClient.println("POST /video_stream HTTP/1.1");
                    streamClient.println("Host: 192.168.15.117");
                    streamClient.println("Content-Type: multipart/x-mixed-replace; boundary=--frame");
                    streamClient.println("Connection: keep-alive");
                    streamClient.println("Cache-Control: no-cache");
                    streamClient.println();
                    
                    streamConnected = true;
                    Serial.println("[STREAM] Stream headers sent - Ready for continuous streaming");
                }
                else
                {
                    Serial.println("[STREAM] Failed to connect to server, retrying...");
                    streamConnected = false;
                    vTaskDelay(pdMS_TO_TICKS(2000));
                    continue;
                }
            }

            // Send frame if connected
            if (streamConnected && streamClient.connected())
            {
                camera_fb_t *fb = esp_camera_fb_get();
                if (fb)
                {
                    imageCounter++;
                    Serial.printf("[STREAM] Sending frame #%d (size: %zu bytes)\n", imageCounter, fb->len);
                    
                    // Send multipart boundary and headers
                    size_t headerBytes = 0;
                    headerBytes += streamClient.println("----frame");
                    headerBytes += streamClient.println("Content-Type: image/jpeg");
                    headerBytes += streamClient.print("Content-Length: ");
                    headerBytes += streamClient.println(fb->len);
                    headerBytes += streamClient.println();
                    
                    // Send the actual JPEG data
                    size_t bytesWritten = streamClient.write(fb->buf, fb->len);
                    streamClient.println();
                    
                    if (bytesWritten == fb->len && headerBytes > 0) {
                        Serial.printf("[STREAM] Frame #%d sent successfully: %zu bytes\n", imageCounter, bytesWritten);
                    } else {
                        Serial.printf("[STREAM] Frame #%d send error: wrote %zu/%zu bytes, headers: %zu\n", 
                                     imageCounter, bytesWritten, fb->len, headerBytes);
                        streamConnected = false; // Force reconnection
                    }
                    
                    esp_camera_fb_return(fb);
                }
                else
                {
                    Serial.println("[STREAM] Failed to capture frame, continuing...");
                }
                
                // Check if connection is still alive after sending
                if (!streamClient.connected()) {
                    Serial.println("[STREAM] Connection lost after sending frame");
                    streamConnected = false;
                }
            }
            else
            {
                streamConnected = false;
                Serial.println("[STREAM] Connection lost, will reconnect...");
            }
        }
        else
        {
            // STREAM_ON is false, close connection if open
            if (streamConnected) {
                streamClient.stop();
                streamConnected = false;
                Serial.println("[STREAM] Stream disabled, connection closed");
            }
        }

        // Stream at ~20 FPS when connected, slower when not streaming
        if (STREAM_ON && streamConnected) {
            vTaskDelay(pdMS_TO_TICKS(50)); // 20 FPS for active streaming
        } else {
            vTaskDelay(pdMS_TO_TICKS(500)); // Slower when not streaming
        }
    }
}

// Alternative simple JPEG upload function (uncomment to use instead)
/*
void sendSingleJPEG(WiFiClient& client, camera_fb_t* fb) {
    if (!fb) return;
    
    Serial.printf("[SIMPLE] Sending single JPEG (%zu bytes)\n", fb->len);
    
    client.println("POST /upload HTTP/1.1");
    client.println("Host: 192.168.4.8");
    client.println("Content-Type: image/jpeg");
    client.print("Content-Length: ");
    client.println(fb->len);
    client.println();
    
    size_t bytesWritten = client.write(fb->buf, fb->len);
    Serial.printf("[SIMPLE] Sent %zu/%zu bytes\n", bytesWritten, fb->len);
}
*/