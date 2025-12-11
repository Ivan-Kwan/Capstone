#include "esp_rom_sys.h"
#include "http_client_common.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include <string.h>
#include <time.h>
#include <stdio.h>

// Provided by tls_bundle.c
extern const char root_ca_pem[];
#define API_KEY "HOLY_MOLY_123"

esp_err_t httpPostJson(const char *url, const char *json, int *status, int timeout_ms, int retry_max)
{
    esp_err_t ret = ESP_FAIL; 
    int sc = -1;

    for (int i = 0; i < retry_max; ++i) {
        esp_http_client_config_t cfg = {
            .url = url,
            .method = HTTP_METHOD_POST,
            .cert_pem = root_ca_pem,
            .timeout_ms = timeout_ms,
            .buffer_size = 4096,
        };

        esp_http_client_handle_t h = esp_http_client_init(&cfg);
        if (!h) {
            // Initialize Failed
            if (i == retry_max - 1) break;
            vTaskDelay(pdMS_TO_TICKS(300 * (i+1)));
            continue;
        }
        esp_http_client_set_header(h, "Content-Type", "application/json");
        esp_http_client_set_header(h, "x-api-key", API_KEY); // API_KEY 来自 http_client_common.c
        
        time_t now;
        time(&now);
        char ts_str[20];
        snprintf(ts_str, sizeof(ts_str), "%ld", (long)now);
        esp_http_client_set_header(h, "timestamp", ts_str);

        esp_http_client_set_post_field(h, json, strlen(json)); 

        ret = esp_http_client_perform(h);

        if (ret == ESP_OK) {
            // sc = esp_http_client_get_status_code(h);
            // esp_rom_printf("[HTTP] POST Status = %d, content_length = %d\n",
            //                sc,
            //                (int)esp_http_client_get_content_length(h));
            sc = esp_http_client_get_status_code(h);
            esp_rom_printf("[HTTP] POST Status = %d, content_length = %d\n",
                           sc,
                           (int)esp_http_client_get_content_length(h));

            if (sc >= 200 && sc < 300) {
                esp_http_client_cleanup(h);
                if (status) *status = sc;
                return ESP_OK;
            } else {
                esp_rom_printf("DEBUG: Server responded with error status: %d\n", sc);
            }
        } else {
            esp_rom_printf("DEBUG: HTTP Failed! Error: 0x%x (%s)\n", ret, esp_err_to_name(ret));
        }

        esp_http_client_cleanup(h);
        if (i < retry_max - 1) {
            vTaskDelay(pdMS_TO_TICKS(300 * (i+1)));
        }
    }

    if (status) *status = sc;
    return ret; 
}

esp_err_t httpGetJson(const char *url, char *response_buf, int buf_len, int *status_code, int timeout_ms)
{
    esp_http_client_config_t cfg = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .cert_pem = root_ca_pem,
        .timeout_ms = timeout_ms,
        .buffer_size = 4096,
        .event_handler = NULL,
    };

    esp_http_client_handle_t h = esp_http_client_init(&cfg);
    if (!h) return ESP_FAIL;

    esp_http_client_set_header(h, "Content-Type", "application/json");
    esp_http_client_set_header(h, "x-api-key", API_KEY);
    
    esp_err_t ret = esp_http_client_open(h, 0); // content_length=0 for GET
    if (ret != ESP_OK) {
        esp_rom_printf("DEBUG: HTTP Open Failed! Error: 0x%x\n", ret);
        esp_http_client_cleanup(h);
        return ret;
    }

    int contentLength = esp_http_client_fetch_headers(h);
    
    if (contentLength < 0) {
        esp_rom_printf("DEBUG: HTTP Fetch Headers Failed! Content Length: %d\n", contentLength);
        esp_http_client_cleanup(h);
        return ESP_FAIL;
    }

    int sc = esp_http_client_get_status_code(h);
    if (status_code) *status_code = sc;

    if (sc == 200) {
        int total_read = 0;
        int bytes_to_read = buf_len - 1; 

        // Now data should be in the internal buffer, awaiting esp_http_client_read()
        while (total_read < bytes_to_read) {
            int len = esp_http_client_read(h, response_buf + total_read, bytes_to_read - total_read);
            
            if (len <= 0) {
                break; 
            }
            total_read += len;
            
            if (contentLength > 0 && total_read >= contentLength) {
                break;
            }
        }

        response_buf[total_read] = 0; 
    } else {
        ret = ESP_FAIL;
        esp_rom_printf("DEBUG: HTTP Failed! Status Code: %d\n", sc);
    }
    esp_http_client_close(h); 
    esp_http_client_cleanup(h);
    return ret;
}