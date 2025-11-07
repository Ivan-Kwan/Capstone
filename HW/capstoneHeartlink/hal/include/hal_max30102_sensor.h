#ifndef HAL_MAX30102_SENSOR_H
#define HAL_MAX30102_SENSOR_H

#include "esp_err.h"
#include "hal_reg_map.h"
#include "max30102_types.h"
#include <stdint.h>

// ==== Function Prototypes ====
esp_err_t max30102SensorInit(void);
esp_err_t max30102SetMode(sensorMode mode);
esp_err_t max30102SetLedCurrent(uint8_t redCurrent, uint8_t irCurrent);
esp_err_t max30102FifoInit(void);
esp_err_t max30102ResetFifo(void);
esp_err_t max30102ReadFifo(uint32_t *redData, uint32_t *irData);
esp_err_t max30102ReadInterrupt(uint8_t *intStatus1, uint8_t *intStatus2);
#endif