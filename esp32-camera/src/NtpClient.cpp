#include "NtpClient.h"

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "ccronexpr.h"  // Cron‐Bibliothek als C‐Code
#ifdef __cplusplus
}
#endif

NtpClient::NtpClient(const char* server, const char* timezone) : _server(server), _timezone(timezone) {}

bool NtpClient::begin() {
  configTime(0, 0, _server);
  setenv("TZ", _timezone, 1);
  tzset();
  struct tm tm;
  if (!getLocalTime(&tm)) {
    Serial.println("NTP: Zeit nicht verfuegbar");
    return false;
  }
  return true;
}

String NtpClient::formatTimestamp(time_t t) const {
  struct tm tmInfo;
  // Zeit in lokale Zeit umwandeln
  if (localtime_r(&t, &tmInfo) == nullptr) {
    return String("unknown_time");
  }

  char buf[32];
  // YYYY_MM_DD-HH_mm_SS
  snprintf(buf, sizeof(buf), "%04u_%02u_%02u-%02u_%02u_%02u", tmInfo.tm_year + 1900, tmInfo.tm_mon + 1, tmInfo.tm_mday,
           tmInfo.tm_hour, tmInfo.tm_min, tmInfo.tm_sec);
  return String(buf);
}

time_t NtpClient::getNow() const { return time(nullptr); }

time_t NtpClient::calculateWakeupTimestamp(time_t from, const char* cronString) const {
  if (from <= 0) {
    from = getNow();
    if (from <= 0) {
      return -1;
    }
  }

  const char* cronError = nullptr;
  cron_expr* expr = cron_parse_expr(cronString, &cronError);
  if (cronError != nullptr || expr == nullptr) {
    // Ungültiger Cron‐String: Fallback
    Serial.print("calculateSleepTime: Ungültiger Cron: ");
    Serial.println(cronError != nullptr ? cronError : "cron_parse_expr hat nullptr zurückgegeben");
    return -1;
  }

  time_t nextEpoch = cron_next(expr, from);
  cron_expr_free(expr);

  return nextEpoch;
}

int32_t NtpClient::calculateSleepTimeSeconds(time_t from, time_t wakeupAt) const {
  if (from <= 0) {
    // Ungültiger Zeitpunkt: Fallback
    Serial.println("calculateSleepTime: Ungültiger Zeitpunkt 'from', setze Fallback");
    return FALLBACK_SLEEP_US;
  }
  if (wakeupAt <= 0) {
    // Ungültiger Zeitpunkt: Fallback
    Serial.println("calculateSleepTime: Ungültiger Zeitpunkt 'wakeupAt', setze Fallback");
    return FALLBACK_SLEEP_US;
  }

  int32_t diffSec = static_cast<long>(wakeupAt - from);
  if (diffSec < 0) {
    diffSec = 0;
  }

  return diffSec;
}
