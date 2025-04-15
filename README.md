# ESP32-CAM Web Stream

This project sets up an ESP32-CAM board as a web server to stream video over WiFi.

## Hardware Setup

### Board Information
- Board: ESP32-CAM-MB
- Camera Module: OV2640 (standard with ESP32-CAM)

### Pin Configuration
The following pins are used for the camera module:
```
PWDN_GPIO_NUM: 32
RESET_GPIO_NUM: -1 (not used)
XCLK_GPIO_NUM: 0
SIOD_GPIO_NUM: 26
SIOC_GPIO_NUM: 27
Y9_GPIO_NUM: 35
Y8_GPIO_NUM: 34
Y7_GPIO_NUM: 39
Y6_GPIO_NUM: 36
Y5_GPIO_NUM: 21
Y4_GPIO_NUM: 19
Y3_GPIO_NUM: 18
Y2_GPIO_NUM: 5
VSYNC_GPIO_NUM: 25
HREF_GPIO_NUM: 23
PCLK_GPIO_NUM: 22
```

## WiFi Configuration

1. Copy `config.h.template` to `config.h`
2. Edit `config.h` and replace the following placeholders with your WiFi credentials:
   - `YOUR_WIFI_SSID`: Your WiFi network name
   - `YOUR_WIFI_PASSWORD`: Your WiFi password

## Usage

After uploading the code and connecting to WiFi:
1. The ESP32-CAM will print its IP address to the serial monitor
2. Open a web browser and navigate to `http://<ESP32_IP_ADDRESS>`
3. The video stream will be available at `http://<ESP32_IP_ADDRESS>/stream`

## Troubleshooting

- If the camera fails to initialize, check the serial monitor for error codes
- Ensure the camera module is properly connected to the ESP32-CAM board
- Make sure your WiFi credentials are correct in `config.h`
- The board requires a stable power supply (5V, 2A recommended) 