#include "CameraModule.h"

#include "driver/rtc_io.h"

CameraModule::CameraModule(uint16_t trainingShots) : _trainingShots(trainingShots) {
  // AF2569 = OV2640
  _config.ledc_channel = LEDC_CHANNEL_0;
  _config.ledc_timer = LEDC_TIMER_0;
  _config.pin_d0 = Y2_GPIO_NUM;
  _config.pin_d1 = Y3_GPIO_NUM;
  _config.pin_d2 = Y4_GPIO_NUM;
  _config.pin_d3 = Y5_GPIO_NUM;
  _config.pin_d4 = Y6_GPIO_NUM;
  _config.pin_d5 = Y7_GPIO_NUM;
  _config.pin_d6 = Y8_GPIO_NUM;
  _config.pin_d7 = Y9_GPIO_NUM;
  _config.pin_xclk = XCLK_GPIO_NUM;
  _config.pin_pclk = PCLK_GPIO_NUM;
  _config.pin_vsync = VSYNC_GPIO_NUM;
  _config.pin_href = HREF_GPIO_NUM;
  _config.pin_sccb_sda = SIOD_GPIO_NUM;
  _config.pin_sccb_scl = SIOC_GPIO_NUM;
  _config.pin_pwdn = PWDN_GPIO_NUM;
  _config.pin_reset = RESET_GPIO_NUM;
  _config.xclk_freq_hz = 20000000;  // 16MHz
  _config.pixel_format = PIXFORMAT_JPEG;
  // _config.frame_size = FRAMESIZE_SVGA;
  _config.frame_size = FRAMESIZE_SXGA;
  _config.jpeg_quality = 10;
  _config.fb_count = 2;  // for permanent running, use 2 buffers
  _config.fb_location = CAMERA_FB_IN_PSRAM;
}

esp_err_t CameraModule::begin() {
  // flash LED (off)
  rtc_gpio_hold_dis((gpio_num_t)FLASH_LED_PIN);
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, HIGH);

  // camera power (on)
  rtc_gpio_hold_dis((gpio_num_t)_config.pin_pwdn);
  pinMode(_config.pin_pwdn, OUTPUT);
  digitalWrite(_config.pin_pwdn, LOW);

  // just in case (I²C or I²S bus) and clock
  rtc_gpio_hold_dis((gpio_num_t)_config.pin_xclk);
  rtc_gpio_hold_dis((gpio_num_t)_config.pin_sccb_sda);
  rtc_gpio_hold_dis((gpio_num_t)_config.pin_sccb_scl);
  pinMode(_config.pin_sccb_sda, INPUT_PULLUP);
  pinMode(_config.pin_sccb_scl, INPUT_PULLUP);

  esp_err_t err = ESP_FAIL;
  for (uint8_t i = 0; i < 10 && err != ESP_OK; ++i) {
    Serial.println("Starte Kamera-Initialisierung(" + String(i) + ")");

    esp_camera_deinit();

    // delay(50);
    err = esp_camera_init(&_config);
    if (err != ESP_OK) {
      Serial.println("**************************************************");
      Serial.printf("* Kamera-Init fehlgeschlagen: 0x%x\n", err);
      Serial.println("**************************************************");
      digitalWrite(_config.pin_pwdn, HIGH);
      delay(250);
      digitalWrite(_config.pin_pwdn, LOW);
    } else {
      sensor_t* s = esp_camera_sensor_get();
      // s->set_brightness(s, 0);      // -2 to 2
      // s->set_contrast(s, 0);        // -2 to 2
      // s->set_saturation(s, 0);      // -2 to 2
      // s->set_special_effect(s, 0);  // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint,
      //                               // 5 - Blue Tint, 6 - Sepia)
      // s->set_whitebal(s, 0);        // 0 = disable , 1 = enable
      // s->set_awb_gain(s, 0);        // 0 = disable , 1 = enable
      // s->set_wb_mode(s, 0);  // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
      // s->set_exposure_ctrl(s, 0);               // 0 = disable , 1 = enable
      // s->set_aec2(s, 0);                        // 0 = disable , 1 = enable
      // s->set_ae_level(s, 0);                    // -2 to 2
      // s->set_aec_value(s, 300);                 // 0 to 1200
      // s->set_gain_ctrl(s, 0);                   // 0 = disable , 1 = enable
      // s->set_agc_gain(s, 0);                    // 0 to 30
      // s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
      // s->set_bpc(s, 0);                         // 0 = disable , 1 = enable
      // s->set_wpc(s, 0);                         // 0 = disable , 1 = enable
      // s->set_raw_gma(s, 0);                     // 0 = disable , 1 = enable
      // s->set_lenc(s, 0);                        // 0 = disable , 1 = enable
      // s->set_hmirror(s, 0);                     // 0 = disable , 1 = enable
      // s->set_vflip(s, 0);                       // 0 = disable , 1 = enable
      // s->set_dcw(s, 0);                         // 0 = disable , 1 = enable
      // s->set_colorbar(s, 1);                    // 0 = disable , 1 = enable

      // s->set_aec2(s, false);
      // s->set_exposure_ctrl(s, false);
      // s->set_gain_ctrl(s, false);
      // s->set_aec_value(s, 30000);

      // s->set_whitebal(s, false);  // AWB aus
      // s->set_reg(s, 0xD0, 0xFF, 0x40);
      // s->set_reg(s, 0xD1, 0xFF, 0x40);
      s->set_awb_gain(s, false);
      s->set_wb_mode(s, 1);  //    0 = AUTO, 1 = SUNNY, 2 = CLOUDY, 3 = OFFICE, 4 = HOME
    }
  }

  return err;
}

