#include <Arduino.h>

// WLAN Konfiguration
const char* ssid = "myWiFiSSID";
const char* password = "myWiFIPassword";

// NTP
const char* ntpServer = "pool.ntp.org";
// see here: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
const char* ntpTimezone = "CET-1CEST,M3.5.0,M10.5.0/3";  // Europe/Berlin

// FTP Konfiguration
const char* ftp_server = "myFtpHostOrIP";
const char* ftp_user = "alpineftp";
const char* ftp_pass = "alpineftp";
const char* ftp_work_dir = "/ftp/alpineftp";
const char* ftp_update_file = "firmware.bin";

// Picture
uint64_t periodUs = 1000 * 1000 * 60 * 60;  // every hour
// uint64_t periodUs = 1000*1000 * 60; // every minute