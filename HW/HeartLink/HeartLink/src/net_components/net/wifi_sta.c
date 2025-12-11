#include "wifi_sta.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "lwip/ip_addr.h"
#include "esp_rom_sys.h"

static const char *TAG = "WIFI_STA";
static EventGroupHandle_t s_evt;
#define WIFI_CONNECTED_BIT BIT0

static void onEvent(void* arg, esp_event_base_t base, int32_t id, void* data){
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect(); // Simply reconnect
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (netif) {
            esp_netif_dns_info_t dns_info;
            IP4_ADDR(&dns_info.ip.u_addr.ip4, 8, 8, 8, 8);
            dns_info.ip.type = IPADDR_TYPE_V4;
            
            esp_err_t err = esp_netif_set_dns_info(netif, ESP_NETIF_DNS_MAIN, &dns_info);
            if (err == ESP_OK) {
                esp_rom_printf("DNS Forced to 8.8.8.8 Success!\n");
            } else {
                esp_rom_printf("Failed to force DNS: %d\n", err);
            }
        }

        xEventGroupSetBits(s_evt, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifiStaInit(const char *ssid, const char *pass)
{    
    // Get the default net intf of arduino 
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif == NULL) {
        netif = esp_netif_create_default_wifi_sta();
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    // Try to initialize wifi
    esp_err_t ret = esp_wifi_init(&cfg);
    if (ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "WiFi was already initialized by Arduino (Safe to ignore)");
    } else if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init wifi: %s", esp_err_to_name(ret));
        return ret;
    }

    if (s_evt == NULL) {
        s_evt = xEventGroupCreate();
    }

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, onEvent, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, onEvent, NULL, NULL);

    wifi_config_t wifiInfo = {0};
    strncpy((char*)wifiInfo.sta.ssid, ssid, sizeof(wifiInfo.sta.ssid));
    strncpy((char*)wifiInfo.sta.password, pass, sizeof(wifiInfo.sta.password));
    wifiInfo.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifiInfo.sta.pmf_cfg.capable = true;
    wifiInfo.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiInfo));
    esp_err_t err = esp_wifi_start();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "WiFi Start Failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Initiating connection manually...");
    esp_wifi_connect();
    
    return ESP_OK;
}

bool wifiWaitIp(uint32_t timeoutMs)
{
    if (s_evt == NULL) return false;
    
    ESP_LOGI(TAG, "Waiting for IP address...");
    
    uint32_t waited = 0;
    while (waited < timeoutMs) {
        EventBits_t b = xEventGroupWaitBits(s_evt, WIFI_CONNECTED_BIT, pdTRUE, pdFALSE, pdMS_TO_TICKS(1000));
        if (b & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "IP Address Acquired!");
            return true;
        }
        
        waited += 1000;
        ESP_LOGI(TAG, "Still waiting for IP... (%d/%d ms)", waited, timeoutMs);
    }
    
    ESP_LOGW(TAG, "Wait for IP timed out!");
    return false;
}

void wifiStop(void)
{
    esp_wifi_stop();
}