#include "http_client_common.h"
#include "esp_http_client.h"
#include "esp_log.h"

// Provided by tls_bundle.c（Compile with root_ca.pem）
extern const char root_ca_pem[] asm("_binary_root_ca_pem_start");

esp_err_t httpPostJson(const char *url, const char *json, int *status, int timeout_ms, int retry_max)
{
    esp_err_t ret = ESP_FAIL; int sc = -1;
    for (int i = 0; i < retry_max; ++i) {
        esp_http_client_config_t cfg = {
            .url = url,
            .method = HTTP_METHOD_POST,
            .cert_pem = root_ca_pem,
            .timeout_ms = timeout_ms,
        };
        esp_http_client_handle_t h = esp_http_client_init(&cfg);
        if (!h) return ESP_FAIL;

        esp_http_client_set_header(h, "Content-Type", "application/json");
        esp_http_client_set_post_field(h, json, strlen(json));

        ret = esp_http_client_perform(h);
        if (ret == ESP_OK) {
            sc = esp_http_client_get_status_code(h);
            esp_http_client_cleanup(h);
            break;
        }
        esp_http_client_cleanup(h);
        vTaskDelay(pdMS_TO_TICKS(300 * (i+1)));
    }
    if (status) *status = sc;
    return ret;
}