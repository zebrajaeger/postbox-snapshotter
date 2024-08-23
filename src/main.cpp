#include <ArduinoOTA.h>
#include <ESP32_FTPClient.h>
#include <WiFi.h>
#include <esp_camera.h>

#include "driver/rtc_io.h"
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "soc/soc.h"           // Disable brownout problems

#include "credentials.h"

// Camera Pin-Definitions für ESP32-CAM
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

#define FLASH_LED_PIN 4
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

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  // disable brownout detector

  // Serial
  Serial.begin(115200);
  Serial.println("Start");

  // FTP
  ftp.enableDebug();

  // Init Camera
  Serial.println("setupCamera");
  setupCamera();

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
  ArduinoOTA.begin();

  // Config GPIO for Wakeup
  pinMode(WAKEUP_PIN, INPUT_PULLUP);

  // Config camera-LED
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  // Überprüfen, ob der WAKEUP_PIN auf LOW ist, falls ja OTA aktivieren und nicht in Deep Sleep gehen
  if (digitalRead(WAKEUP_PIN) == LOW) {
    // OTA aktiv
    Serial.println("OTA mode - Waiting for updates");
    while (digitalRead(WAKEUP_PIN) == LOW) {
      ArduinoOTA.handle();
      delay(100);
    }
  } else {
    takePictureAndSendFTP();
    enterDeepSleep();
  }
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
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  //   config.frame_size = FRAMESIZE_UXGA;
  config.frame_size = FRAMESIZE_SVGA, config.jpeg_quality = 16;
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

void takePictureAndSendFTP() {
  // acquire a frame
  digitalWrite(FLASH_LED_PIN, HIGH);
  delay(500);
  camera_fb_t* fb = esp_camera_fb_get();
  digitalWrite(FLASH_LED_PIN, LOW);
  if (!fb) {
    Serial.println("Camera Capture Failed");
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  char ts[48] = {'\0'};
  snprintf(ts, 48, "%04u_%02u_%02u-%02u_%02u_%02u.jpg", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  Serial.print("Filename: ");
  Serial.println((char*)ts);

  ftp.OpenConnection();
  ftp.ChangeWorkDir("/ftp/alpineftp");
  ftp.InitFile("Type I");
  ftp.NewFile(ts);
  ftp.WriteData(fb->buf, fb->len);
  ftp.CloseFile();
  ftp.CloseConnection();

  // return the frame buffer back to the driver for reuse
  esp_camera_fb_return(fb);
}

void enterDeepSleep() {
  uint64_t temp = 1000000;  // 1s
  temp *= 3600;             // 1h
  esp_sleep_enable_timer_wakeup(temp);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, 0);  // WAKEUP_PIN als Wakeup Quelle
  rtc_gpio_isolate(GPIO_NUM_4);
  esp_deep_sleep_start();
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay, 10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
}

void loop() {}
