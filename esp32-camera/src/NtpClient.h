#pragma once

#include <Arduino.h>

class NtpClient {
 public:
  NtpClient(const char* server, const char* timezone);
  bool begin();
  String formatTimestamp(time_t t) const;
  time_t getNow() const;
  time_t calculateWakeupTimestamp(time_t from, const char* cronString) const;
  int32_t calculateSleepTimeSeconds(time_t from, time_t wakeupAt) const;

 private:
  const char* _server;
  const char* _timezone;
  const uint64_t FALLBACK_SLEEP_US =
      (uint64_t)1000 * (uint64_t)1000 * (uint64_t)60 * (uint64_t)60;  // 1 Stunde als Fallback-Wert
};