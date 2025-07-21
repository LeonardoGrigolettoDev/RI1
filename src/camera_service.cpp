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

#include <WiFiClientSecure.h>  // se HTTPS for necessário
#include "base64.h"            // Biblioteca base64 que você usar

extern volatile int imageCounter;
extern bool STREAM_ON;

void cameraWorkerTask(void *parameter)
{
    Serial.println("Worker 2 (Camera) started");

    vTaskDelay(pdMS_TO_TICKS(2000));

    while (true)
    {
        if (STREAM_ON)
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                Serial.println("[CAMERA] WiFi not connected, skipping frame upload");
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }

            camera_fb_t *fb = esp_camera_fb_get();
            if (!fb)
            {
                Serial.println("[STREAM] Failed to capture frame, continuing...");
                vTaskDelay(pdMS_TO_TICKS(50));
                continue;
            }

            // Conecta, envia frame e desconecta (requisição POST por frame)
            WiFiClient client;
            if (!client.connect("192.168.4.6", 8080))
            {
                Serial.println("[STREAM] Failed to connect to server");
                esp_camera_fb_return(fb);
                vTaskDelay(pdMS_TO_TICKS(2000));
                continue;
            }

            imageCounter++;
            Serial.printf("[STREAM] Sending frame #%d (size: %zu bytes)\n", imageCounter, fb->len);

            // Enviar HTTP POST headers
            client.printf("POST /video_frame/%s HTTP/1.1\r\n", DEVICE_ID);
            client.printf("Host: 192.168.4.6\r\n");
            client.printf("Content-Type: image/jpeg\r\n");
            client.printf("Content-Length: %zu\r\n", fb->len);
            client.printf("Connection: close\r\n");
            client.printf("\r\n");

            // Enviar dados JPEG
            size_t bytesWritten = client.write(fb->buf, fb->len);
            client.flush();

            if (bytesWritten == fb->len)
                Serial.printf("[STREAM] Frame #%d sent successfully: %zu bytes\n", imageCounter, bytesWritten);
            else
                Serial.printf("[STREAM] Frame #%d send error: wrote %zu/%zu bytes\n", imageCounter, bytesWritten, fb->len);

            esp_camera_fb_return(fb);
            client.stop();

            vTaskDelay(pdMS_TO_TICKS(50));  // ~20 FPS
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(500));
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