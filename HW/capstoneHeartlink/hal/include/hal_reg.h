#ifndef HAL_REG_H
#define HAL_REG_H

#include "hal_i2c.h"
#include "hal_config.h"
#include "esp_err.h"
#include <stdint.h>

// Register read/write function prototypes
esp_err_t regWrite8(uint8_t regAddr, uint8_t data);
esp_err_t regRead8(uint8_t regAddr, uint8_t *data);
esp_err_t regBurstRead(uint8_t regAddr, uint8_t *buffer, uint8_t length);

#endif