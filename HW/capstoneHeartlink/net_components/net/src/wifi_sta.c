#include "wifi_sta.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

static const char *TAG = "WIFI_STA";
static EventGroupHandle_t s_evt;
#define WIFI_CONNECTED_BIT BIT0

static void onEvent(void* arg, esp_event_base_t base, int32_t id, void* data){
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect(); // Simply reconnect
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_evt, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifiStaInit(const char *ssid, const char *pass)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    s_evt = xEventGroupCreate();
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, onEvent, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, onEvent, NULL, NULL);

    wifi_config_t w = {0};
    strncpy((char*)w.sta.ssid, ssid, sizeof(w.sta.ssid));
    strncpy((char*)w.sta.password, pass, sizeof(w.sta.password));
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &w);
    esp_wifi_start();
    return ESP_OK;
}

bool wifiWaitIp(uint32_t timeoutMs)
{
    EventBits_t b = xEventGroupWaitBits(s_evt, WIFI_CONNECTED_BIT, pdTRUE, pdFALSE,
                                        pdMS_TO_TICKS(timeoutMs));
    return (b & WIFI_CONNECTED_BIT) != 0;
}

void wifiStop(void)
{
    esp_wifi_stop();
    esp_wifi_deinit();
}