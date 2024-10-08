#include <ArduinoOTA.h>
#include <ESP32_FTPClient.h>
#include <WiFi.h>
#include <esp_camera.h>

#include "credentials.h"
#include "driver/rtc_io.h"
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "soc/soc.h"           // Disable brownout problems

// Camera Pin-Definitions for ESP32-CAM
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// GPIO Pins
#define FLASH_LED_PIN 4
#define SIGNAL_LED_PIN 33
#define WAKEUP_PIN 12

// NTP
#include "time.h"

// FTPClient Objekt
ESP32_FTPClient ftp(ftp_server, ftp_user, ftp_pass);


// --------------------
void setupCamera();
void takePictureAndSendFTP();
void enterDeepSleep();
void printLocalTime();
void updateFirmware();
void initTime();

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  // disable brownout detector

  // Signal LED
  pinMode(SIGNAL_LED_PIN, OUTPUT);
  digitalWrite(SIGNAL_LED_PIN, false); // inverted 

  // Serial
  Serial.begin(115200);
  Serial.println("Start");

  // Connect WiFI
  Serial.println("wifi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP().toString());

  // OTA Setup
  // if WAKEUP_PIN  LOW  ? -> go into OTA mode
  pinMode(WAKEUP_PIN, INPUT_PULLUP);
  if (digitalRead(WAKEUP_PIN) == LOW) {
    ArduinoOTA.begin();
    bool on = false;
    Serial.println("OTA mode - Waiting for updates");
    for (;;) {
      on = !on;
      digitalWrite(SIGNAL_LED_PIN, on);
      ArduinoOTA.handle();
      delay(100);
    }
  }

  // FTP
  ftp.enableDebug();

  // Init Camera
  Serial.println("setupCamera");
  setupCamera();

  // NTP
  initTime();

  // Config camera-LED
  rtc_gpio_hold_dis(GPIO_NUM_4);
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  takePictureAndSendFTP();
  updateFirmware();
  digitalWrite(SIGNAL_LED_PIN, true); // LED off

  WiFi.mode(WIFI_OFF);
  esp_camera_deinit();
  enterDeepSleep();
}

void setupCamera() {
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 5000000;
  //   config.xclk_freq_hz = 2000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 5;
  config.fb_count = 1;
  config.fb_location = CAMERA_FB_IN_DRAM, config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  pinMode(PWDN_GPIO_NUM, OUTPUT);
  digitalWrite(PWDN_GPIO_NUM, LOW);

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void initTime() {
  struct tm timeinfo;

  Serial.println("Setting up time");
  configTime(0, 0, ntpServer);  // First connect to NTP server, with 0 TZ offset
  if (!getLocalTime(&timeinfo)) {
    Serial.println("  Failed to obtain time");
    return;
  }
  Serial.println("  Got the time from NTP");
  // Now we can set the real timezone
  Serial.printf("  Setting Timezone to %s\n", ntpTimezone);
  setenv("TZ", ntpTimezone, 1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}

int8_t getTime(String& result) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return -1;
  }
  char ts[48] = {'\0'};
  snprintf(ts, 48, "%04u_%02u_%02u-%02u_%02u_%02u", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  result = ts;
  return 0;
}

void takePictureAndSendFTP() {
  // Flashlight on
  digitalWrite(FLASH_LED_PIN, HIGH);

  // training mode vor AGC AWB etc
  for (uint8_t i = 0; i < trainingShots; ++i) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb) {
      esp_camera_fb_return(fb);
    }
  }

  // acquire the frame
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera Capture Failed");
  }

  // Flashlight off
  digitalWrite(FLASH_LED_PIN, LOW);

  String f;
  if (getTime(f)) {
    Serial.println("Error: Could not aquire time");
    return;
  }
  f += ".jpg";
  Serial.print("Filename: ");
  Serial.println(f);

  ftp.OpenConnection();
  ftp.ChangeWorkDir(ftp_work_dir);
  ftp.InitFile("Type I");
  ftp.NewFile(f.c_str());
  ftp.WriteData(fb->buf, fb->len);
  ftp.CloseFile();
  ftp.CloseConnection();

  esp_camera_fb_return(fb);
}

void enterDeepSleep() {
  esp_sleep_enable_timer_wakeup(periodUs);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, 0);  // WAKEUP_PIN
  rtc_gpio_isolate(
      GPIO_NUM_4);  // fix led stays on, see
                    // <https://forum.arduino.cc/t/esp32-onboard-led-flash-lighting-up-in-deep-sleep/1039467/4>
  esp_deep_sleep_start();
}

void updateFirmware() {
  Serial.println("* Try to update firmware");
  ftp.OpenConnection();
  ftp.InitFile("Type I");
  ftp.ChangeWorkDir(ftp_work_dir);

  size_t s1 = ftp.getSize(ftp_update_file);
  Serial.print("size :");
  Serial.println(s1);

  if (s1 == 0) {
    Serial.println("Filesize == 0. Stopping update");
    ftp.CloseFile();
    ftp.CloseConnection();
    return;
  }

  Stream* stream = ftp.requestFile(ftp_update_file);
  if (!stream) {
    Serial.println("Could not open firmware file");
    return;
  }

  Update.onProgress([](size_t progress, size_t total) {
    Serial.print(progress * 100 / total);
    Serial.println("%");
  });

  if (Update.begin(s1, U_FLASH, 4)) {
    size_t written = Update.writeStream(*stream);
    if (written == Update.size()) {
      Serial.println("Firmware Update written successfully.");
    } else {
      Serial.println("Firmware Update failed.");
    }

    if (Update.end()) {
      Serial.println("Update done!");
      if (Update.isFinished()) {
        String bck = ftp_update_file;
        String time;
        getTime(time);
        bck = bck + "." + time;
        ftp.RenameFile(ftp_update_file, bck.c_str());
        Serial.print("Firmware genamed to ");
        Serial.println(bck);

        Serial.println("Reboot...");
        ESP.restart();
      } else {
        Serial.println("Update nicht finished.");
      }
    } else {
      // !!!!
      Serial.printf("Update error: %s\n", Update.errorString());
    }
  } else {
    Serial.println("Update begin failed.");
  }
  ftp.CloseFile();
  ftp.CloseConnection();
}

void loop() {}
