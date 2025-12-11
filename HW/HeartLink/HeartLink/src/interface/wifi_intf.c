#include "wifi_intf.h"
#include "net_components/net/transport_http.h"
#include "esp_log.h"

static const char *TAG = "WIFI_INTF";
static bool s_isNetworkReady = false;
static bool s_is_connected = false;

esp_err_t wifiIntfConnect(void) {
    ESP_LOGI(TAG, "Initializing network connection...");
    
    // transportHttpInit handles WiFi connection and SNTP synchronization
    if (transportHttpInit()) {
        s_isNetworkReady = true;
        s_is_connected = true;
        ESP_LOGI(TAG, "Network initialized successfully");
        return ESP_OK;
    }

    s_isNetworkReady = false;
    s_is_connected = false;
    ESP_LOGE(TAG, "Failed to initialize network");
    return ESP_FAIL;
}

bool wifiIntfIsConnected(void) {
    // In a production app, you might want to query the actual ESP WiFi status here
    return s_isNetworkReady;
}

bool wifiIntfCheckCommand(char *cmd_buffer, int max_len) {
    if (!s_is_connected) {
        return false;
    }
    return transportHttpCheckCommand(cmd_buffer, max_len);
}