#ifdef __cplusplus
extern "C" {
#endif

#pragma once
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/**
 * @brief Initialize the sensor interface and register the data queue.
 * @param queue Handle to the FreeRTOS queue where samples will be sent.
 */
esp_err_t max30102IntfInit(QueueHandle_t queue);

/**
 * @brief Start the sensor data acquisition task.
 */
esp_err_t max30102IntfStart(void);

/**
 * @brief Stop the sensor data acquisition task.
 */
void max30102IntfStop(void);

#ifdef __cplusplus
}
#endif