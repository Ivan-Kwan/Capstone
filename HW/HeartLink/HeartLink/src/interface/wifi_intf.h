#ifdef __cplusplus
extern "C" {
#endif

#pragma once
#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Connect to WiFi and synchronize time (SNTP).
 */
esp_err_t wifiIntfConnect(void);

/**
 * @brief Check if the network is ready for data transmission.
 */
bool wifiIntfIsConnected(void);

bool wifiIntfCheckCommand(char *cmd_buffer, int max_len);

#ifdef __cplusplus
}
#endif