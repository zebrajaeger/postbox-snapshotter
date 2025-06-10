#pragma once

#include <Arduino.h>
#include <esp_camera.h>

#include <functional>

#define FLASH_LED_PIN 4

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

class CameraModule {
 public:
  using CaptureCallback = std::function<void(const uint8_t* buf, size_t len)>;

  /**
   * Konstruktor initialisiert interne Kamera-Konfiguration.
   * @param trainingShots Anzahl der Aufwärm-Bilder (AGC/AWB)
   * @param flashPin GPIO-Pin für Blitz-LED
   */
  CameraModule(uint16_t trainingShots = 10);

  /**
   * Initialisiert die Kamera mit der internen Konfiguration.
   */
  esp_err_t begin();
  void sleep();

  void flash(bool enable);

  void doWormupShots(uint8_t shots);

  /**
   * Macht ein Foto und liefert es via Callback zurück.
   * @param callback Lambda, das Buffer-Pointer und Länge erhält
   * @return 0 bei Erfolg, -1 bei Fehler
   */
  void capture(CaptureCallback callback);

  /**
   * Zugriff auf die Kamera-Konfiguration (zum Lesen/Ändern)
   */
  camera_config_t& getConfig();

  String toString(esp_err_t code) const;

 private:
  camera_config_t _config;
  uint16_t _trainingShots;
};