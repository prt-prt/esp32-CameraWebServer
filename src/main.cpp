#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_camera.h>
#include <PubSubClient.h>
#include "config.h"

WebServer server(80);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Camera pins for ESP32-CAM-MB
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Function to generate unique client ID based on IP address
String getClientId() {
  IPAddress ip = WiFi.localIP();
  char clientId[20];
  snprintf(clientId, sizeof(clientId), "esp32-cam-%d", ip[3]);
  return String(clientId);
}

void setupCamera() {
  camera_config_t config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sccb_sda = SIOD_GPIO_NUM,
    .pin_sccb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_VGA,
    .jpeg_quality = 12,
    .fb_count = 2,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY
  };

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x", err);
    return;
  }
}

void handleStream() {
  WiFiClient client = server.client();
  
  // Send headers
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n";
  response += "Access-Control-Allow-Origin: *\r\n";
  response += "\r\n";
  client.write(response.c_str());

  while (true) {
    if (!client.connected()) {
      client.stop();
      return;
    }

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      continue;
    }

    // Send frame header
    String header = "--frame\r\nContent-Type: image/jpeg\r\n\r\n";
    client.write(header.c_str());
    
    // Send frame data
    if (!client.write((char *)fb->buf, fb->len)) {
      esp_camera_fb_return(fb);
      client.stop();
      return;
    }
    
    // Send frame footer
    client.write("\r\n");
    
    esp_camera_fb_return(fb);
    
    // Small delay to prevent overwhelming the client
    delay(10);
  }
}

void handleRoot() {
  server.send(200, "text/html", 
    "<!DOCTYPE html><html><head><title>ESP32-CAM Stream</title></head><body>"
    "<h1>ESP32-CAM Stream</h1><img src='/stream' style='width:100%;'/></body></html>"
  );
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = getClientId();
    
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      Serial.printf("Client ID: %s\n", clientId.c_str());
      
      // Publish initial status
      String status = "{\"ip\":\"" + WiFi.localIP().toString() + "\",\"status\":\"online\",\"clientId\":\"" + clientId + "\"}";
      mqttClient.publish("esp32-cams", status.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  // Log system info
  Serial.println("\n=== ESP32-CAM System Info ===");
  Serial.printf("IP Address: http://%s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Stream URL: http://%s/stream\n", WiFi.localIP().toString().c_str());
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("PSRAM Size: %d bytes\n", ESP.getPsramSize());
  Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.println("===========================\n");
  
  setupCamera();
  
  server.on("/", handleRoot);
  server.on("/stream", handleStream);
  server.begin();

  // Setup MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  reconnectMQTT();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();
  server.handleClient();
} 