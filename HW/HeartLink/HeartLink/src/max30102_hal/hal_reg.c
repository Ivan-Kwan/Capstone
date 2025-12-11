#include "hal_reg.h"
#include "esp_log.h"

static const char *TAG = "HAL_REG";

/**
 * @brief Write one byte to a register
 */
esp_err_t regWrite8(uint8_t regAddr, uint8_t data)
{
    uint8_t writeBuf[2] = {regAddr, data};
    esp_err_t ret = hal_i2c_write(MAX30102_ADDR, writeBuf, 2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Write reg 0x%02X failed", regAddr);
    }
    return ret;
}

/**
 * @brief Read one byte from a register
 */
esp_err_t regRead8(uint8_t regAddr, uint8_t *data)
{
    esp_err_t ret = hal_i2c_write(MAX30102_ADDR, &regAddr, 1);
    if (ret != ESP_OK) return ret;

    ret = hal_i2c_read(MAX30102_ADDR, data, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Read reg 0x%02X failed", regAddr);
    }
    return ret;
}

/**
 * @brief Burst read multiple bytes starting from a register
 */
esp_err_t regBurstRead(uint8_t regAddr, uint8_t *buffer, uint8_t length)
{
    esp_err_t ret = hal_i2c_write(MAX30102_ADDR, &regAddr, 1);
    if (ret != ESP_OK) return ret;

    ret = hal_i2c_read(MAX30102_ADDR, buffer, length);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Burst read from 0x%02X failed", regAddr);
    }
    return ret;
}