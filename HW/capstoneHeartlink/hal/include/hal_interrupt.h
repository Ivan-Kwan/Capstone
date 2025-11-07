#ifndef HAL_INTERRUPT_H
#define HAL_INTERRUPT_H

#include "esp_err.h"
#include <stdbool.h>

/**
 * @brief Initialize GPIO interrupt for MAX30102
 */
esp_err_t interruptInit(void);

/**
 * @brief Wait for an interrupt event
 * @param timeoutMs Timeout in milliseconds
 * @return true if event received, false if timeout
 */
bool interruptWaitEvent(uint32_t timeoutMs);

/**
 * @brief Deinitialize interrupt system
 */
void interruptDeinit(void);

#endif