void CameraModule::sleep() {
  Serial.println("Kamera in den Schlafmodus versetzen");

  esp_camera_deinit();

  // flash LED and camera power down
  rtc_gpio_hold_en((gpio_num_t)FLASH_LED_PIN);
  digitalWrite(FLASH_LED_PIN, LOW);

  rtc_gpio_hold_en((gpio_num_t)PWDN_GPIO_NUM);
  digitalWrite(PWDN_GPIO_NUM, HIGH);
}

camera_config_t& CameraModule::getConfig() { return _config; }

void CameraModule::flash(bool enable) {
  if (enable) {
    Serial.println("Blitz-LED einschalten");
    digitalWrite(FLASH_LED_PIN, HIGH);
  } else {
    Serial.println("Blitz-LED ausschalten");
    digitalWrite(FLASH_LED_PIN, LOW);
  }
}

void CameraModule::doWormupShots(uint8_t shots) {
  Serial.println("Aufwärmshots: ");
  for (uint16_t i = 0; i < shots; ++i) {
    Serial.print("- #");
    Serial.print(i + 1);
    Serial.print(": ");
    uint32_t t1 = millis();
    camera_fb_t* fb = esp_camera_fb_get();
    uint32_t t2 = millis();
    Serial.print(t2 - t1);
    Serial.println(" ms");

    if (fb) esp_camera_fb_return(fb);
  }
  Serial.print("- Aufwärmshots abgeschlossen\n");
  ;
}

void CameraModule::capture(CaptureCallback callback) {
  Serial.println("Mache Foto");
  camera_fb_t* fb = esp_camera_fb_get();
  if (fb) {
    printf("- Foto-Size: %zu Bytes\n", fb->len);
    callback(fb->buf, fb->len);
    esp_camera_fb_return(fb);
    Serial.println("- Foto-Aufnahme erfolgreich");
  } else {
    Serial.println("- Kamera-Aufnahme fehlgeschlagen");
  }
}

String CameraModule::toString(esp_err_t code) const {
  switch (code) {
    case ESP_OK:
      return String("ESP_OK (0x00): esp_err_t value indicating success (no error)");
    case ESP_FAIL:
      return String("ESP_FAIL (0xFFFFFFFF): Generic esp_err_t code indicating failure");
    case ESP_ERR_NO_MEM:
      return String("ESP_ERR_NO_MEM (0x0101): Out of memory");
    case ESP_ERR_INVALID_ARG:
      return String("ESP_ERR_INVALID_ARG (0x0102): Invalid argument");
    case ESP_ERR_INVALID_STATE:
      return String("ESP_ERR_INVALID_STATE (0x0103): Invalid state");
    case ESP_ERR_INVALID_SIZE:
      return String("ESP_ERR_INVALID_SIZE (0x0104): Invalid size");
    case ESP_ERR_NOT_FOUND:
      return String("ESP_ERR_NOT_FOUND (0x0105): Requested resource not found");
    case ESP_ERR_NOT_SUPPORTED:
      return String("ESP_ERR_NOT_SUPPORTED (0x0106): Operation or feature not supported");
    case ESP_ERR_TIMEOUT:
      return String("ESP_ERR_TIMEOUT (0x0107): Operation timed out");
    case ESP_ERR_INVALID_RESPONSE:
      return String("ESP_ERR_INVALID_RESPONSE (0x0108): Received response was invalid");
    case ESP_ERR_INVALID_CRC:
      return String("ESP_ERR_INVALID_CRC (0x0109): CRC or checksum was invalid");
    case ESP_ERR_INVALID_VERSION:
      return String("ESP_ERR_INVALID_VERSION (0x010A): Version was invalid");
    case ESP_ERR_INVALID_MAC:
      return String("ESP_ERR_INVALID_MAC (0x010B): MAC address was invalid");
    case ESP_ERR_NOT_FINISHED:
      return String("ESP_ERR_NOT_FINISHED (0x010C): There are items remained to retrieve");
    default: {
      // Unbekannter Code
      char buf[48];
      // cast auf unsigned, um negatives richtig als hex anzuzeigen
      snprintf(buf, sizeof(buf), "UNKNOWN (0x%X)", (uint32_t)code);
      return String(buf);
    }
  }
}
