#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAL_I2C_H
#define HAL_I2C_H

#include "esp_err.h"
#include <stdint.h>

esp_err_t hal_i2c_init(void);
esp_err_t hal_i2c_write(uint8_t devAddr, const uint8_t *data, size_t len);
esp_err_t hal_i2c_read(uint8_t devAddr, uint8_t *data, size_t len);

#endif

#ifdef __cplusplus
}
#endif