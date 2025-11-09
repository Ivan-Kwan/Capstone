#pragma once
#include <stdbool.h>
#include "esp_err.h"

esp_err_t wifiStaInit(const char *ssid, const char *pass);
bool      wifiWaitIp(uint32_t timeoutMs);
void      wifiStop(void);