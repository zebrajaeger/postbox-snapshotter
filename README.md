# Post Box Snapshooter

- Based on ESP32-CAM wit OV
  - Details: <https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/>
- Takes every hour an snapshot (max. 800x600) from the post box and upload it via FTP

## Configure and build

- Clone project
- Open with Visual Studio Code and PlattformIO Plugin
- Open main.cpp
- Set your WiFI credentials
- Set your FTP Server host and credentials
- Set your Timezone
- Build and Upload

## Hardware

- Case: I use <https://www.thingiverse.com/thing:5638294>

## Improvments

- Store config in LittleFS
- Using a WiFi manager
- Using  <https://esphome.github.io/esp-web-tools/> like WLED: <https://install.wled.me/>
- Add PSRAM support for bigger images (IPUS IP6404LQ: (2.7..3.6)V, <./doc/doc/IPUS_IPS6404_Datasheet.pdf>)
- Consider daylight saving time
