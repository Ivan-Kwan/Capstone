#include "transport_http.h"
#include "net_config.h"
#include "wifi_sta.h"
#include "net_time_sntp.h"
#include "http_client_common.h"
#include "esp_log.h"

static const char *TAG = "TRANS_HTTP";

bool transportHttpInit(void)
{
    if (wifiStaInit(WIFI_SSID, WIFI_PASS) != ESP_OK) return false;
    if (!wifiWaitIp(10000)) return false;
    // Activate https after time is matching
    sntpSyncOnce(NTP_SERVER, NTP_SYNC_TIMEOUTMS);
    return true;
}

bool transportHttpSend(const uint8_t *buf, size_t len)
{
    int status = 0;
    esp_err_t ret = httpPostJson(HTTP_INGEST_URL, (const char*)buf, &status,
                                 HTTP_TIMEOUT_MS, HTTP_RETRY_MAX);
    return (ret == ESP_OK) && (status >= 200 && status < 300);
}

bool transportHttpCheckCommand(char *cmd_buffer, int max_len)
{
    int status = 0;
    // Initiate GET request
    esp_err_t ret = httpGetJson(HTTP_CONTROL_URL, cmd_buffer, max_len, &status, HTTP_TIMEOUT_MS);
    
    // If returns 200 OK, request successful
    if (ret == ESP_OK && status >= 200 && status < 300) {
        return true;
    }
    return false;
}

void transportHttpDeinit(void)
{
    wifiStop();
}