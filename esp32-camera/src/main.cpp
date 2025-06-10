#include <Arduino.h>
#include <WiFi.h>
#include <driver/rtc_io.h>
#include <esp_sleep.h>
#include <esp_system.h>
#include <soc/rtc_cntl_reg.h>

#include "../config.h"
#include "CameraModule.h"
#include "FirmwareUpdater.h"
#include "FtpClientWrapper.h"
#include "HttpClientWrapper.h"
#include "NtpClient.h"

CameraModule cam;
HttpClientWrapper http(uploadServer, uploadPort, uploadPath);
NtpClient ntp(ntpServer, ntpTimezone);

RTC_DATA_ATTR time_t lastWakeup = 0;

void wakeup() {
  // release Boot Key
  rtc_gpio_deinit(GPIO_NUM_0);

  // Brownout-Detektor abschalten
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
}

void doSleep(uint64_t sleepUs) {
  Serial.println("Gehe in den Schlafmodus");
  WiFi.mode(WIFI_OFF);

  cam.sleep();

  // Wakeup via Boot Key
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);

  esp_sleep_enable_timer_wakeup(sleepUs);
  esp_deep_sleep_start();
}

void doFailSleep() {
  Serial.println("Fehler aufgetreten, gehe in den Schlafmodus");
  doSleep(1000ULL * 1000 * 60 * 60);
}

void setup() {
  Serial.begin(115200);

  Serial.println("Boote...");

  wakeup();

  // Kamera initialisieren
  esp_err_t camErr = cam.begin();
  if (camErr != ESP_OK) {
    Serial.println("Kamera Init fehlgeschlagen: " + cam.toString(camErr));
    doFailSleep();
  } else {
    Serial.println("Kamera erfolgreich initialisiert");
  }

  // Bild aufnehmen und hochladen
  cam.flash(true);
  cam.doWormupShots(15);
  cam.capture([&](const uint8_t* buf, size_t len) {
    cam.flash(false);

    // WLAN-Verbindung
    Serial.print("Connect WiFi");
    WiFi.begin(ssid, password);
    for (uint8_t i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WIFI Verbindung fehlgeschlagen");
      cam.flash(false);
      doFailSleep();
    }
    Serial.println("WIFI verbunden");

    // NTP starten
    if (ntp.begin()) {
      Serial.println("NTP Client erfolgreich gestartet und synchronisiert");
    } else {
      Serial.println("NTP Client konnte nicht gestartet werden");
      cam.flash(false);
      doFailSleep();
    }
    String ts = ntp.formatTimestamp(ntp.getNow());
    Serial.println("Timestamp: " + ts);

    String filename = ts + ".jpg";
    if (http.uploadImage(buf, len, filename.c_str())) {
      Serial.println("Bild hochgeladen: " + filename);
    } else {
      Serial.println("HTTP Upload fehlgeschlagen");
    }
  });

  // First time, set lastWakeup to current time
  if (lastWakeup == 0) {
    lastWakeup = ntp.getNow();
  }

  time_t nextWakeup = ntp.calculateWakeupTimestamp(lastWakeup, cronSchedule);
  Serial.println("Last Wakeup: " + ntp.formatTimestamp(lastWakeup));
  Serial.println("Next Wakeup: " + ntp.formatTimestamp(nextWakeup));

  lastWakeup = nextWakeup;  // Update lastWakeup for next cycle

  uint64_t sleepUs = ntp.calculateSleepTimeSeconds(ntp.getNow(), nextWakeup);
  Serial.println("Sleep for " + String(sleepUs) + " seconds");
  sleepUs *= 1000000;  // Umwandlung in Mikrosekunden

  doSleep(sleepUs);
}

void loop() {
  // leer
}
