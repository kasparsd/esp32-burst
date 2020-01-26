#include <Arduino.h>
#include <WiFi.h>
#include <esp_http_client.h>
#include "esp_camera.h"

// Specify your camera module:
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

const char * ssid = "SSID";
const char * password = "PASSWORD";

void upload_photo(camera_fb_t * fb) {
  esp_http_client_config_t config = {
    .url = "http://example.com/upload?app_key=123",
  };

  esp_http_client_handle_t http_client = esp_http_client_init(&config);

  esp_http_client_set_method(http_client, HTTP_METHOD_POST); // Setting this in config doesn't work for some reason.

  esp_http_client_set_header(http_client, "Content-Type", "image/jpeg");
  esp_http_client_set_header(http_client, "Content-Disposition", "inline; filename=frame.jpeg");
  
  esp_http_client_set_post_field(http_client, (const char *)fb->buf, fb->len);

  esp_err_t err = esp_http_client_perform(http_client);
  if (err == ESP_OK) {
    Serial.printf("Upload HTTP response was %d", esp_http_client_get_status_code(http_client));
  }

  esp_http_client_cleanup(http_client);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

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

  // Adjust this depending on your device memory and capabilities.
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  WiFi.begin(ssid, password);

  while (WL_CONNECTED != WiFi.status()) {
    Serial.printf("Trying to connect to %s", ssid);
    delay(500);
  }

  Serial.printf("Connected to %s!", ssid);
}

void loop() {
  camera_fb_t * fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Camera capture failed, retrying in 3 seconds.");
  } else {
    Serial.printf("Camera captured %d bytes!", fb->len);
    upload_photo(fb);
    esp_camera_fb_return(fb);
  }

  sleep(10);
}