#ifdef __cplusplus
extern "C" {
#endif

#pragma once
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/**
 * @brief Initialize the upload interface.
 * @param queue The queue to receive sensor data from.
 */
esp_err_t uploadIntfInit(QueueHandle_t queue);

/**
 * @brief Start the background upload task.
 */
esp_err_t uploadIntfStart(void);

/**
 * @brief Stop the upload task.
 */
void uploadIntfStop(void);

#ifdef __cplusplus
}
#endif