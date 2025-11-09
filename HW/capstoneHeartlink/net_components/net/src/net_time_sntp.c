#include "net_time_sntp.h"
#include "esp_sntp.h"
#include "esp_log.h"

static const char *TAG = "SNTP";

bool sntpSyncOnce(const char *server, uint32_t timeoutMs)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, (char*)server);
    sntp_init();

    uint32_t waited = 0;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && waited < timeoutMs) {
        vTaskDelay(pdMS_TO_TICKS(200));
        waited += 200;
    }
    bool ok = (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED);
    if (!ok) ESP_LOGW(TAG, "SNTP timeout");
    return ok;
}