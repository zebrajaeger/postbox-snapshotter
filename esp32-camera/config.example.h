#include <Arduino.h>

// WLAN credentials
const char* ssid = "";
const char* password = "";

// Upload Server
const char* uploadServer = "192.168.178.31";
const uint16_t uploadPort = 3000;
const char* uploadPath = "/upload";

// Cron Schedule
// const char* cronSchedule =    "*/15 * * * * *"; // 15s
const char* cronSchedule =    "0 */15 * * * *"; // 15min

// NTP
const char* ntpServer = "pool.ntp.org";
// see here: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
const char* ntpTimezone = "CET-1CEST,M3.5.0,M10.5.0/3";  // Europe/Berlin
