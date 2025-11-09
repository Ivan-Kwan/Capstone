#ifndef MAX30102_DRIVER_H
#define MAX30102_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "max30102_types.h"

/**
 * @brief One-shot default init suitable for most cases.
 * Calls: max30102SensorInit -> max30102FifoInit -> max30102ResetFifo ->
 *        max30102SetMode(SPO2) -> max30102SetLedCurrent(0x24, 0x24)
 */
esp_err_t max30102Init(void);

/**
 * @brief Configure mode and LED currents in one call.
 */
esp_err_t max30102Configure(sensorMode mode, uint8_t redCurrent, uint8_t irCurrent);

/**
 * @brief Read a single RED/IR sample from FIFO.
 */
esp_err_t max30102ReadSample(max30102Sample *out);

/**
 * @brief Soft-reset FIFO pointers (does not change mode/currents).
 */
esp_err_t max30102Reset(void);

/**
 * @brief Read (and clear-on-read) interrupt status registers.
 */
esp_err_t max30102ReadInterruptStatus(uint8_t *intStatus1, uint8_t *intStatus2);

/**
 * @brief Convenience: handle an interrupt by (optionally) reading a sample.
 * If intStatus1 indicates data-ready, reads one sample into @out (if non-NULL).
 * Returns ESP_OK if the flow succeeded (even if no sample was read).
 */
esp_err_t max30102HandleInterrupt(max30102Sample *out);

/**
 * @brief Set the detection time.
 */
esp_err_t max30102StartSession(uint32_t durationMs);

/**
 * @brief Stop the detection.
 */
esp_err_t max30102StopSession(void);

/**
 * @brief Check the status of max30102.
 */
bool max30102IsRunning(void);

#endif