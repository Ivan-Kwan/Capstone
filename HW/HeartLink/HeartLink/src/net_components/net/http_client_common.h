#ifdef __cplusplus
extern "C" { 
#endif

#pragma once
#include "esp_err.h"

esp_err_t httpPostJson(const char *url, const char *json, int *status_code, int timeout_ms, int retry_max);
esp_err_t httpGetJson(const char *url, char *response_buf, int buf_len, int *status_code, int timeout_ms);

#ifdef __cplusplus
}
#endif