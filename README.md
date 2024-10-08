# Post Box Snapshooter

- Based on ESP32-CAM wit OV2640 camera
  - Details: <https://randomnerdtutorials.com/esp32-cam-ai-thinker-pinout/>

## Features

- Takes every hour an snapshot and uploads it via FTP
- Special pin (GPIO12, active-low) keeps device in OTA mode for firmware update
- Update via FTP possible (put the firmware.bin file into the ftp upload folder)

## Example picture from my post box

![Picture from potbox](./doc/2024_08_24-15_08_54.jpg)

## Snapshooter in my postbox

![Snapshooter](./doc/mypostbox.jpg)

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

## ToDo

### Important

- [X] Firmware image on ftp server for updates
- [X] Add PSRAM support for bigger images (IPUS IP6404LQ: (2.7..3.6)V, <./doc/doc/IPUS_IPS6404_Datasheet.pdf>)
- [X] Let the camera run for some seconds to let gain- and color-control settle up

### Nice to have

- [X] Consider timezone and daylight saving time
- [ ] Store config in LittleFS (but how to set it?)

### Maybe later...

- [ ] Using a WiFi manager
- [ ] Using  <https://esphome.github.io/esp-web-tools/> like WLED: <https://install.wled.me/> for easy install
- [ ] Cleanup - because this ist not the ebst code  i've ever written...
