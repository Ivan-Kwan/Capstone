#pragma once
#include "esp_err.h"

esp_err_t httpPostJson(const char *url, const char *json, int *status_code, int timeout_ms, int retry_max);