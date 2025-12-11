#ifdef __cplusplus
extern "C" { 
#endif

#pragma once

#define WIFI_SSID          "iPhone" // Your SSID
#define WIFI_PASS          "13613039060" // Your Password

#define HTTP_INGEST_URL    "https://iot-backend-131221236310.us-central1.run.app/ingest" // Your Cloud URL
#define HTTP_CONTROL_URL   "https://iot-backend-131221236310.us-central1.run.app/control"

#define HTTP_TIMEOUT_MS    15000
#define HTTP_RETRY_MAX     3

#define NTP_SERVER         "216.239.35.0" // NTP Server
#define NTP_SYNC_TIMEOUTMS 200000

#ifdef __cplusplus
}
#endif